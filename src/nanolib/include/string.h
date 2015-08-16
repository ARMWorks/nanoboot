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

#ifndef	_STRING_H
#define	_STRING_H

#include <stddef.h>

void *memcpy(void *dest, const void *src, size_t n);
void *memset (void *s, int c, size_t n);
char *strcpy(char *dest, const char *src);
size_t strlen(const char *s);
#define bzero(s, n) memset(s, 0, n)

#endif /* _STRING_H */
