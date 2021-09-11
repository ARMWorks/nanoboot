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

#include <stdbool.h>

#include <asm/io.h>
#include <dnw.h>
#include <udc.h>
#include <s5pv210.h>
#include <timer.h>

void larson(void)
{
    int i;
    void *gpio = (void *) ELFIN_GPIO_BASE;
    writel((readl(gpio + GPJ2CON_OFFSET) & ~0x0000FFFF) | 0x00001111,
            gpio + GPJ2CON_OFFSET);

    while (true) {
        for (i = 0; i <= 3; i++) {
            writel((readl(gpio + GPJ2DAT_OFFSET) & ~0x0000000F) |
                    (~(1 << i) & 0x0000000F), gpio + GPJ2DAT_OFFSET);
            mdelay(100);

        }
        for (i = 3; i >= 0; i--) {
            writel((readl(gpio + GPJ2DAT_OFFSET) & ~0x0000000F) |
                    (~(1 << i) & 0x0000000F), gpio + GPJ2DAT_OFFSET);
            mdelay(100);
        }
    }
}

void main(void)
{
    udc_probe();
    udc_register_gadget(&dnw_gadget);
    larson();
}
