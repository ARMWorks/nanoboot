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

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

static int itoa(int value, char *s, unsigned int radix, bool uppercase, bool is_signed)
{
    unsigned int n;
    char *p;
    char digit;
    int digits = 0;
    int negative = 0;

    if (value == 0) {
        s[0] = '0';
        s[1] = 0;
        return 1;
    }

    if (is_signed && radix == 10 && value < 0) {
        value = -value;
        negative = 1;
        digits += 1;
    }

    n = value;
    while (n) {
        n /= radix;
        digits += 1;
    }

    p = s + digits;
    *p-- = 0;

    n = value;
    while (n) {
        digit = (n % radix);
        n /= radix;
        digit += (digit <= 9) ? '0' : (uppercase ? 'A' : 'a') - 10;
        *p-- = digit;
    }

    if (negative) {
        *p = '-';
    }

    return digits;
}

int vprintf(const char *fmt, va_list va)
{
    char buf[12];
    char c;
    char zero_pad;
    unsigned int len;

    while ((c = *fmt++)) {
        if (c != '%') {
            fputc(c, stdout);
            if (c == '\n')
                fputc('\r', stdout);
            continue;
        }

        c = *fmt++;

        zero_pad = 0;
        if (c == '0') {
            c = *fmt++;
            if (c == '\0')
                return 0;
            while (c >= '0' && c <= '9') {
                zero_pad *= 10;
                zero_pad += c - '0';
                c = *fmt++;
            }
        }

        switch (c) {
            case 0:
                return 0;

            case 'u':
            case 'd':
                len = itoa(va_arg(va, int), buf, 10, 0, (c=='d'));
                while (zero_pad-- > len)
                    fputc('0', stdout);
                fputs(buf, stdout);
                break;

            case 'x':
            case 'X':
                len = itoa(va_arg(va, int), buf, 16, (c=='X'), 0);
                while (zero_pad-- > len)
                    fputc('0', stdout);
                fputs(buf, stdout);
                break;

            case 'c':
                fputc((char)(va_arg(va, int)), stdout);
                break;

            case 's':
                fputs(va_arg(va, char *), stdout);
                break;

            default:
                fputc(c, stdout);
                if (c == '\n')
                    fputc('\r', stdout);
                break;
        }
    }
    return 0;
}
