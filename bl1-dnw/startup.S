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

	.arm
	.section .text.entry
	.globl _entry
_entry:
	sub r4, pc, #24
	b _startup

	.section .text.ivt
	.globl _ivt
_ivt:
	b _entry
	b exc_undef
	b exc_swi
	b exc_pabort
	b exc_dabort
	b error_halt
	b exc_irq
	b exc_fiq

	.ltorg

	.section .text
_startup:
	bl clock_init
	bl dram_init

	/* Relocate */							@ r4: current start
											@ r5: current end
	ldr r6, =__image_copy_start				@ r6: desired start
	ldr r7, =__image_copy_end				@ r7: desired end
	subs r8, r7, r6							@ r8: size
	add r5, r8, r4

	beq reloc_done
	bgt 2f

1:	ldmia r4!, {r0-r3}
	stmia r6!, {r0-r3}
	cmp r4, r5
	blt 1b
	b 3f

2:	ldmdb r5!, {r0-r3}
	stmdb r7!, {r0-r3}
	cmp r4, r5
	blt 2b

3:	bl icache_invalidate
	bl icache_enable
	ldr pc, =reloc_done

reloc_done:
	/* Stack in SDRAM */
	ldr sp, =__image_copy_start
	add sp, sp, #0x00100000

clear_bss:
	ldr r0, =_sbss
	ldr r1, =_ebss
	cmp r0, r1
	bge 2f
	mov r2, #0
1:	stmia r0!, {r2}
	cmp r0, r1
	ble 1b
2:

	bl uart0_init
	bl irq_init
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
	mov r0, #0x1000000
2:
	cmp r0, #0
	sub r0, r0, #1
	bne 2b
	eor r1, r1, #0xf
	b 1b

	.ltorg
