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

#include <errno.h>
#include <stddef.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "uart.h"


void *_sbrk(int incr)
{
    extern char _end; /* Defined by the linker */
    static char *heap_end = 0;
    char *prev_heap_end;

    /* disable interrupts */

    register char *stack __asm("sp");

    if (heap_end == 0) {
        heap_end = &_end;
    }

    if (stack < heap_end + incr) {
        /* Heap and stack collision */
        /* enable interrupts */
        errno = ENOMEM;
        return (void *) -1;
    }

    prev_heap_end = heap_end;
    heap_end += incr;

    /* enable interrupts */

    return (void *) prev_heap_end;
}

int _close(int fd)
{
    return -1;
}

int _fstat(int file, struct stat *st)
{
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int file)
{
    return 1;
}

int _lseek(int file, int offset, int whence)
{
    return 0;
}

void _exit(int status)
{
    __asm("BKPT #0");
    while(1);
}

void _kill(int pid, int sig)
{
    return;
}

int _getpid(void)
{
    return -1;
}

int _write(int file, char *buf, int len)
{
    int written = 0;

    if ((file != 1) && (file != 2) && (file != 3)) {
        return -1;
    }

    for (; len != 0; --len) {
        uart0_putc(*buf++);
        ++written;
    }

    return written;
}

int _read(int file, char *buf, int len)
{
    int read = 0;

    if (file != 0) {
        return -1;
    }

    for (; len > 0; --len) {
        *buf++ = uart0_getc();
        read++;
    }

    return read;
}
