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
#include "uart.h"

#include "s5pv210.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#define CHUNK_SIZE (1024)

#define MIN(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
       _a < _b ? _a : _b; })

static void readline(char *buf, size_t maxlen)
{
    size_t len = 0;

    while (len < maxlen - 1) {
        read(STDIN_FILENO, buf + len, 1);
        if (*(buf + len) == '\n') {
            break;
        }
        len++;
    }
    *(buf + len) = '\0';
}

void main(void)
{
    char buf[64], *p;
    uint32_t load_addr = 0x20000000;
    uint32_t exec_addr;
    size_t length = 0;

    uart0_set_baudrate(460800);
    uart0_puts("BL1 UART Loader READY");

    while (true) {
        readline(buf, sizeof(buf));
        if (strncmp(buf, "load ", 5) == 0) {
            length = strtoul(buf + 5, &p, 0);
            load_addr = 0x20000000;
            if (*p == ' ' && *(p + 1) == '@') {
                load_addr = strtoul(p + 2, &p, 0);
            }
            read(STDIN_FILENO, (void *)load_addr, length);
        } else if (strncmp(buf, "exec", 4) == 0) {
            exec_addr = load_addr;
            if (*(buf + 4) == ' ') {
                exec_addr = strtoul(buf + 5, &p, 0);
            }
            icache_invalidate();
            ((void (*)(void))exec_addr)();
        } else if (strncmp(buf, "check ", 6) == 0) {
            uint32_t checksum = 0;
            p = (char *) load_addr;
            for (size_t i = 0; i < length; i++) {
                checksum += *p++;
            }
            if (checksum != strtoul(buf + 6, &p, 0)) {
                uart0_puts("BL1 CHECKSUM FAIL");
            } else {
                uart0_puts("BL1 CHECKSUM OK");
            }
        }
    }
}
