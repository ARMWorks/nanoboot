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

#include "config.h"
#include "dwc2.h"
#include "irom.h"
#include "irq.h"
#include "sec_gadget.h"
#include "timer.h"
#include "uart.h"

#include "s5pv210.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>


static void hd(void *addr, int dwords)
{
    int i;
    uint32_t *p = (uint32_t*)addr;

    for (i = 0; i < dwords; i++) {
        if ((i % 8) == 0) {
            if (i != 0) {
                printf("\n");
            }
            printf("%08lx", (uint32_t)(p + i));
        }

        printf(" %08lx", *(uint32_t *)(p + i));
    }

    printf("\n");
}

void main(void)
{
    udc_init(&dwc2_driver);
    udc_gadget_attach(&sec_gadget);

    hd((void *)0xEC100000, 16);
    putchar('\n');
    hd((void *)0xEC000000, 1024);

    enable_irqs();

    while (1) {
        putchar('.'); fflush(stdout);
        udelay(1000000);
    }
}
