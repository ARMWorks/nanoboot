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

#pragma once

#include "linux/list.h"
#include "linux/usb/ch9.h"

#define NUM_ENDPOINTS 3

typedef struct udc_t udc_t;
typedef struct udc_ep_t udc_ep_t;
typedef struct udc_ep_ops_t udc_ep_ops_t;
typedef struct udc_req_t udc_req_t;
typedef struct udc_gadget_t udc_gadget_t;
typedef struct udc_gadget_priv_t udc_gadget_priv_t;
typedef struct udc_driver_t udc_driver_t;

enum ep0_state {
    WAIT_FOR_SETUP = 0,
    DATA_STATE_TX,
    DATA_STATE_RX
};

struct udc_ep_t {
    void *fifo;
    uint8_t address;
    uint8_t stopped;
    uint16_t maxpacket;
    udc_t *udc;
    udc_ep_ops_t *ops;
    struct list_head queue;
};

struct udc_t {
    udc_driver_t *driver;
    udc_gadget_t *gadget;
    uint8_t speed;
    uint8_t config;
    uint8_t state;
    enum ep0_state ep0_state;
    udc_ep_t ep[NUM_ENDPOINTS];
};

struct udc_ep_ops_t {
    int (*enable)(udc_ep_t *ep, const struct usb_endpoint_descriptor *desc);
    int (*disable)(udc_ep_t *ep);
    udc_req_t *(*alloc_req)(udc_ep_t *ep);
    void (*free_req)(udc_ep_t *ep, udc_req_t *req);
    int (*queue)(udc_ep_t *ep, udc_req_t *req);
    int (*dequeue)(udc_ep_t *ep, udc_req_t *req);
    int (*set_halt)(udc_ep_t *ep, bool halt);
    void (*fifo_flush)(udc_ep_t *ep);
};

struct udc_req_t {
    void *buf;
    size_t length;
    size_t actual;
    bool zero;
    void (*complete)(udc_ep_t *ep, udc_req_t *req);
    int status;
    struct list_head queue;
};

struct udc_gadget_t {
    void (*init)(udc_t *udc);
    void (*free)(udc_t *udc);
    int (*setup)(udc_t *udc, struct usb_ctrlrequest *ctrl);
};

struct udc_driver_t {
    void (*attach)(udc_t *udc);
    void (*detach)(udc_t *udc);
};

void udc_init(udc_driver_t *driver);
void udc_free(void);
void udc_gadget_attach(udc_gadget_t *gadget);
void udc_gadget_detach(void);
