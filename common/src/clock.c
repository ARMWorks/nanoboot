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

#define APLL_LOCK 			( *((volatile unsigned long *)0xE0100000) )
#define MPLL_LOCK 			( *((volatile unsigned long *)0xE0100008) )

#define APLL_CON0 			( *((volatile unsigned long *)0xE0100100) )
#define APLL_CON1 			( *((volatile unsigned long *)0xE0100104) )
#define MPLL_CON 			( *((volatile unsigned long *)0xE0100108) )

#define CLK_SRC0 			( *((volatile unsigned long *)0xE0100200) )
#define CLK_SRC1 			( *((volatile unsigned long *)0xE0100204) )
#define CLK_SRC2 			( *((volatile unsigned long *)0xE0100208) )
#define CLK_SRC3 			( *((volatile unsigned long *)0xE010020c) )
#define CLK_SRC4 			( *((volatile unsigned long *)0xE0100210) )
#define CLK_SRC5 			( *((volatile unsigned long *)0xE0100214) )
#define CLK_SRC6 			( *((volatile unsigned long *)0xE0100218) )
#define CLK_SRC_MASK0 		( *((volatile unsigned long *)0xE0100280) )
#define CLK_SRC_MASK1 		( *((volatile unsigned long *)0xE0100284) )

#define CLK_DIV0 			( *((volatile unsigned long *)0xE0100300) )
#define CLK_DIV1 			( *((volatile unsigned long *)0xE0100304) )
#define CLK_DIV2 			( *((volatile unsigned long *)0xE0100308) )
#define CLK_DIV3 			( *((volatile unsigned long *)0xE010030c) )
#define CLK_DIV4 			( *((volatile unsigned long *)0xE0100310) )
#define CLK_DIV5 			( *((volatile unsigned long *)0xE0100314) )
#define CLK_DIV6 			( *((volatile unsigned long *)0xE0100318) )
#define CLK_DIV7 			( *((volatile unsigned long *)0xE010031c) )

#define CLK_DIV0_MASK		0x7fffffff

#define APLL_MDIV       	125
#define APLL_PDIV       	3
#define APLL_SDIV      	 	1

#define MPLL_MDIV			667
#define MPLL_PDIV			12
#define MPLL_SDIV			1

#define set_pll(mdiv, pdiv, sdiv)	(1<<31 | mdiv<<16 | pdiv<<8 | sdiv)
#define APLL_VAL		    set_pll(APLL_MDIV, APLL_PDIV, APLL_SDIV)
#define MPLL_VAL		    set_pll(MPLL_MDIV, MPLL_PDIV, MPLL_SDIV)


void clock_init(void)
{
	CLK_SRC0 = 0x0;

	APLL_LOCK = 0x0000FFFF;
	MPLL_LOCK = 0x0000FFFF;

	CLK_DIV0 = 0x14131440;

	// FOUT = MDIV * FIN / (PDIV * 2^(SDIV - 1)) = 1000 MHz
	APLL_CON0 = APLL_VAL;
	// FOUT = MDIV * FIN / (PDIV * 2^SDIV) = 667 MHz
	MPLL_CON  = MPLL_VAL;

	CLK_SRC0 = 0x10001111;
}
