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

#include "asm/io.h"
#include "irq.h"
#include "s5pv210.h"
#include "dnw.h"
#include "system.h"
#include "timer.h"
#include "uart.h"
#include "udc.h"

#include <stdlib.h>


static void isr_key(uint32_t irq, void *pv)
{
    uart0_putc('!');
    writel(readl(ELFIN_GPIO_BASE + EXT_INT_2_PEND) | (1 << 0),
            ELFIN_GPIO_BASE + EXT_INT_2_PEND);
}

void main(void)
{
    uart0_puts("BL2");

    writel(readl(ELFIN_GPIO_BASE + GPH2CON_OFFSET) | 0xF,
            ELFIN_GPIO_BASE + GPH2CON_OFFSET);
    writel(readl(ELFIN_GPIO_BASE + EXT_INT_2_CON) | (1 << 1),
            ELFIN_GPIO_BASE + EXT_INT_2_CON);
    writel(readl(ELFIN_GPIO_BASE + EXT_INT_2_MASK) & ~(1 << 0),
            ELFIN_GPIO_BASE + EXT_INT_2_MASK);

    irq_set_handler(IRQ_EINT16_31, isr_key, NULL);
    irq_enable(IRQ_EINT16_31);

    //udc_probe();
    //udc_register_gadget(&dnw_gadget);

    while (1) {
        uart0_putc('.');
        udelay(1000000);
    }
}
