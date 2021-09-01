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

.arm
.globl entry
entry:
	bl blink

	.space 512

blink:
	/* set LED GPIOs as output */
	ldr r2, =0xE0200280
	ldr r1, =0x00001111
	str r1, [r2]
	ldr r2, =0xE0200284
	ldr r1, =0xA
1:
	/* flash LEDs */
	str r1, [r2]
	mov r0, #0x1000000
2:
	cmp r0, #0
	sub r0, r0, #1
	bne 2b
	eor r1, r1, #0xF
	b 1b

.ltorg
