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

#include "dnw.h"
#include "udc.h"

#include <cache.h>
#include <irq.h>
#include <macros.h>
#include <halt.h>

#include <linux/errno.h>
#include "linux/usb/ch9.h"

#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define CHUNK_MAX_SIZE (16 * 1024)
#define NUM_STRING_DESC 3
#define NUM_CONFIG_DESC 1
#define NUM_IF 2
#define NUM_IF0_EP 2
#define NUM_IF1_EP 2

typedef struct dnw_data dnw_data_t;
typedef struct adv_data adv_data_t;
typedef struct gadget_priv gadget_priv_t;

struct dnw_data {
    uint8_t chunk[CHUNK_MAX_SIZE];
    size_t load_addr;
    size_t length;
    size_t offset;
    unsigned first_chunk : 1;
};

typedef enum adv_state {
    ADV_STATE_COMMAND,
    ADV_STATE_RX_DATA,
    ADV_STATE_TX_DATA,
} adv_state_t;

struct adv_data {
    uint8_t buf[256];
    adv_state_t state;
    uint32_t checksum;
};

struct gadget_priv {
    uint16_t config;
    dnw_data_t dnw;
    adv_data_t adv;
};

/* functions */
static inline int usb_req_desc(udc_t *udc, struct usb_ctrlrequest *ctrl);
static inline int usb_req_config(udc_t *udc, struct usb_ctrlrequest *ctrl);
static inline int usb_req_iface(udc_t *udc, struct usb_ctrlrequest *ctrl);
static inline void set_config(udc_t *udc, int config);
static void configured(udc_t *udc);

/* descriptors */
struct usb_device_config_descriptor {
    struct usb_config_descriptor cfg;
    struct usb_interface_descriptor if0;
    struct usb_endpoint_descriptor if0_ep[NUM_IF0_EP];
    struct usb_interface_descriptor if1;
    struct usb_endpoint_descriptor if1_ep[NUM_IF1_EP];
} __attribute__((packed));

static const struct usb_device_descriptor sec_dths_dev;
static const struct usb_qualifier_descriptor sec_dths_qual;
static struct usb_device_config_descriptor sec_dths_config;
static const struct usb_device_descriptor sec_dtfs_dev;
static const struct usb_qualifier_descriptor sec_dtfs_qual;
static struct usb_device_config_descriptor sec_dtfs_config;
static const struct usb_string_descriptor *sec_dt_string[];

static int bind(udc_t *udc)
{
    gadget_priv_t *priv;

    priv = calloc(sizeof(*priv), 1);
    if (!priv) {
        return -ENOMEM;
    }

    udc->gadget_data = priv;

    return 0;
}

static void unbind(udc_t *udc)
{
    gadget_priv_t *priv = udc->gadget_data;
    free(priv);
    udc->gadget_data = NULL;
}

static int setup(udc_t *udc, struct usb_ctrlrequest *ctrl)
{

    switch (ctrl->bRequestType & USB_TYPE_MASK) {
    case USB_TYPE_STANDARD:
        switch (ctrl->bRequest) {
        case USB_REQ_GET_DESCRIPTOR:
            if ((ctrl->bRequestType & USB_RECIP_MASK) != USB_RECIP_DEVICE) {
                break;
            }
            return usb_req_desc(udc, ctrl);

        case USB_REQ_GET_CONFIGURATION:
        case USB_REQ_SET_CONFIGURATION:
            if ((ctrl->bRequestType & USB_RECIP_MASK) != USB_RECIP_DEVICE) {
                break;
            }
            return usb_req_config(udc, ctrl);

        case USB_REQ_GET_INTERFACE:
        case USB_REQ_SET_INTERFACE:
            if ((ctrl->bRequestType & USB_RECIP_MASK) != USB_RECIP_INTERFACE) {
                break;
            }
            return usb_req_iface(udc, ctrl);
        }
    }

    return -1;
}

udc_gadget_t dnw_gadget = {
    .bind = bind,
    .unbind = unbind,
    .setup = setup,
};

/*****************************************************************************/

static inline int usb_req_desc(udc_t *udc, struct usb_ctrlrequest *ctrl)
{
    udc_req_t *req = NULL;

    switch (ctrl->wValue >> 8) {
    case USB_DT_DEVICE:
        req = udc->ops->alloc_req(&udc->ep[0]);
        if (udc->speed == USB_SPEED_HIGH) {
            req->buf = (uint16_t *) &sec_dths_dev;
            req->length = sec_dths_dev.bLength;
        } else {
            req->buf = (uint16_t *) &sec_dtfs_dev;
            req->length = sec_dtfs_dev.bLength;
        }
        break;

    case USB_DT_DEVICE_QUALIFIER:
        req = udc->ops->alloc_req(&udc->ep[0]);
        if (udc->speed == USB_SPEED_HIGH) {
            req->buf = (uint16_t *) &sec_dths_qual;
            req->length = sec_dths_qual.bLength;
        } else {
            req->buf = (uint16_t *) &sec_dtfs_qual;
            req->length = sec_dtfs_qual.bLength;
        }
        break;

    case USB_DT_OTHER_SPEED_CONFIG:
        req = udc->ops->alloc_req(&udc->ep[0]);
        if (udc->speed == USB_SPEED_HIGH) {
            sec_dtfs_config.cfg.bDescriptorType = USB_DT_OTHER_SPEED_CONFIG;
            req->buf = (uint16_t *) &sec_dtfs_dev;
            req->length = sec_dtfs_dev.bLength;
        } else {
            sec_dths_config.cfg.bDescriptorType = USB_DT_OTHER_SPEED_CONFIG;
            req->buf = (uint16_t *) &sec_dths_dev;
            req->length = sec_dths_dev.bLength;
        }
        break;

    case USB_DT_CONFIG:
        req = udc->ops->alloc_req(&udc->ep[0]);
        if (udc->speed == USB_SPEED_HIGH) {
            sec_dths_config.cfg.bDescriptorType = USB_DT_CONFIG;
            req->buf = (uint16_t *) &sec_dths_config;
            req->length = sec_dths_config.cfg.wTotalLength;
        } else {
            sec_dtfs_config.cfg.bDescriptorType = USB_DT_CONFIG;
            req->buf = (uint16_t *) &sec_dtfs_config;
            req->length = sec_dtfs_config.cfg.wTotalLength;
        }
        break;

    case USB_DT_STRING: {
        uint8_t i = ctrl->wValue & 0xFF;
        if (i >= NUM_STRING_DESC)
            return -1;
        req = udc->ops->alloc_req(&udc->ep[0]);
        req->buf = (uint16_t *)sec_dt_string[i];
        req->length = sec_dt_string[i]->bLength;
        break;
    }
    }

    if (req) {
        req->length = MIN((size_t) ctrl->wLength, req->length);
        udc->ops->queue(&udc->ep[0], req);
        return 0;
    }

    return -1;
}

static inline int usb_req_config(udc_t *udc, struct usb_ctrlrequest *ctrl)
{
    gadget_priv_t *priv = (gadget_priv_t *) udc->gadget_data;
    udc_req_t *req = NULL;

    switch (ctrl->bRequest) {
    case USB_REQ_SET_CONFIGURATION:
        if (ctrl->wValue > NUM_CONFIG_DESC) {
            return -1;
        }
        set_config(udc, ctrl->wValue);

        req = udc->ops->alloc_req(&udc->ep[0]);
        req->buf = &priv->config;
        break;

    case USB_REQ_GET_CONFIGURATION:
        req = udc->ops->alloc_req(&udc->ep[0]);
        req->buf = &priv->config;
        req->length = 1;
        break;
    }

    if (req) {
        req->length = MIN((size_t) ctrl->wLength, req->length);
        udc->ops->queue(&udc->ep[0], req);
        return 0;
    }

    return -1;
}

static inline int usb_req_iface(udc_t *udc, struct usb_ctrlrequest *ctrl)
{
    gadget_priv_t *priv = (gadget_priv_t *) udc->gadget_data;
    uint8_t interface = ctrl->wIndex & 0xff;
    uint8_t alternate = ctrl->wValue & 0xff;

    udc_req_t *req;

    switch (ctrl->bRequest) {
    case USB_REQ_SET_INTERFACE:
        if (interface || alternate)
            return -1;
        return 0;

    case USB_REQ_GET_INTERFACE:
        req = udc->ops->alloc_req(&udc->ep[0]);
        priv->config = 0;
        req->buf = &priv->config;
        req->length = 1;
        break;
    }

    if (req) {
        req->length = MIN((size_t) ctrl->wLength, req->length);
        udc->ops->queue(&udc->ep[0], req);
        return 0;
    }

    return -1;
}

static inline void set_config(udc_t *udc, int config)
{
    gadget_priv_t *priv = (gadget_priv_t *) udc->gadget_data;
    struct usb_endpoint_descriptor *desc;

    priv->config = config;

    if (config == 1) {
        for (int i = 0; i < NUM_IF0_EP; i++) {
            if (udc->speed == USB_SPEED_HIGH) {
                desc = &sec_dths_config.if0_ep[i];
            } else {
                desc = &sec_dtfs_config.if0_ep[i];
            }
            udc->ep[i + 1].addr = desc->bEndpointAddress;
            udc->ops->ep_enable(&udc->ep[1 + 1], desc);
        }
        for (int i = NUM_IF0_EP + 0; i < NUM_IF0_EP + NUM_IF1_EP; i++) {
            if (udc->speed == USB_SPEED_HIGH) {
                desc = &sec_dths_config.if1_ep[i];
            } else {
                desc = &sec_dtfs_config.if1_ep[i];
            }
            udc->ep[i + 1].addr = desc->bEndpointAddress;
            udc->ops->ep_enable(&udc->ep[1 + 1], desc);
        }
        configured(udc);
    } else {
        for (int i = 0; i < NUM_IF0_EP + NUM_IF1_EP; i++) {
            udc->ops->ep_disable(&udc->ep[i]);
        }
    }
}

static void dnw_rx_complete(udc_ep_t *ep, udc_req_t *req)
{
    udc_t *udc = ep->udc;
    gadget_priv_t *priv = (gadget_priv_t *) udc->gadget_data;

    if (req->status) {
        udc->ops->free_req(ep, req);
        return;
    }

    if (priv->dnw.first_chunk && req->actual >= 10) {
        priv->dnw.load_addr = *(uint32_t *) req->buf;
        priv->dnw.length = *(uint32_t *) (req->buf + 4);
        priv->dnw.length -= 8;
        memcpy((void *) priv->dnw.load_addr, req->buf + 8, req->actual - 8);
        priv->dnw.offset = req->actual - 8;
        priv->dnw.first_chunk = false;
    } else {
        memcpy((void *) priv->dnw.load_addr + priv->dnw.offset, req->buf,
                req->actual);
        priv->dnw.offset += req->actual;
    }

    if (priv->dnw.offset >= priv->dnw.length) {
        udc->ops->free_req(ep, req);
        udc_register_gadget(NULL);
        disable_irqs();
        icache_invalidate();
        ((void (*)(void)) priv->dnw.load_addr)();
    }

    req->actual = 0;
    udc->ops->queue(ep, req);
}

static void adv_rx_complete(udc_ep_t *ep, udc_req_t *req)
{
    udc_t *udc = ep->udc;
    gadget_priv_t *priv = (gadget_priv_t *) udc->gadget_data;
    char c, *p, *argv[3];
    int argc;
    size_t address, length, checksum;

    switch (priv->adv.state) {
    case ADV_STATE_COMMAND:
        length = MIN(req->length - 1, req->actual);
        ((uint8_t *) req->buf)[length] = '\0';
        p = req->buf;
        while (*p && *p == ' ') {
            p++;
        }
        c = ' ';
        for (argc = 0; argc < 3 && c == ' '; argc++) {
            argv[argc] = p;
            while (*p && *p != ' ') {
                p++;
            }
            c = *p;
            *p = '\0';
            while (*p && *p == ' ') {
                p++;
            }
        }
        if (argc == 0) {
            return;
        }

        if (strcasecmp("memwrite", argv[0]) == 0 && argc == 2) {
            priv->adv.state = ADV_STATE_RX_DATA;
            address = strtoul(argv[1], NULL, 0);
            req->buf = (void *) address;
            req->actual = 0;
            udc->ops->queue(ep, req);
        } else if (strcasecmp("memread", argv[0]) == 0 && argc == 2) {
            priv->adv.state = ADV_STATE_TX_DATA;
            address = strtoul(argv[1], NULL, 0);
            length = strtoul(argv[2], NULL, 0);
            udc_req_t *newreq = udc->ops->alloc_req(&udc->ep[3]);
            newreq->buf = (void *) address;
            newreq->length = length;
            udc->ops->queue(&udc->ep[3], newreq);
        } else if (strcasecmp("checksum", argv[0]) == 0 && argc == 3) {
            priv->adv.state = ADV_STATE_TX_DATA;
            address = strtoul(argv[1], NULL, 0);
            length = strtoul(argv[2], NULL, 0);
            checksum = 0;
            while (length--) {
                checksum += *((uint8_t *) address++);
            }
            udc_req_t *newreq = udc->ops->alloc_req(&udc->ep[3]);
            newreq->buf = priv->adv.buf;
            newreq->length = sprintf(newreq->buf, "0x%08x", checksum);
            udc->ops->queue(&udc->ep[3], newreq);
        } else if (strcasecmp("execute", argv[0]) == 0 && argc == 2) {
            address = strtoul(argv[0], NULL, 0);
            udc->ops->free_req(ep, req);
            udc_register_gadget(NULL);
            disable_irqs();
            icache_invalidate();
            ((void (*)(void)) address)();
            error_halt();
        }
        break;

    case ADV_STATE_RX_DATA:
        priv->adv.state = ADV_STATE_COMMAND;
        req->buf = priv->adv.buf;
        req->actual = 0;
        udc->ops->queue(ep, req);
        break;

    default:
        break;
    }
}

static void configured(udc_t *udc)
{
    gadget_priv_t *priv = (gadget_priv_t *) udc->gadget_data;

    /* DNW OUT ep */
    udc_req_t *req = udc->ops->alloc_req(&udc->ep[2]);
    req->buf = priv->dnw.chunk;
    req->length = sizeof(priv->dnw.chunk);
    req->complete = dnw_rx_complete;
    priv->dnw.first_chunk = true;
    udc->ops->queue(&udc->ep[2], req);

    req = udc->ops->alloc_req(&udc->ep[4]);
    req->buf = priv->adv.buf;
    req->length = sizeof(priv->adv.buf);
    req->complete = adv_rx_complete;
    udc->ops->queue(&udc->ep[4], req);
}

#define MANUFACTURER_STRING u"Jeff Kent <jeff@jkent.net>"
#define PRODUCT_STRING u"SAMSUNG DNW + Nanoboot"

/* High speed descriptors */
__attribute__((aligned(4)))
static const struct usb_device_descriptor sec_dths_dev = {
    .bLength            = USB_DT_DEVICE_SIZE,
    .bDescriptorType    = USB_DT_DEVICE,
    .bcdUSB             = 0x0200,
    .bMaxPacketSize0    = 64,
    .idVendor           = 0x04e8,
    .idProduct          = 0x1234,
    .iManufacturer      = 1,
    .iProduct           = 2,
    .bNumConfigurations = NUM_CONFIG_DESC,
};

__attribute__((aligned(4)))
static const struct usb_qualifier_descriptor sec_dths_qual = {
    .bLength            = USB_DT_DEVICE_QUALIFIER_SIZE,
    .bDescriptorType    = USB_DT_DEVICE_QUALIFIER,
    .bcdUSB             = 0x0200,
    .bMaxPacketSize0    = 8,
    .bNumConfigurations = NUM_CONFIG_DESC,
};

__attribute__((aligned(4)))
static struct usb_device_config_descriptor sec_dths_config = {
    .cfg = {
        .bLength             = USB_DT_CONFIG_SIZE,
        .bDescriptorType     = USB_DT_CONFIG,
        .wTotalLength        = USB_DT_CONFIG_SIZE +
                               USB_DT_INTERFACE_SIZE +
                               (USB_DT_ENDPOINT_SIZE * 2) +
                               USB_DT_INTERFACE_SIZE +
                               (USB_DT_ENDPOINT_SIZE * 2),
        .bNumInterfaces      = NUM_IF,
        .bConfigurationValue = 1,
        .bmAttributes        = USB_CONFIG_ATT_ONE |
                               USB_CONFIG_ATT_SELFPOWER,
    },
    .if0 = {
        .bLength             = USB_DT_INTERFACE_SIZE,
        .bDescriptorType     = USB_DT_INTERFACE,
        .bInterfaceNumber    = 0,
        .bNumEndpoints       = NUM_IF0_EP,
        .bInterfaceClass     = 255,
    },
    {
        {
            .bLength             = USB_DT_ENDPOINT_SIZE,
            .bDescriptorType     = USB_DT_ENDPOINT,
            .bEndpointAddress    = 1 | USB_DIR_IN,
            .bmAttributes        = USB_ENDPOINT_XFER_BULK,
            .wMaxPacketSize      = 512,
        },
        {
            .bLength             = USB_DT_ENDPOINT_SIZE,
            .bDescriptorType     = USB_DT_ENDPOINT,
            .bEndpointAddress    = 2 | USB_DIR_OUT,
            .bmAttributes        = USB_ENDPOINT_XFER_BULK,
            .wMaxPacketSize      = 512,
        },
    },
    .if1 = {
        .bLength             = USB_DT_INTERFACE_SIZE,
        .bDescriptorType     = USB_DT_INTERFACE,
        .bInterfaceNumber    = 1,
        .bNumEndpoints       = NUM_IF1_EP,
        .bInterfaceClass     = 255,
    },
    {
        {
            .bLength             = USB_DT_ENDPOINT_SIZE,
            .bDescriptorType     = USB_DT_ENDPOINT,
            .bEndpointAddress    = 3 | USB_DIR_IN,
            .bmAttributes        = USB_ENDPOINT_XFER_BULK,
            .wMaxPacketSize      = 512,
        },
        {
            .bLength             = USB_DT_ENDPOINT_SIZE,
            .bDescriptorType     = USB_DT_ENDPOINT,
            .bEndpointAddress    = 4 | USB_DIR_OUT,
            .bmAttributes        = USB_ENDPOINT_XFER_BULK,
            .wMaxPacketSize      = 512,
        },
    },
};

/* Full speed descriptors */
__attribute__((aligned(4)))
static const struct usb_device_descriptor sec_dtfs_dev = {
    .bLength            = USB_DT_DEVICE_SIZE,
    .bDescriptorType    = USB_DT_DEVICE,
    .bcdUSB             = 0x0200,
    .bMaxPacketSize0    = 8,
    .idVendor           = 0x04e8,
    .idProduct          = 0x1234,
    .iManufacturer      = 1,
    .iProduct           = 2,
    .bNumConfigurations = NUM_CONFIG_DESC,
};

__attribute__((aligned(4)))
static const struct usb_qualifier_descriptor sec_dtfs_qual = {
    .bLength            = USB_DT_DEVICE_QUALIFIER_SIZE,
    .bDescriptorType    = USB_DT_DEVICE_QUALIFIER,
    .bcdUSB             = 0x0200,
    .bMaxPacketSize0    = 64,
    .bNumConfigurations = NUM_CONFIG_DESC,
};

__attribute__((aligned(4)))
static struct usb_device_config_descriptor sec_dtfs_config = {
    .cfg = {
        .bLength             = USB_DT_CONFIG_SIZE,
        .bDescriptorType     = USB_DT_CONFIG,
        .wTotalLength        = USB_DT_CONFIG_SIZE + USB_DT_INTERFACE_SIZE +
                               (USB_DT_ENDPOINT_SIZE * 2) +
                               USB_DT_INTERFACE_SIZE +
                               (USB_DT_ENDPOINT_SIZE * 2),
        .bNumInterfaces      = 1,
        .bConfigurationValue = 1,
        .bmAttributes        = USB_CONFIG_ATT_ONE |
                               USB_CONFIG_ATT_SELFPOWER,
    },
    .if0 = {
        .bLength             = USB_DT_INTERFACE_SIZE,
        .bDescriptorType     = USB_DT_INTERFACE,
        .bInterfaceNumber    = 0,
        .bNumEndpoints       = NUM_IF0_EP,
    },
    {
        {
            .bLength             = USB_DT_ENDPOINT_SIZE,
            .bDescriptorType     = USB_DT_ENDPOINT,
            .bEndpointAddress    = 1 | USB_DIR_IN,
            .bmAttributes        = USB_ENDPOINT_XFER_BULK,
            .wMaxPacketSize      = 64,
        },
        {
            .bLength             = USB_DT_ENDPOINT_SIZE,
            .bDescriptorType     = USB_DT_ENDPOINT,
            .bEndpointAddress    = 2 | USB_DIR_OUT,
            .bmAttributes        = USB_ENDPOINT_XFER_BULK,
            .wMaxPacketSize      = 64,
        },
    },
    .if1 = {
        .bLength             = USB_DT_INTERFACE_SIZE,
        .bDescriptorType     = USB_DT_INTERFACE,
        .bInterfaceNumber    = 1,
        .bNumEndpoints       = NUM_IF1_EP,
    },
    {
        {
            .bLength             = USB_DT_ENDPOINT_SIZE,
            .bDescriptorType     = USB_DT_ENDPOINT,
            .bEndpointAddress    = 1 | USB_DIR_IN,
            .bmAttributes        = USB_ENDPOINT_XFER_BULK,
            .wMaxPacketSize      = 64,
        },
        {
            .bLength             = USB_DT_ENDPOINT_SIZE,
            .bDescriptorType     = USB_DT_ENDPOINT,
            .bEndpointAddress    = 2 | USB_DIR_OUT,
            .bmAttributes        = USB_ENDPOINT_XFER_BULK,
            .wMaxPacketSize      = 64,
        },
    },
};

/* String descriptors */
__attribute__((aligned(4)))
static const struct usb_string_descriptor str0_descriptor = {
    .bLength         = 2 + 1*2,
    .bDescriptorType = USB_DT_STRING,
    .wData           = { 0x0409 },
};

__attribute__((aligned(4)))
static const struct usb_string_descriptor str1_descriptor = {
    .bLength         = 2 + (sizeof(MANUFACTURER_STRING) - 2),
    .bDescriptorType = USB_DT_STRING,
    .wData           = { MANUFACTURER_STRING },
};

__attribute__((aligned(4)))
static const struct usb_string_descriptor str2_descriptor = {
    .bLength         = 2 + (sizeof(PRODUCT_STRING) - 2),
    .bDescriptorType = USB_DT_STRING,
    .wData           = { PRODUCT_STRING },
};

__attribute__((aligned(4)))
static const struct usb_string_descriptor *sec_dt_string[] = {
    &str0_descriptor,
    &str1_descriptor,
    &str2_descriptor,
};
