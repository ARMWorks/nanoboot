/* SPDX-License-Identifier: GPL-2.0 */
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

#include "linux/usb/ch9.h"

#include "sec_gadget.h"
#include "udc.h"

#include <stdlib.h>
#include <string.h>

#define NUM_STRING_DESC 3
#define NUM_CONFIG_DESC 1

#define CMDBUF_SIZE (512)

#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
      __typeof__ (b) _b = (b); \
      _a < _b ? _a : _b; })

#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type, member) );})
#define gadget_to_priv(container) container_of(container, udc_gadget_priv_t, \
		gadget)

struct usb_device_config_descriptor {
	struct usb_config_descriptor cfg;
	struct usb_interface_descriptor if0;
	struct usb_endpoint_descriptor ep1;
	struct usb_endpoint_descriptor ep2;
} __attribute__((packed));

typedef struct udc_gadget_priv_t udc_gadget_priv_t;
struct udc_gadget_priv_t {
	udc_gadget_t gadget;
  	udc_req_t setup_req;
	udc_req_t command_req;
	udc_req_t buffer_req;
};

/* functions */
static void gadget_init(udc_t *udc);
static void gadget_free(udc_t *udc);
static int gadget_setup(udc_t *udc, struct usb_ctrlrequest *ctrl);
static void command_request(udc_ep_t *ep, udc_req_t *req);
static void command_response(udc_ep_t *ep, udc_req_t *req);
static void buffer_req_complete(udc_ep_t *ep, udc_req_t *req);
static inline int process_req_desc(udc_t *udc, struct usb_ctrlrequest *ctrl);
static inline int process_req_config(udc_t *udc, struct usb_ctrlrequest *ctrl);
static inline int process_req_iface(udc_t *udc, struct usb_ctrlrequest *ctrl);
static inline void set_config(udc_t *udc, int config);
static void configured(udc_t *udc);

udc_gadget_t sec_gadget = {
	.init = gadget_init,
	.free = gadget_free,
	.setup = gadget_setup,
};

/* descriptors */
static const struct usb_device_descriptor sec_dths_dev;
static const struct usb_qualifier_descriptor sec_dths_qual;
static struct usb_device_config_descriptor sec_dths_config;
static const struct usb_device_descriptor sec_dtfs_dev;
static const struct usb_qualifier_descriptor sec_dtfs_qual;
struct usb_device_config_descriptor sec_dtfs_config;
static const struct usb_string_descriptor *sec_dt_string[];

static void gadget_init(udc_t *udc)
{
	udc->gadget = malloc(sizeof(udc_gadget_priv_t));
	if (udc->gadget == NULL) {
		return;
	}
	udc_gadget_priv_t *priv = gadget_to_priv(udc->gadget);
	memcpy(&priv->gadget, &sec_gadget, sizeof(sec_gadget));

	priv->command_req.buf = malloc(CMDBUF_SIZE);
	if (priv->command_req.buf == NULL) {
		free(&priv->gadget);
		udc->gadget = NULL;
		return;
	}

	priv->command_req.length = CMDBUF_SIZE - 2;
	priv->command_req.complete = command_request;
	INIT_LIST_HEAD(&priv->command_req.queue);

	priv->buffer_req.complete = buffer_req_complete;
	INIT_LIST_HEAD(&priv->buffer_req.queue);
}

static void gadget_free(udc_t *udc)
{
	udc_gadget_priv_t *priv = gadget_to_priv(udc->gadget);
	free(priv->command_req.buf);
	free(priv);
	udc->gadget = NULL;
}

static int gadget_setup(udc_t *udc, struct usb_ctrlrequest *ctrl)
{
	if ((ctrl->bRequestType & USB_TYPE_MASK) != USB_TYPE_STANDARD)
		return -1;

	switch (ctrl->bRequest) {
	case USB_REQ_GET_DESCRIPTOR:
		if ((ctrl->bRequestType & USB_RECIP_MASK) != USB_RECIP_DEVICE)
			break;
		return process_req_desc(udc, ctrl);

	case USB_REQ_GET_CONFIGURATION:
	case USB_REQ_SET_CONFIGURATION:
		if ((ctrl->bRequestType & USB_RECIP_MASK) != USB_RECIP_DEVICE)
			break;
		return process_req_config(udc, ctrl);

	case USB_REQ_GET_INTERFACE:
	case USB_REQ_SET_INTERFACE:
		if ((ctrl->bRequestType & USB_RECIP_MASK) != USB_RECIP_INTERFACE)
			break;
		return process_req_iface(udc, ctrl);
	}
	return -1;
}

static void command_request(udc_ep_t *ep, udc_req_t *req)
{
	if (req->status) {
		return;
	}

	//udc_gadget_priv_t *priv = gadget_to_priv(ep->udc->gadget);
	/* TODO */
}

static void command_response(udc_ep_t *ep, udc_req_t *req)
{
	if (req->status) {
		return;
	}

	udc_gadget_priv_t *priv = gadget_to_priv(ep->udc->gadget);
	req->buf = priv->command_req.buf;
	req->length = CMDBUF_SIZE - 2;
	req->complete = command_request;
	ep->ops->queue(&ep->udc->ep[2], req);
}

static void buffer_req_complete(udc_ep_t *ep, udc_req_t *req)
{
	if (req->status) {
		return;
	}

	udc_gadget_priv_t *priv = gadget_to_priv(ep->udc->gadget);
	ep->ops->queue(&ep->udc->ep[2], &priv->command_req);
}

static inline int process_req_desc(udc_t *udc, struct usb_ctrlrequest *ctrl)
{
	udc_gadget_priv_t *priv = gadget_to_priv(udc->gadget);
	int i;

	switch (ctrl->wValue >> 8) {
	case USB_DT_DEVICE:
		if (udc->speed == USB_SPEED_HIGH) {
			priv->setup_req.buf = (uint16_t *)&sec_dths_dev;
			priv->setup_req.length = sec_dths_dev.bLength;
		} else {
			priv->setup_req.buf = (uint16_t *)&sec_dtfs_dev;
			priv->setup_req.length = sec_dtfs_dev.bLength;
		}
		break;

	case USB_DT_DEVICE_QUALIFIER:
		if (udc->speed == USB_SPEED_HIGH) {
			priv->setup_req.buf = (uint16_t *)&sec_dths_qual;
			priv->setup_req.length = sec_dths_qual.bLength;
		} else {
			priv->setup_req.buf = (uint16_t *)&sec_dtfs_qual;
			priv->setup_req.length = sec_dtfs_qual.bLength;
		}
		break;

	case USB_DT_OTHER_SPEED_CONFIG:
		if (udc->speed == USB_SPEED_HIGH) {
			sec_dtfs_config.cfg.bDescriptorType =
					USB_DT_OTHER_SPEED_CONFIG;
			priv->setup_req.buf = (uint16_t *)&sec_dtfs_dev;
			priv->setup_req.length = sec_dtfs_dev.bLength;
		} else {
			sec_dths_config.cfg.bDescriptorType =
					USB_DT_OTHER_SPEED_CONFIG;
			priv->setup_req.buf = (uint16_t *)&sec_dths_dev;
			priv->setup_req.length = sec_dths_dev.bLength;
		}
		break;

	case USB_DT_CONFIG:
		if (udc->speed == USB_SPEED_HIGH) {
			sec_dths_config.cfg.bDescriptorType = USB_DT_CONFIG;
			priv->setup_req.buf = (uint16_t *)&sec_dths_config;
			priv->setup_req.length = sec_dths_config.cfg.wTotalLength;
		} else {
			sec_dtfs_config.cfg.bDescriptorType = USB_DT_CONFIG;
			priv->setup_req.buf = (uint16_t *)&sec_dtfs_config;
			priv->setup_req.length = sec_dtfs_config.cfg.wTotalLength;
		}
		break;

	case USB_DT_STRING:
		i = ctrl->wValue & 0xFF;
		if (i >= NUM_STRING_DESC)
			return -1;
		priv->setup_req.buf = (uint16_t *)sec_dt_string[i];
		priv->setup_req.length = sec_dt_string[i]->bLength;
		break;

	default:
		return -1;
	}

	INIT_LIST_HEAD(&priv->setup_req.queue);
	priv->setup_req.length = min((uint32_t)ctrl->wLength,
			priv->setup_req.length);
	udc->ep[0].ops->queue(&udc->ep[0], &priv->setup_req);
	return 0;
}

static inline int process_req_config(udc_t *udc, struct usb_ctrlrequest *ctrl)
{
	udc_gadget_priv_t *priv = gadget_to_priv(udc->gadget);

	if (ctrl->bRequest == USB_REQ_SET_CONFIGURATION) {
		if (ctrl->wValue > NUM_CONFIG_DESC)
			return -1;
		set_config(udc, ctrl->wValue);
	} else {
		memset(&priv->setup_req, 0, sizeof(udc_req_t));
		priv->setup_req.buf = &udc->config;
		priv->setup_req.length = 1;
		udc->ep[0].ops->queue(&udc->ep[0], &priv->setup_req);
	}
	return 0;
}

static inline int process_req_iface(udc_t *udc, struct usb_ctrlrequest *ctrl)
{
	udc_gadget_priv_t *priv = gadget_to_priv(udc->gadget);
	uint8_t interface = ctrl->wIndex & 0xff;
	uint8_t alternate = ctrl->wValue & 0xff;
	uint16_t reply;

	if (ctrl->bRequest == USB_REQ_SET_INTERFACE) {
		if (interface || alternate)
			return -1;
	} else {
		memset(&priv->setup_req, 0, sizeof(udc_req_t));
		INIT_LIST_HEAD(&priv->setup_req.queue);
		reply = 0;
		priv->setup_req.buf = &reply;
		priv->setup_req.length = 1;
		udc->ep[0].ops->queue(&udc->ep[0], &priv->setup_req);
	}
	return 0;
}

static inline void set_config(udc_t *udc, int config)
{
	struct usb_endpoint_descriptor *desc1, *desc2;

	if (udc->speed == USB_SPEED_HIGH) {
		desc1 = &sec_dths_config.ep1;
		desc2 = &sec_dths_config.ep2;
	} else {
		desc1 = &sec_dtfs_config.ep1;
		desc2 = &sec_dtfs_config.ep2;
	}

	udc->ep[1].ops->disable(&udc->ep[1]);
	udc->ep[2].ops->disable(&udc->ep[2]);
	if (config) {
		udc->ep[1].ops->enable(&udc->ep[1], desc1);
		udc->ep[1].ops->enable(&udc->ep[2], desc2);
		configured(udc);
	}

	udc->config = config;
}

static void configured(udc_t *udc)
{
	udc_gadget_priv_t *priv = gadget_to_priv(udc->gadget);

	if (list_empty(&udc->ep[2].queue)) {
		priv->command_req.length = CMDBUF_SIZE - 2;
		priv->command_req.complete = command_request;
		INIT_LIST_HEAD(&priv->command_req.queue);

		udc->ep[2].ops->queue(&udc->ep[2], &priv->command_req);
	}
}

#define MANUFACTURER_STRING u"Jeff Kent <jeff@jkent.net>"
#define PRODUCT_STRING u"Nanoboot bootloader"

/* High speed descriptors */
__attribute__((aligned(2)))
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

__attribute__((aligned(2)))
static const struct usb_qualifier_descriptor sec_dths_qual = {
	.bLength            = USB_DT_DEVICE_QUALIFIER_SIZE,
	.bDescriptorType    = USB_DT_DEVICE_QUALIFIER,
	.bcdUSB             = 0x0200,
	.bMaxPacketSize0    = 8,
	.bNumConfigurations = 1,
};

__attribute__((aligned(2)))
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
	.ep1 = {
		.bLength             = USB_DT_ENDPOINT_SIZE,
		.bDescriptorType     = USB_DT_ENDPOINT,
		.bEndpointAddress    = 1 | USB_DIR_IN,
		.bmAttributes        = USB_ENDPOINT_XFER_BULK,
		.wMaxPacketSize      = 512,
	},
	.ep2 = {
		.bLength             = USB_DT_ENDPOINT_SIZE,
		.bDescriptorType     = USB_DT_ENDPOINT,
		.bEndpointAddress    = 2 | USB_DIR_OUT,
		.bmAttributes        = USB_ENDPOINT_XFER_BULK,
		.wMaxPacketSize      = 512,
	},
};

/* Full speed descriptors */
__attribute__((aligned(2)))
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

__attribute__((aligned(2)))
static const struct usb_qualifier_descriptor sec_dtfs_qual = {
	.bLength            = USB_DT_DEVICE_QUALIFIER_SIZE,
	.bDescriptorType    = USB_DT_DEVICE_QUALIFIER,
	.bcdUSB             = 0x0200,
	.bMaxPacketSize0    = 64,
	.bNumConfigurations = 1,
};

__attribute__((aligned(2)))
struct usb_device_config_descriptor sec_dtfs_config = {
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
	.ep1 = {
		.bLength             = USB_DT_ENDPOINT_SIZE,
		.bDescriptorType     = USB_DT_ENDPOINT,
		.bEndpointAddress    = 1 | USB_DIR_IN,
		.bmAttributes        = USB_ENDPOINT_XFER_BULK,
		.wMaxPacketSize      = 64,
	},
	.ep2 = {
		.bLength             = USB_DT_ENDPOINT_SIZE,
		.bDescriptorType     = USB_DT_ENDPOINT,
		.bEndpointAddress    = 2 | USB_DIR_OUT,
		.bmAttributes        = USB_ENDPOINT_XFER_BULK,
		.wMaxPacketSize      = 64,
	},
};

/* String descriptors */
__attribute__((aligned(2)))
static const struct usb_string_descriptor str0_descriptor = {
	.bLength         = 2 + 1*2,
	.bDescriptorType = USB_DT_STRING,
	.wData           = { 0x0409 },
};

__attribute__((aligned(2)))
static const struct usb_string_descriptor str1_descriptor = {
	.bLength         = 2 + (sizeof(MANUFACTURER_STRING) - 2),
	.bDescriptorType = USB_DT_STRING,
	.wData           = { MANUFACTURER_STRING },
};

__attribute__((aligned(2)))
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
