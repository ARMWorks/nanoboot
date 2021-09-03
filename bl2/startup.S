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
	ldr r0, =NANOBOOT_STACK_TOP
	/* Setup FIQ stack */
	msr cpsr_c, #0xD1
	mov sp, r0
	sub r0, r0, #NANOBOOT_FIQ_STACK_SIZE
	/* Setup IRQ stack */
	msr cpsr_c, #0xD2
	mov sp, r0
	mov fp, #0
	sub r0, r0, #NANOBOOT_IRQ_STACK_SIZE
	/* Setup ABT stack */
	msr cpsr_c, #0xD7
	mov sp, r0
	sub r0, r0, #NANOBOOT_ABT_STACK_SIZE
	/* Setup UND stack */
	msr cpsr_c, #0xDB
	mov sp, r0
	sub r0, r0, #NANOBOOT_UND_STACK_SIZE
	/* Setup SVC stack */
	msr cpsr_c, #0xD3
	mov sp, r0
	sub r0, r0, #NANOBOOT_SVC_STACK_SIZE
	/* Setup SYS stack with interrupts enabled */
	msr cpsr_c, #0xDF
	mov sp, r0
    mov fp, #0 /* no previous frame */

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

	bl icache_disable
	bl icache_invalidate

	bl mmu_init
	bl mmu_load
	bl tlb_invalidate
	bl mmu_enable
	bl icache_enable

	bl irq_init
	bl uart0_init
	bl timer_init

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
	mov r0, #0x400000
2:
	cmp r0, #0
	sub r0, r0, #1
	bne 2b
	eor r1, r1, #0xf
	b 1b

.ltorg
