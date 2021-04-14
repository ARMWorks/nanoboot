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

#include "udc.h"

static udc_t udc = {};

void udc_init(udc_driver_t *driver)
{
    udc.driver = driver;
}

void udc_free(void)
{
    if (udc.gadget) {
        udc_gadget_detach();
    }
}

void udc_gadget_attach(udc_gadget_t *gadget)
{
    if (udc.gadget) {
        udc_gadget_detach();
    }
    gadget->init(&udc);
    udc.driver->attach(&udc);
}

void udc_gadget_detach(void)
{
    udc.driver->detach(&udc);
    udc.gadget->free(&udc);
}
