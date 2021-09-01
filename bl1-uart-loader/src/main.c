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
#include <unistd.h>


#define CHUNK_SIZE (1024)

#define MIN(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
       _a < _b ? _a : _b; })

typedef struct bl2_header_t bl2_header_t;
struct bl2_header_t {
    uint32_t size;
    uint32_t reserved1;
    uint32_t checksum;
    uint32_t reserved2;
} __attribute__((__packed__));

void main(void)
{
    uart0_set_baudrate(460800);
    puts("BL1 UART Loader READY");

    bl2_header_t *header = (bl2_header_t *) NANOBOOT_BASE;
    read(STDIN_FILENO, header, 16);

    uint8_t *p = ((uint8_t *) header) + 16;
    read(STDIN_FILENO, p, header->size - 16);

    uint32_t checksum = 0;
    for (uint32_t i = 0; i < header->size - 16; i++) {
        checksum += *p++;
    }

    if (checksum == header->checksum) {
        icache_invalidate();
        ((void (*)(void))NANOBOOT_BASE + 16)();
    } else {
        puts("bad checksum");
    }
}
