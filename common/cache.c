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

#include "asm/barriers.h"
#include "system.h"

#include <stdint.h>


void icache_invalidate(void)
{
	uint32_t sbz = 0;
	asm("mcr p15, 0, %0, c7, c5, 0" :: "r" (sbz));
}

__attribute__((target("arm")))
void icache_enable(void)
{
	uint32_t reg;
	reg = get_sctlr();
	reg |= SCTLR_I;
	set_sctlr(reg);
}

__attribute__((target("arm")))
void icache_disable(void)
{
	uint32_t reg;
	reg = get_sctlr();
	reg &= ~SCTLR_I;
	set_sctlr(reg);
}
