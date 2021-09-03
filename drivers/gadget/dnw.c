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

#include <linux/errno.h>
#include "linux/usb/ch9.h"

#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define CHUNK_MAX_SIZE (16 * 1024)
#define NUM_ENDPOINTS (2)
#define NUM_STRING_DESC (3)
#define NUM_CONFIG_DESC (1)

enum {
    VEND_REQ_GET_EXECADDR,
    VEND_REQ_SET_EXECADDR,
};


typedef struct gadget_priv gadget_priv_t;
struct gadget_priv {
    uint8_t chunk[CHUNK_MAX_SIZE];
    uint16_t config;
    uint32_t chunk_num;
    void *loadaddr;
    void *execaddr;
    uint32_t length;
    uint32_t offset;
};

/* functions */
static inline int usb_req_desc(udc_t *udc, struct usb_ctrlrequest *ctrl);
static inline int usb_req_config(udc_t *udc, struct usb_ctrlrequest *ctrl);
static inline int usb_req_iface(udc_t *udc, struct usb_ctrlrequest *ctrl);
static inline int vend_req_execaddr(udc_t *udc, struct usb_ctrlrequest *ctrl);
static inline void set_config(udc_t *udc, int config);
static void configured(udc_t *udc);

/* descriptors */
struct usb_device_config_descriptor {
    struct usb_config_descriptor cfg;
    struct usb_interface_descriptor if0;
    struct usb_endpoint_descriptor ep[];
} __attribute__((packed));

static const struct usb_device_descriptor sec_dths_dev;
static const struct usb_qualifier_descriptor sec_dths_qual;
static struct usb_device_config_descriptor sec_dths_config;
static const struct usb_device_descriptor sec_dtfs_dev;
static const struct usb_qualifier_descriptor sec_dtfs_qual;
static struct usb_device_config_descriptor sec_dtfs_config;
static const struct usb_string_descriptor *sec_dt_string[];

int bind(udc_t *udc)
{
    gadget_priv_t *priv;

    priv = calloc(sizeof(*priv), 1);
    if (!priv) {
        return -ENOMEM;
    }

    udc->gadget_data = priv;

    return 0;
}

void unbind(udc_t *udc)
{
    gadget_priv_t *priv = udc->gadget_data;
    free(priv);
    udc->gadget_data = NULL;
}

int setup(udc_t *udc, struct usb_ctrlrequest *ctrl)
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

    case USB_TYPE_VENDOR:
        switch (ctrl->bRequest) {
        case VEND_REQ_GET_EXECADDR:
        case VEND_REQ_SET_EXECADDR:
            if ((ctrl->bRequestType & USB_RECIP_MASK) != USB_RECIP_DEVICE) {
                break;
            }
            return vend_req_execaddr(udc, ctrl);
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

static inline int vend_req_execaddr(udc_t *udc, struct usb_ctrlrequest *ctrl)
{
    gadget_priv_t *priv = (gadget_priv_t *) udc->gadget_data;
    udc_req_t *req = NULL;

    switch (ctrl->bRequest) {
    case VEND_REQ_GET_EXECADDR:
    case VEND_REQ_SET_EXECADDR:
        req = udc->ops->alloc_req(&udc->ep[0]);
        req->buf = &priv->execaddr;
        req->length = sizeof(priv->execaddr);
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
    struct usb_endpoint_descriptor *desc[NUM_ENDPOINTS];

    priv->config = config;

    if (config == 0) {
        for (int i = 0; i < NUM_ENDPOINTS; i++) {
            udc->ops->ep_disable(&udc->ep[i]);
        }
    } else if (config == 1) {
        for (int i = 0; i < NUM_ENDPOINTS; i++) {
            if (udc->speed == USB_SPEED_HIGH) {
                desc[i] = &sec_dths_config.ep[i];
            } else {
                desc[i] = &sec_dtfs_config.ep[i];
            }
            udc->ep[i + 1].addr = desc[i]->bEndpointAddress;
            udc->ops->ep_enable(&udc->ep[1 + 1], desc[i]);
        }
        configured(udc);
    }
}

static void ep2_complete(udc_ep_t *ep, udc_req_t *req)
{
    udc_t *udc = ep->udc;
    gadget_priv_t *priv = (gadget_priv_t *) udc->gadget_data;

    if (req->status) {
        udc->ops->free_req(ep, req);
        return;
    }

    if (priv->chunk_num == 0 && req->actual >= 10) {
        memcpy(&priv->loadaddr, priv->chunk, sizeof(priv->loadaddr));
        memcpy(&priv->length, priv->chunk + 4, sizeof(priv->length));
        priv->length -= 10;
        memcpy(priv->loadaddr, priv->chunk + 8, req->actual - 8);
        priv->offset = req->actual - 8;
    } else {
        memcpy(priv->loadaddr + priv->offset, priv->chunk, req->actual);
        priv->offset += req->actual;
    }

    if (priv->offset >= priv->length) {
        udc->ops->free_req(ep, req);

        udc_register_gadget(NULL);
        disable_irqs();
        icache_invalidate();
        if (priv->execaddr) {
            ((void (*)(void))priv->execaddr)();
        } else {
            ((void (*)(void))priv->loadaddr)();
        }
    }

    priv->chunk_num++;
    req->actual = 0;
    udc->ops->queue(ep, req);
}

static void configured(udc_t *udc)
{
    gadget_priv_t *priv = (gadget_priv_t *) udc->gadget_data;

    if (list_empty(&udc->ep[2].queue)) {
        udc_req_t *req = udc->ops->alloc_req(&udc->ep[2]);
        req->buf = priv->chunk;
        req->length = sizeof(priv->chunk);
        req->complete = ep2_complete;
        udc->ops->queue(&udc->ep[2], req);
    }
}

#define MANUFACTURER_STRING u"Jeff Kent <jeff@jkent.net>"
#define PRODUCT_STRING u"Nanoboot DNW"

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
    .bNumConfigurations = 1,
};

__attribute__((aligned(4)))
static struct usb_device_config_descriptor sec_dths_config = {
    .cfg = {
        .bLength             = USB_DT_CONFIG_SIZE,
        .bDescriptorType     = USB_DT_CONFIG,
        .wTotalLength        = USB_DT_CONFIG_SIZE +
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
        .bNumEndpoints       = 2,
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
    .bNumConfigurations = 1,
};

__attribute__((aligned(4)))
static struct usb_device_config_descriptor sec_dtfs_config = {
    .cfg = {
        .bLength             = USB_DT_CONFIG_SIZE,
        .bDescriptorType     = USB_DT_CONFIG,
        .wTotalLength        = USB_DT_CONFIG_SIZE + USB_DT_INTERFACE_SIZE +
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
        .bNumEndpoints       = 2,
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

static const struct usb_string_descriptor *sec_dt_string[] = {
    &str0_descriptor,
    &str1_descriptor,
    &str2_descriptor,
};
