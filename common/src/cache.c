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

#include "barriers.h"
#include "system.h"

#include <stdint.h>


void icache_invalidate(void)
{
	uint32_t sbz = 0;
	asm("mcr p15, 0, %0, c7, c5, 0" :: "r" (sbz));
}

void icache_enable(void)
{
	uint32_t reg;
	reg = get_sctlr();
	reg |= SCTLR_I;
	set_sctlr(reg);
}

void icache_disable(void)
{
	uint32_t reg;
	reg = get_sctlr();
	reg &= ~SCTLR_I;
	set_sctlr(reg);
}

void dcache_invalidate(void)
{
	asm("1:	mrc p15, 0, r15, c7, c14, 3\n\t"
		"bne 1b" ::: "cc", "r15");
}

void dcache_enable(void)
{
	dcache_invalidate();

	uint32_t reg;
	reg = get_sctlr();
	reg |= SCTLR_C;
	set_sctlr(reg);
}

void dcache_disable(void)
{
	dcache_invalidate();

	uint32_t reg;
	reg = get_sctlr();
	reg &= ~SCTLR_C;
	set_sctlr(reg);
}

void invalidate_l1(void)
{
	asm("mov r0, #0\n"
		"mcr p15, 2, r0, c0, c0, 0\n"
		"mrc p15, 1, r0, c0, c0, 0\n"

		"movw r1, 0x7fff\n"
		"and r2, r1, r0, lsr #13\n"

		"movw r1, #0x3ff\n"

		"and r3, r1, r0, lsr #3\n"
		"and r2, r2, #1\n"

		"and r0, r0, #0x7\n"
		"and r0, r0, #4\n"

		"clz r1, r3\n"
		"add r4, r3, #1\n"
		"1: sub r2, r2, #1\n"
		"mov r3, r4\n"
		"2: subs r3, r3, #1\n"
		"mov r5, r3, lsl r1\n"
		"mov r6, r2, lsl r0\n"
		"orr r5, r5, r6\n"
		"mcr p15, 0, r5, c7, c6, 2\n"
		"bgt 2b\n"
		"cmp r2, #0\n"
		"bgt 1b\n"
		"dsb st\n"
		"isb\n" ::: "r0", "r1", "r2", "r3", "r4", "r5", "r6");
}

void tlb_invalidate(void)
{
	/* Invalidate entire unified TLB */
	asm volatile ("mcr p15, 0, %0, c8, c7, 0" : : "r" (0));
	/* Invalidate entire data TLB */
	asm volatile ("mcr p15, 0, %0, c8, c6, 0" : : "r" (0));
	/* Invalidate entire instruction TLB */
	asm volatile ("mcr p15, 0, %0, c8, c5, 0" : : "r" (0));
	/* Full system DSB - make sure that the invalidation is complete */
	dsb();
	/* Full system ISB - make sure the instruction stream sees it */
	isb();
}