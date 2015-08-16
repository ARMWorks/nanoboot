/*
 * Copyright (C) 2013 Jeff Kent <jeff@jkent.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _STDIO_H
#define _STDIO_H

#include <stdarg.h>
#include <stdio.h>
#include "config.h"

#include "serial.h"

#define FILE void
#define stdin (void *)0
#define stdout (void *)1
#define stderr (void *)2

int printf (const char *, ...);
int vprintf(const char *fmt, va_list va);

#define putchar(c) serial_putc((char)c)
#define puts(s) serial_puts(s)
#define fputc(c, stream) serial_putc((char)c)
#define fputs(s, stream) serial_puts(s)
#define fflush(stream)

#endif /* _STDIO_H */
