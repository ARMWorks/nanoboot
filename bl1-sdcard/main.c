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

#include "cache.h"
#include "config.h"
#include "irom.h"

#include "s5pv210.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>


void main(void)
{
    irom_init();

    puts("\nBL1");

    int channel = -1;

    switch (globalSDMMCBase) {
    case ELFIN_HSMMC_0_BASE:
        channel = 0;
        break;

    case ELFIN_HSMMC_1_BASE:
        channel = 1;
        break;

    case ELFIN_HSMMC_2_BASE:
        channel = 2;
        break;

    case ELFIN_HSMMC_3_BASE:
        channel = 3;
        break;

    default:
        return;
    }

    CopySDMMCtoMem(channel, 33, 32, (uint32_t *)NANOBOOT_BASE, false);

    icache_invalidate();
    ((void (*)(void))NANOBOOT_BASE + 0x10)();
}
