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

#include "cache.h"
#include "config.h"
#include "irom.h"
#include "uart.h"

#include "s5pv210.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>


typedef struct bl2_header_t bl2_header_t;
struct bl2_header_t {
    uint32_t size;
    uint32_t reserved1;
    uint32_t checksum;
    uint32_t reserved2;
} __attribute__((__packed__));


void main(void)
{
    uint8_t *p = (uint8_t *) NANOBOOT_BASE;

    uart0_set_baudrate(460800);

    puts("BL1 UART Loader READY");
    fread(p, 16, 1, stdin);

    bl2_header_t *header = (void *) p;

    p += 16;
    fread(p, header->size - 16, 1, stdin);

    uint32_t checksum = 0;
    for (uint32_t i = 0; i < header->size - 16; i++) {
        checksum += *p++;
    }

    if (checksum == header->checksum) {
        icache_invalidate();
        ((void (*)(void))NANOBOOT_BASE + 0x10)();
    } else {
        puts("bad checksum");
    }
}
