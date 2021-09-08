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

#include "config.h"

.arm
.globl entry
entry:
    /* Switch to SVC mode */
    mrs r0, cpsr
    bic r0, r0, #0x1f
    orr r0, r0, #0xd3
    msr cpsr, r0

	ldr	sp, =0xD0037D80 /* SRAM SVC stack */
    mov fp, #0 /* no previous frame, so fp=0 */

clear_bss:
	ldr r0, =_sbss
	ldr r1, =_ebss
	mov r2, #0
	cmp r0, r1
	bge 2f
1:  str r2, [r0]
    add r0, r0, #4
	cmp r0, r1
	ble 1b
2:

	bl clock_init
	bl uart0_init
	bl sdram_init

	ldr r0, =main
	blx r0

.globl error_halt
error_halt:
	/* set LED GPIOs as output */
	ldr r2, =0xE0200280
	ldr r1, =0x00001111
	str r1, [r2]
	ldr r2, =0xE0200284
	ldr r1, =0xf
1:
	/* flash LEDs */
	str r1, [r2]
	mov r0, #0x1000000
2:
	cmp r0, #0
	sub r0, r0, #1
	bne 2b
	eor r1, r1, #0xf
	b 1b

.ltorg
