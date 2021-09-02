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
#include "s5pv210.h"
#include "timer.h"

#include <stdio.h>


static uint32_t reset_value;
static uint32_t lastinc;

void timer_init(void)
{
    uint32_t timer_base = ELFIN_TIMER_BASE;

    writel((readl(timer_base + TCFG0_OFFSET) & ~0x0000FF00) | 0x00004100,
            timer_base + TCFG0_OFFSET);
    writel(readl(timer_base + TCFG1_OFFSET) & ~0x000F0000,
            timer_base + TCFG1_OFFSET);
    writel(0xFFFFFFFF, timer_base + TCNTB4_OFFSET);
    writel((readl(timer_base + TCON_OFFSET) & ~0x00F000000) | TCON_4_UPDATE,
            timer_base + TCON_OFFSET);
    writel(readl(timer_base + TCON_OFFSET) & ~0x00F00000,
            timer_base + TCON_OFFSET);
    writel(readl(timer_base + TCON_OFFSET) | TCON_4_AUTO | TCON_4_ONOFF,
            timer_base + TCON_OFFSET);

    reset_value = 0;
    lastinc = timer_get_us_down();
}

uint32_t timer_get(uint32_t base)
{
    uint32_t now = timer_get_us_down();

    /* Increment the time by the ammount elapsed since the last read.
     * The timer may have wrapped around, but it makes no difference to
     * our arithmetic here. */
    reset_value += lastinc - now;
    lastinc = now;

    /* Divide by 1000 to convert from us to ms */
    uint32_t time_ms = reset_value;
    time_ms /= 1000;
    return time_ms - base;
}

uint32_t timer_get_us_down(void)
{
    uint32_t timer_base = ELFIN_TIMER_BASE;

    return readl(timer_base + TCNTO4_OFFSET);
}

uint32_t timer_get_us(void)
{
    static uint32_t base_time_us = 0;
    uint32_t now_downward_us = timer_get_us_down();

    if (!base_time_us) {
        base_time_us = now_downward_us;
    }

    return base_time_us - now_downward_us;
}

void udelay(uint32_t us)
{
    uint32_t start = timer_get_us_down();
    while ((int32_t)(start - timer_get_us_down()) < (int32_t)us);
}
