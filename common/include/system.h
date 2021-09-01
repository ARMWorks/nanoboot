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

#pragma once

#include "asm/barriers.h"

#include <stdint.h>


#define CPSR_F (1 << 6)
#define CPSR_I (1 << 7)
#define CPSR_MODE_MASK (0x1F)
#define CPSR_USR (0x10)
#define CPSR_FIQ (0x11)
#define CPSR_IRQ (0x12)
#define CPSR_SVC (0x13)
#define CPSR_MON (0x16)
#define CPSR_ABT (0x17)
#define CPSR_HYP (0x1A)
#define CPSR_UND (0x1B)
#define CPSR_SYS (0x1F)

static inline uint32_t get_cpsr(void)
{
	uint32_t reg;
	asm volatile("mrs %0, cpsr" : "=r" (reg));
	return reg;
}

static inline void set_cpsr(uint32_t reg)
{
	asm volatile("msr cpsr, %0" :: "r" (reg));
}

#define SCTLR_M (1 << 0)
#define SCTLR_C (1 << 2)
#define SCTLR_Z (1 << 11)
#define SCTLR_I (1 << 12)
#define SCTLR_V (1 << 13)

static inline uint32_t get_sctlr(void)
{
	uint32_t reg;
    asm volatile("mrc p15, 0, %0, c1, c0, 0" : "=r" (reg) :: "cc");
	return reg;
}

static inline void set_sctlr(uint32_t reg)
{
    asm volatile("mcr p15, 0, %0, c1, c0, 0" :: "r" (reg) : "cc");
	isb();
}
