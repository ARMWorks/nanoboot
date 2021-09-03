/* SPDX-License-Identifier: (GPL-2.0+ OR BSD-3-Clause) */
/*
 * Copyright (C) 2021 Jeff Kent <jeff@jkent.net>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; version 2.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "dwc2_hw.h"
#include "udc.h"

#include <irq.h>
#include <s5pv210.h>
#include <timer.h>
#include <macros.h>

#include <asm/io.h>
#include <linux/errno.h>
#include <linux/usb/ch9.h>

#include <malloc.h>
#include <string.h>


static udc_t udc = { 0 };

extern void dwc2_phy_on(void);
extern void dwc2_phy_off(void);

/* Local declarations */
static int udc_ep_enable(udc_ep_t *ep, const struct usb_endpoint_descriptor *);
static int udc_ep_disable(udc_ep_t *ep);
static udc_req_t *udc_alloc_req(udc_ep_t *ep);
static void udc_free_req(udc_ep_t *ep, udc_req_t *req);
static int udc_queue(udc_ep_t *ep, udc_req_t *req);
static int udc_dequeue(udc_ep_t *ep, udc_req_t *req);
static int udc_set_halt(udc_ep_t *ep, int value);

const udc_ops_t udc_ops = {
    .ep_enable = udc_ep_enable,
    .ep_disable = udc_ep_disable,
    .alloc_req = udc_alloc_req,
    .free_req = udc_free_req,
    .queue = udc_queue,
    .dequeue = udc_dequeue,
    .set_halt = udc_set_halt,
};

static void udc_ep0_setup(udc_t *udc);
static void udc_ep0_read(udc_t *udc);
static void udc_ep0_kick(udc_t *udc, udc_ep_t *ep);
static void udc_handle_ep0(udc_t *gudc);
static int udc_ep0_write(udc_t *udc);
static int udc_write_fifo_ep0(udc_ep_t *ep, udc_req_t *req);
static void udc_done(udc_ep_t *ep, udc_req_t *req, int status);
static void udc_stop_activity(udc_t *udc);
static void udc_set_address(udc_t *udc, uint8_t address);
static void udc_reconfig(udc_t *udc);
static void udc_set_max_pktsize(udc_t *udc, enum usb_device_speed speed);
static void udc_nuke(udc_ep_t *ep, int status);
static void udc_set_nak(udc_ep_t *ep);
static void udc_enable(udc_t *udc);
static void udc_disable(udc_t *udc);
static void udc_irq(int num, void *priv);

/*****************************************************************************/

static void udc_reinit(udc_t *udc)
{

    if (udc->gadget) {
        udc->gadget->unbind(udc);
    }

    udc->ep0_state = EP0_WAIT_FOR_SETUP;

    for (int ep_num = 0; ep_num < 16; ep_num++) {
        udc_ep_t *ep = &udc->ep[ep_num];

        ep->desc = NULL;
        ep->stopped = false;
        INIT_LIST_HEAD(&ep->queue);
        udc->ep[ep_num].udc = udc;
    }

    if (udc->gadget) {
        udc->gadget->bind(udc);
    }
}

static void udc_enable(udc_t *udc)
{
    dwc2_phy_on();
    udc_reconfig(udc);

    irq_set_handler(IRQ_OTG, udc_irq, udc);
}

static void udc_disable(udc_t *udc)
{
    udc_set_address(udc, 0);

    udc->ep0_state = EP0_WAIT_FOR_SETUP;
    udc->speed = USB_SPEED_UNKNOWN;

    dwc2_phy_off();
}

static inline void udc_ep0_zlp(udc_t *udc)
{
    writel((uint32_t) &udc->ctrl, DIEPDMA(0));
    writel(DIEPTSIZ0_PKTCNT(1), DIEPTSIZ0);
    writel(readl(DIEPCTL0) | DXEPCTL_EPENA | DXEPCTL_CNAK, DIEPCTL0);
    udc->ep0_state = EP0_WAIT_FOR_IN_COMPLETE;
}

static inline void udc_pre_setup(udc_t *udc)
{
    writel((uint32_t) &udc->ctrl, DOEPDMA(0));
    writel((DXEPTSIZ_PKTCNT(1)) |
            DXEPTSIZ_XFERSIZE(sizeof(struct usb_ctrlrequest)), DOEPTSIZ(0));
    writel(readl(DOEPCTL0) | DXEPCTL_EPENA, DOEPCTL0);
}

static inline void udc_ep0_complete_out(udc_t *udc)
{
    writel((uint32_t) &udc->ctrl, DOEPDMA(0));
    writel(DXEPTSIZ_PKTCNT(1) |
            DXEPTSIZ_XFERSIZE(sizeof(struct usb_ctrlrequest)), DOEPTSIZ0);
    writel(readl(DOEPCTL0) | DXEPCTL_EPENA | DXEPCTL_CNAK, DOEPCTL0);
}

static int udc_setdma_rx(udc_ep_t *ep, udc_req_t *req)
{
    uint32_t pktcnt;
    uint8_t ep_num = ep->addr & 0xf;

	void *buf = req->buf + req->actual;
	uint32_t length = MIN(req->length - req->actual, ep->max_packet);

    ep->len = length;
    ep->buf = buf;

    if (ep_num == 0 || length == 0) {
        pktcnt = 1;
    } else {
        pktcnt = (length - 1) / ep->max_packet + 1;
    }

    writel((size_t) ep->buf, DOEPDMA(ep_num));
    writel(DXEPTSIZ_PKTCNT(pktcnt) | DXEPTSIZ_XFERSIZE(length),
            DOEPTSIZ(ep_num));
    writel(readl(DOEPCTL(ep_num)) | DXEPCTL_EPENA | DXEPCTL_CNAK,
            DOEPCTL(ep_num));

    return 0;
}

static int udc_setdma_tx(udc_ep_t *ep, udc_req_t *req)
{
    uint32_t pktcnt;
    uint8_t ep_num = ep->addr & 0xf;

    void *buf = req->buf + req->actual;
    uint32_t length = req->length - req->actual;

    if (ep_num == 0) {
        length = MIN(length, ep->max_packet);
    }

    ep->len = length;
    ep->buf = buf;

    if (length == 0) {
        pktcnt = 1;
    } else {
        pktcnt = (length - 1) / ep->max_packet + 1;
    }

    /* Flush the endpoint's Tx FIFO */
    writel(GRSTCTL_TXFNUM(ep_num) | GRSTCTL_TXFFLSH, GRSTCTL);
    while (readl(GRSTCTL) & GRSTCTL_TXFFLSH)
        ;

    writel((size_t) ep->buf, DIEPDMA(ep_num));
    writel(DXEPTSIZ_PKTCNT(pktcnt) | DXEPTSIZ_XFERSIZE(length),
            DIEPTSIZ(ep_num));

    uint32_t ctrl = readl(DIEPCTL(ep_num));

    /* Write the FIFO number to be used for this endpoint */
    ctrl &= ~DXEPCTL_TXFNUM_MASK;
    ctrl |= DXEPCTL_TXFNUM(ep_num);

    /* Clear reserved (Next EP) bits */
    ctrl &= ~(DXEPCTL_NEXTEP_MASK);

    writel(ctrl | DXEPCTL_EPENA | DXEPCTL_CNAK, DIEPCTL(ep_num));

    return length;
}

static void udc_complete_rx(udc_t *udc, uint8_t ep_num)
{
    udc_ep_t *ep = &udc->ep[ep_num];
    uint32_t ep_tsr = 0;
    uint32_t xfer_size = 0;
    bool is_short = false;

    if (list_empty(&ep->queue)) {
        return;
    }

    udc_req_t *req = list_entry(ep->queue.next, udc_req_t, queue);
    ep_tsr = readl(DOEPTSIZ(ep_num));

    if (ep_num == 0) {
        xfer_size = (ep_tsr & DOEPTSIZ0_XFERSIZE_LIMIT);
    } else {
        xfer_size = (ep_tsr & DXEPTSIZ_XFERSIZE_LIMIT);
    }

    xfer_size = ep->len - xfer_size;

    req->actual += MIN(xfer_size, (req->length - req->actual));
    is_short = !!(xfer_size % ep->max_packet);

    if (is_short || req->actual == req->length) {
        if (ep_num == 0 && udc->ep0_state == EP0_DATA_STATE_RECV) {
            udc_ep0_zlp(udc);
        } else {
            udc_done(ep, req, 0);

            if (!list_empty(&ep->queue)) {
                req = list_entry(ep->queue.next, udc_req_t, queue);
                udc_setdma_rx(ep, req);
            }
        }
    } else {
        udc_setdma_rx(ep, req);
    }
}

static void udc_complete_tx(udc_t *udc, uint8_t ep_num)
{
    udc_ep_t *ep = &udc->ep[ep_num];
    udc_req_t *req;
    uint32_t xfer_size = 0;

    if (udc->ep0_state == EP0_WAIT_FOR_NULL_COMPLETE) {
        udc->ep0_state = EP0_WAIT_FOR_OUT_COMPLETE;
        udc_ep0_complete_out(udc);
        return;
    }

    if (list_empty(&ep->queue)) {
        return;
    }

    req = list_entry(ep->queue.next, udc_req_t, queue);

    xfer_size = ep->len;
    req->actual += MIN(xfer_size, (req->length - req->actual));

    if (ep_num == 0) {
        if (udc->ep0_state == EP0_DATA_STATE_XMIT) {
            if (udc_write_fifo_ep0(ep, req)) {
                udc->ep0_state = EP0_WAIT_FOR_COMPLETE;
            }
        } else if (udc->ep0_state == EP0_WAIT_FOR_IN_COMPLETE) {
            udc_done(ep, req, 0);
            udc->ep0_state = EP0_WAIT_FOR_SETUP;
        } else if (udc->ep0_state == EP0_WAIT_FOR_COMPLETE) {
            udc_done(ep, req, 0);
            udc->ep0_state = EP0_WAIT_FOR_OUT_COMPLETE;
            udc_ep0_complete_out(udc);
        }
        return;
    }

    if (req->actual == req->length) {
        udc_done(ep, req, 0);
    }

    if (!list_empty(&ep->queue)) {
        req = list_entry(ep->queue.next, udc_req_t, queue);
        udc_setdma_tx(ep, req);
    }
}

static inline void udc_check_tx_queue(udc_t *udc, uint8_t ep_num)
{
    udc_ep_t *ep = &udc->ep[ep_num];
    udc_req_t *req;

    if (!list_empty(&ep->queue)) {
        req = list_entry(ep->queue.next, udc_req_t, queue);
        if (ep->addr & USB_DIR_IN) {
            udc_setdma_tx(ep, req);
        } else {
            udc_setdma_rx(ep, req);
        }
    }
}

static inline void udc_ep_in_intr(udc_t *udc, uint8_t ep_num,
        uint32_t intr_status)
{
    if (intr_status & DXEPINT_XFERCOMPL) {
        udc_complete_tx(udc, ep_num);

        if (ep_num == 0) {
            if (udc->ep0_state == EP0_WAIT_FOR_IN_COMPLETE) {
                udc->ep0_state = EP0_WAIT_FOR_SETUP;
            }

            if (udc->ep0_state == EP0_WAIT_FOR_SETUP) {
                udc_pre_setup(udc);
            }

            if (udc->clear_feature_flag) {
                udc_check_tx_queue(udc, udc->clear_feature_num);
                udc->clear_feature_flag = false;
            }
        }
    }
}

static inline void udc_ep_out_intr(udc_t *udc, uint8_t ep_num,
        uint32_t intr_status)
{
    if (ep_num == 0) {
        if (intr_status & DXEPINT_XFERCOMPL) {
            uint32_t ep_tsr = readl(DOEPTSIZ(0));
            uint32_t xfer_size = ep_tsr & DOEPTSIZ0_XFERSIZE_LIMIT;

            if (xfer_size == sizeof(struct usb_ctrlrequest) &&
                    udc->ep0_state == EP0_WAIT_FOR_SETUP) {
                udc_pre_setup(udc);
            } else if (udc->ep0_state != EP0_WAIT_FOR_OUT_COMPLETE) {
                udc_complete_rx(udc, ep_num);
            } else {
                udc->ep0_state = EP0_WAIT_FOR_SETUP;
                udc_pre_setup(udc);
            }
        }

        if (intr_status & DXEPINT_SETUP) {
            udc_handle_ep0(udc);
        }
    } else if(intr_status & DXEPINT_XFERCOMPL) {
        udc_complete_rx(udc, ep_num);
    }
}

static void udc_irq(int num, void *priv)
{
    udc_t *udc = (udc_t *) priv;
    uint32_t gintsts = readl(GINTSTS);

    if (gintsts & GINTSTS_ENUMDONE) {
        writel(GINTSTS_ENUMDONE, GINTSTS);

        uint32_t usb_status = readl(DSTS) &
                (DSTS_ENUMSPD_FS48 | DSTS_ENUMSPD_FS);
        if (usb_status) {
            udc_set_max_pktsize(udc, USB_SPEED_FULL);
        } else {
            udc_set_max_pktsize(udc, USB_SPEED_HIGH);
        }
    }

    if (gintsts & GINTSTS_USBRST) {
        writel(GINTSTS_USBRST, GINTSTS);
        udc_reinit(udc);
        udc->ep0_state = EP0_WAIT_FOR_SETUP;
        udc_pre_setup(udc);
    }

    if (gintsts & GINTSTS_IEPINT) {
        uint32_t ep_intr = readl(DAINT) & 0xFFFF;
        uint8_t ep_num = 0;
        while (ep_intr) {
            if (ep_intr & 1) {
                uint32_t ep_intr_status = readl(DIEPINT(ep_num));
                writel(ep_intr_status, DIEPINT(ep_num));
                udc_ep_in_intr(udc, ep_num, ep_intr_status);
            }
            ep_intr >>= 1;
            ep_num++;
        }
    }

    if (gintsts & GINTSTS_OEPINT) {
        uint32_t ep_intr = (readl(DAINT) >> 16) & 0xFFFF;
        uint8_t ep_num = 0;
        while (ep_intr) {
            if (ep_intr & 1) {
                uint32_t ep_intr_status = readl(DOEPINT(ep_num));
                writel(ep_intr_status, DOEPINT(ep_num));
                udc_ep_out_intr(udc, ep_num, ep_intr_status);
            }
            ep_intr >>= 1;
            ep_num++;
        }
    }
}

static int udc_write_fifo_ep0(udc_ep_t *ep, udc_req_t *req)
{
    bool is_last;
    size_t count = udc_setdma_tx(ep, req);

    /* last packet is usually short (or a zlp) */
    if (count != ep->max_packet) {
        is_last = true;
    } else {
        if ((req->length != req->actual + count)) {
            is_last = false;
        } else {
            is_last = true;
        }
    }

    /* requests complete when all IN data is in the FIFO */
    if (is_last) {
        ep->udc->ep0_state = EP0_WAIT_FOR_SETUP;
        return 1;
    }

    return 0;
}

static int udc_fifo_read(udc_ep_t *ep, void *data, int max)
{
    return 0;
}

static void udc_set_address(udc_t *udc, uint8_t addr)
{
    writel((readl(DCFG) & ~DCFG_DEVADDR_MASK) | DCFG_DEVADDR(addr), DCFG);
    udc_ep0_zlp(udc);
}

static void udc_ep0_set_stall(udc_ep_t *ep)
{
    udc_t *udc = ep->udc;
    uint8_t ep_num = ep->addr & 0xf;
    uint32_t ep_ctrl = readl(DIEPCTL(ep_num));

    if (ep_ctrl & DXEPCTL_EPENA) {
        ep_ctrl |= DXEPCTL_EPDIS;
    }

    ep_ctrl |= DXEPCTL_STALL;

    writel(ep_ctrl, DIEPCTL(ep_num));

    udc->ep0_state = EP0_WAIT_FOR_SETUP;
    udc_pre_setup(udc);
}

static void udc_ep0_read(udc_t *udc)
{
    udc_req_t *req;
    udc_ep_t *ep = &udc->ep[0];

    if (!list_empty(&ep->queue)) {
        req = list_entry(ep->queue.next, udc_req_t, queue);
    } else {
        return;
    }

    if (req->length == 0) {
        /* zlp for Set_configuration, Set_interface,
         * or Bulk-Only mass storge reset */
        ep->len = 0;
        udc_ep0_zlp(udc);
        return;
    }

    udc_setdma_rx(ep, req);
}

static int udc_ep0_write(udc_t *udc)
{
    udc_req_t *req;
    udc_ep_t *ep = &udc->ep[0];
    int ret, need_zlp = 0;

    if (list_empty(&ep->queue)) {
        req = NULL;
    } else {
        req = list_entry(ep->queue.next, udc_req_t, queue);
    }

    if (!req) {
        return 0;
    }

    if (req->length - req->actual == udc->ep0_fifo_size) {
        need_zlp = 1;
    }

    ret = udc_write_fifo_ep0(ep, req);

    if ((ret == 1) && !need_zlp) {
        /* Last packet */
        udc->ep0_state = EP0_WAIT_FOR_COMPLETE;
    } else {
        udc->ep0_state = EP0_DATA_STATE_XMIT;
    }

    return 1;
}

static int udc_get_status(udc_t *udc)
{
    uint8_t ep_num = udc->ctrl.wIndex & 0x0f;
    uint16_t g_status = 0;

    switch (udc->ctrl.bRequestType & USB_RECIP_MASK) {
    case USB_RECIP_INTERFACE:
        g_status = 0;
        break;

    case USB_RECIP_DEVICE:
        g_status = 0x1; /* Self powered */
        break;

    case USB_RECIP_ENDPOINT:
        if (udc->ctrl.wLength > 2) {
            return 1;
        }
        g_status = udc->ep[ep_num].stopped;
        break;

    default:
        return 1;
    }

    memcpy(&udc->ctrl, &g_status, sizeof(g_status));

    writel((uint32_t) &udc->ctrl, DIEPDMA(0));
    writel(DXEPTSIZ_PKTCNT(1) | DXEPTSIZ_XFERSIZE(2), DIEPTSIZ0);
    writel(readl(DIEPCTL0) | DXEPCTL_EPENA | DXEPCTL_CNAK, DIEPCTL0);
    udc->ep0_state = EP0_WAIT_FOR_NULL_COMPLETE;

    return 0;
}

static void udc_set_nak(udc_ep_t *ep)
{
    uint8_t ep_num = ep->addr & 0xf;

    if (ep->addr & USB_DIR_IN) {
        writel(readl(DIEPCTL(ep_num)) | DXEPCTL_SNAK, DIEPCTL(ep_num));
    } else {
        writel(readl(DOEPCTL(ep_num)) | DXEPCTL_SNAK, DOEPCTL(ep_num));
    }
}

static void udc_ep_set_stall(udc_ep_t *ep)
{
    uint8_t ep_num = ep->addr & 0xf;
    uint32_t ep_ctrl = 0;

    if (ep->addr & USB_DIR_IN) {
        ep_ctrl = readl(DIEPCTL(ep_num));

        if (ep_ctrl & DXEPCTL_EPENA)
            ep_ctrl |= DXEPCTL_EPDIS;

        ep_ctrl |= DXEPCTL_STALL;

        writel(ep_ctrl, DIEPCTL(ep_num));
    } else {
        ep_ctrl = readl(DOEPCTL(ep_num));

        ep_ctrl |= DXEPCTL_STALL;

        writel(ep_ctrl, DOEPCTL(ep_num));
    }

    return;
}

static void udc_ep_clear_stall(udc_ep_t *ep)
{
    uint8_t ep_num = ep->addr & 0xf;

    if (ep->addr & USB_DIR_IN) {
        writel(readl(DIEPCTL(ep_num)) & ~(DXEPCTL_STALL | DXEPCTL_SETD0PID),
                DIEPCTL(ep_num));
    } else {
        writel(readl(DOEPCTL(ep_num)) & ~(DXEPCTL_STALL | DXEPCTL_SETD0PID),
                DOEPCTL(ep_num));
    }
}

static void udc_ep_activate(udc_ep_t *ep)
{
    uint8_t ep_num = ep->addr & 0xf;
    uint32_t ep_ctrl;
    uint32_t daintmsk;

    /* Read DEPCTLn register */
    if (ep->addr & USB_DIR_IN) {
        ep_ctrl = readl(DIEPCTL(ep_num));
        daintmsk = DAINT_INEP(ep_num);
    } else {
        ep_ctrl = readl(DOEPCTL(ep_num));
        daintmsk = DAINT_OUTEP(ep_num);
    }

    if (!(ep_ctrl & DXEPCTL_USBACTEP)) {
        ep_ctrl = (ep_ctrl & ~DXEPCTL_EPTYPE_MASK) |
            (ep->desc->bmAttributes << DXEPCTL_EPTYPE_SHIFT);
        ep_ctrl = (ep_ctrl & ~DXEPCTL_MPS_MASK) |
            (ep->max_packet << DXEPCTL_MPS_SHIFT);
        ep_ctrl |= (DXEPCTL_SETD0PID | DXEPCTL_USBACTEP | DXEPCTL_SNAK);

        if (ep->addr & USB_DIR_IN) {
            writel(ep_ctrl, DIEPCTL(ep_num));
        } else {
            writel(ep_ctrl, DOEPCTL(ep_num));
        }
    }

    /* Unmask EP Interrtupt */
    writel(readl(DAINTMSK) | daintmsk, DAINTMSK);
}

static int udc_clear_feature(udc_ep_t *ep)
{
    udc_t *udc = ep->udc;
    uint8_t ep_num = ep->addr & 0xf;

    if (udc->ctrl.wLength != 0) {
        return 1;
    }

    switch(udc->ctrl.bRequestType & USB_RECIP_MASK) {
    case USB_RECIP_DEVICE:
        udc_ep0_zlp(udc);
        break;

    case USB_RECIP_ENDPOINT:
        if (udc->ctrl.wValue == USB_ENDPOINT_HALT) {
            if (ep_num == 0) {
                udc_ep0_set_stall(ep);
                return 0;
            }

            udc_ep0_zlp(udc);

            udc_ep_clear_stall(ep);
            udc_ep_activate(ep);
            ep->stopped = false;

            udc->clear_feature_num = ep_num;
            udc->clear_feature_flag = true;
        }
        break;
    }

    return 0;
}

static int udc_set_feature(udc_ep_t *ep)
{
    udc_t *udc = ep->udc;
    uint8_t ep_num = ep->addr & 0xf;

    if (udc->ctrl.wLength != 0) {
        return 1;
    }

    switch (udc->ctrl.bRequestType & USB_RECIP_MASK) {
    case USB_RECIP_DEVICE:
        udc_ep0_zlp(udc);
        return 0;

    case USB_RECIP_ENDPOINT:
        if (udc->ctrl.wValue == USB_ENDPOINT_HALT) {
            if (ep_num == 0) {
                udc_ep0_set_stall(ep);
                return 0;
            }

            ep->stopped = true;
            udc_ep_set_stall(ep);
        }

        udc_ep0_zlp(udc);
        return 0;
    }

    return 1;
}

static void udc_ep0_setup(udc_t *udc)
{
    uint8_t ep_num;
    udc_ep_t *ep = &udc->ep[0];

    udc_nuke(ep, -EPROTO);

    udc_fifo_read(ep, &udc->ctrl, 8);

    if (udc->ctrl.bRequestType & USB_DIR_IN) {
        ep->addr |= USB_DIR_IN;
    } else {
        ep->addr &= ~USB_DIR_IN;
    }

    bool req_std = (udc->ctrl.bRequestType & USB_TYPE_MASK) ==
            USB_TYPE_STANDARD;

    if (req_std) {
        switch (udc->ctrl.bRequest) {
        case USB_REQ_SET_ADDRESS:
            if (udc->ctrl.bRequestType !=
                    (USB_TYPE_STANDARD | USB_RECIP_DEVICE)) {
                break;
            }
            udc_set_address(udc, udc->ctrl.wValue);
            return;

        case USB_REQ_GET_STATUS:
            if (!udc_get_status(udc)) {
                return;
            }
            break;

        case USB_REQ_CLEAR_FEATURE:
            ep_num = udc->ctrl.wLength & 0x0f;
            if (!udc_clear_feature(&udc->ep[ep_num])) {
                return;
            }
            break;

        case USB_REQ_SET_FEATURE:
            ep_num = udc->ctrl.wLength & 0x0f;
            if (!udc_set_feature(&udc->ep[ep_num])) {
                return;
            }
            break;
        }
    }

    if (udc->gadget) {
        int ret = udc->gadget->setup(udc, &udc->ctrl);
        if (ret < 0) {
            udc_ep0_set_stall(ep);
            udc->ep0_state = EP0_WAIT_FOR_SETUP;
        }
    }
}

static void udc_handle_ep0(udc_t *udc)
{
    if (udc->ep0_state == EP0_WAIT_FOR_SETUP) {
        udc_ep0_setup(udc);
    }
}

static void udc_ep0_kick(udc_t *udc, udc_ep_t *ep)
{
    if (ep->addr & USB_DIR_IN) {
        udc->ep0_state = EP0_DATA_STATE_XMIT;
        udc_ep0_write(udc);
    } else {
        udc->ep0_state = EP0_DATA_STATE_RECV;
        udc_ep0_read(udc);
    }
}

static void udc_reconfig(udc_t *udc)
{
    /* Reset core */
    writel(readl(GRSTCTL) | GRSTCTL_CSFTRST, GRSTCTL);
    while(readl(GRSTCTL) & GRSTCTL_CSFTRST)
        ;
    mdelay(1);
    while(!(readl(GRSTCTL) & GRSTCTL_AHBIDLE))
        ;

    /* Disconnect */
    writel(readl(DCTL) | DCTL_SFTDISCON, DCTL);

    /* Force device mode */
    writel(readl(GUSBCFG) | GUSBCFG_FORCEDEVMODE, GUSBCFG);
    mdelay(25);

    /* Reconnect */
    writel(readl(DCTL) & ~DCTL_SFTDISCON, DCTL);

    /* Unmask core interrupts */
    writel(readl(GINTMSK) | GINTSTS_OEPINT | GINTSTS_IEPINT |
            GINTSTS_ENUMDONE | GINTSTS_USBRST, GINTMSK);

    /* Set NAK bit of EP0-15 */
    writel(DXEPCTL_EPDIS | DXEPCTL_SNAK, DOEPCTL0);
    writel(DXEPCTL_EPDIS | DXEPCTL_SNAK, DIEPCTL0);

    for (uint8_t ep_num = 1; ep_num < 16; ep_num++) {
        writel(DXEPCTL_EPDIS | DXEPCTL_SNAK, DOEPCTL(ep_num));
        writel(DXEPCTL_EPDIS | DXEPCTL_SNAK, DIEPCTL(ep_num));
    }

    /* Unmask EP0 interrupts */
    writel((1 << 16) | (1 << 0), DAINTMSK);

    /* Unmask IN and OUT common interrupts */
    writel(DXEPINT_SETUP | DXEPINT_XFERCOMPL, DOEPMSK);
    writel(DXEPINT_XFERCOMPL, DIEPMSK);

    /* Setup FIFOs */
    writel(1024, GRXFSIZ);
    writel((1024 << 16) | 1024, GNPTXFSIZ);
    writel((1024 << 16) | 2048, DPTXFSIZN(1));
    writel((1024 << 16) | 3072, DPTXFSIZN(2));
    writel((1024 << 16) | 4096, DPTXFSIZN(3));
    writel((1024 << 16) | 5120, DPTXFSIZN(4));
    writel((1024 << 16) | 6144, DPTXFSIZN(5));

    /* Flush RX FIFO */
    writel(GRSTCTL_RXFFLSH, GRSTCTL);
    while(readl(GRSTCTL) & GRSTCTL_RXFFLSH)
        ;

    /* Flush all the TX FIFOs */
    writel(GRSTCTL_TXFNUM(16), GRSTCTL);
    writel(GRSTCTL_TXFNUM(16) | GRSTCTL_TXFFLSH, GRSTCTL);
    while(readl(GRSTCTL) & GRSTCTL_TXFFLSH)
        ;

    /* Clear NAK bit of EP0 */
    writel(DXEPCTL_EPDIS | DXEPCTL_CNAK, DOEPCTL0);

    writel(GAHBCFG_P_TXF_EMP_LVL | GAHBCFG_NP_TXF_EMP_LVL | GAHBCFG_DMA_EN |
            GAHBCFG_HBSTLEN_INCR4 | GAHBCFG_GLBL_INTR_EN, GAHBCFG);
}

static void udc_done(udc_ep_t *ep, udc_req_t *req, int status)
{
    bool stopped = ep->stopped;

    list_del_init(&req->queue);

    if (req->status == -EINPROGRESS) {
        req->status = status;
    }

    ep->stopped = true;
    if (req->complete) {
        req->complete(ep, req);
    }
    ep->stopped = stopped;
}

static void udc_nuke(udc_ep_t *ep, int status)
{
    while (!list_empty(&ep->queue)) {
        udc_req_t *req = list_entry(ep->queue.next, udc_req_t, queue);
        udc_done(ep, req, status);
    }
}

static void udc_stop_activity(udc_t *udc)
{
    if (udc->speed == USB_SPEED_UNKNOWN) {
        udc->gadget = NULL;
    }
    udc->speed = USB_SPEED_UNKNOWN;

    for (int ep_num = 1; ep_num < 16; ep_num++) {
        udc_ep_t *ep = &udc->ep[ep_num];
        ep->stopped = true;
        udc_nuke(ep, -ESHUTDOWN);
    }

    udc_reconfig(udc);
}

static void udc_set_max_pktsize(udc_t *udc, enum usb_device_speed speed)
{
    if (speed == USB_SPEED_HIGH) {
        udc->ep0_fifo_size = 64;
        udc->ep_fifo_size = 1024;
        udc->speed = USB_SPEED_HIGH;
    } else {
        udc->ep0_fifo_size = 64;
        udc->ep_fifo_size = 64;
        udc->speed = USB_SPEED_FULL;
    }

    udc->ep[0].max_packet = udc->ep0_fifo_size;
    for (int ep_num = 1; ep_num < 16; ep_num++) {
        udc->ep[ep_num].max_packet = udc->ep_fifo_size;
    }

    /* EP0 - Control IN (64 bytes)*/
    writel(readl(DIEPCTL0) & ~3, DIEPCTL0);

    /* EP0 - Control OUT (64 bytes)*/
    writel(readl(DOEPCTL0) & ~3, DOEPCTL0);
}

/*****************************************************************************/

static int udc_ep_enable(udc_ep_t *ep,
        const struct usb_endpoint_descriptor *desc)
{
    if (!ep || !desc || ep->desc || (ep->addr & 0xf) == 0
            || desc->bDescriptorType != USB_DT_ENDPOINT
            || ep->addr != desc->bEndpointAddress
            || ep->max_packet < desc->wMaxPacketSize) {
        return -EINVAL;
    }

    if ((desc->bmAttributes == USB_ENDPOINT_XFER_BULK &&
        desc->wMaxPacketSize > ep->max_packet) || !desc->wMaxPacketSize) {
        return -ERANGE;
    }

    udc_t *udc = ep->udc;
    if (!udc->gadget || udc->speed == USB_SPEED_UNKNOWN) {
        return -ESHUTDOWN;
    }

    ep->stopped = false;
    ep->desc = desc;
    ep->max_packet = desc->wMaxPacketSize;

    udc_set_nak(ep);
    udc_set_halt(ep, 0);

    udc_ep_activate(ep);

    return 0;
}

static int udc_ep_disable(udc_ep_t *ep)
{
    // bool reenable_irqs = disable_irqs();
    udc_nuke(ep, -ESHUTDOWN);
    ep->desc = NULL;
    ep->stopped = true;
    // if (reenable_irqs) {
    //     enable_irqs();
    // }

    return 0;
}

static udc_req_t *udc_alloc_req(udc_ep_t *ep)
{
    udc_req_t *req;

    req = calloc(sizeof(*req), 1);
    if (!req) {
        return 0;
    }

    INIT_LIST_HEAD(&req->queue);
    req->complete = udc_free_req;

    return req;
}

static void udc_free_req(udc_ep_t *ep, udc_req_t *req)
{
    free(req);
}

static int udc_queue(udc_ep_t *ep, udc_req_t *req)
{
    uint32_t ep_num = ep->addr & 0xf;

    if (!req || !req->buf || !list_empty(&req->queue)) {
        return -EINVAL;
    }

    if (!ep || (!ep->desc && ep_num != 0)) {
        return -EINVAL;
    }

    udc_t *udc = ep->udc;
    if (!ep->udc || udc->speed == USB_SPEED_UNKNOWN) {
        return -ESHUTDOWN;
    }

    req->status = -EINPROGRESS;
    req->actual = 0;

    if (list_empty(&ep->queue) && !ep->stopped) {
        if (ep_num == 0) {
            list_add_tail(&req->queue, &ep->queue);
            udc_ep0_kick(udc, ep);
            req = NULL;
        } else if (ep->addr & USB_DIR_IN) {
            udc_setdma_tx(ep, req);
        } else {
            udc_setdma_rx(ep, req);
        }
    }

    if (req != NULL) {
        list_add_tail(&req->queue, &ep->queue);
    }

    return 0;
}

static int udc_dequeue(udc_ep_t *ep, udc_req_t *_req)
{
    udc_req_t *req;

    if (!ep || (ep->addr & 0xf) == 0) {
        return -EINVAL;
    }

    list_for_each_entry(req, &ep->queue, queue) {
        if (req == _req) {
            break;
        }
    }
    if (req != _req) {
        return -EINVAL;
    }

    udc_done(ep, req, -ECONNRESET);

    return 0;
}

static int udc_set_halt(udc_ep_t *ep, int value)
{
    uint8_t ep_num = ep->addr & 0xf;
    udc_t *udc = ep->udc;

    if (!ep || !ep->desc || ep_num == 0 ||
             ep->desc->bmAttributes == USB_ENDPOINT_XFER_ISOC) {
        return -EINVAL;
    }

    if (value && (ep->addr & USB_DIR_IN) && !list_empty(&ep->queue)) {
        return -EAGAIN;
    }

    if (value == 0) {
        ep->stopped = false;
        udc_ep_clear_stall(ep);
    } else {
        if (ep_num == 0) {
            udc->ep0_state = EP0_WAIT_FOR_SETUP;
        }

        ep->stopped = true;
        udc_ep_set_stall(ep);
    }

    return 0;
}

/*****************************************************************************/

int udc_probe(void)
{
    udc.ops = &udc_ops;
    udc_reinit(&udc);
    return 0;
}

int udc_register_gadget(udc_gadget_t *gadget)
{
    if (udc.gadget) {
        udc_stop_activity(&udc);
        udc.gadget->unbind(&udc);
        udc.gadget = NULL;
        irq_disable(IRQ_OTG);
        udc_disable(&udc);
    }

    if (gadget) {
        udc.gadget = gadget;
        int retval = gadget->bind(&udc);
        if (retval) {
            udc.gadget = NULL;
            return retval;
        }
        irq_enable(IRQ_OTG);
        udc_enable(&udc);
    }

    return 0;
}
