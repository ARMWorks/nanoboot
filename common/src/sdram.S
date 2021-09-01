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

#include "s5pv210.h"

/* BL=4, 1Chip, DDR2 Type, dynamic self refresh, force precharge,
 * dynamic power down off */
#define DMC0_MEMCONTROL			0x00202400
/* 256MB config, 8 banks, mapping[15:12] 0:linear, 1:linterleaved, 2:Mixed */
#define DMC0_MEMCONFIG_0		0x20F00323
#define DMC0_MEMCONFIG_1		0x00F00323

/* 7.8us * 133MHz = 1038(0x40E), 100MHz = 780(0x30C), 20MHz = 156(0x9C),
 * 10MHz = 78(0x4E) */
#define DMC0_TIMINGA_REF        0x00000618
/* @200MHz */
#define DMC0_TIMING_ROW         0x2B34438A
/* CL=3 */
#define DMC0_TIMING_DATA        0x24240000
#define DMC0_TIMING_PWR         0x0BDC0343

.globl sdram_init
sdram_init:
	/* 2x drive strength */
	ldr	r0, =ELFIN_GPIO_BASE
	ldr	r1, =0x0000AAAA
	str	r1, [r0, #MP1_0DRV_SR_OFFSET]
	ldr	r1, =0x0000AAAA
	str	r1, [r0, #MP1_1DRV_SR_OFFSET]
	ldr	r1, =0x0000AAAA
	str	r1, [r0, #MP1_2DRV_SR_OFFSET]
	ldr	r1, =0x0000AAAA
	str	r1, [r0, #MP1_3DRV_SR_OFFSET]
	ldr	r1, =0x0000AAAA
	str	r1, [r0, #MP1_4DRV_SR_OFFSET]
	ldr	r1, =0x0000AAAA
	str	r1, [r0, #MP1_5DRV_SR_OFFSET]
	ldr	r1, =0x0000AAAA
	str	r1, [r0, #MP1_6DRV_SR_OFFSET]
	ldr	r1, =0x0000AAAA
	str	r1, [r0, #MP1_7DRV_SR_OFFSET]
	ldr	r1, =0x00002AAA
	str	r1, [r0, #MP1_8DRV_SR_OFFSET]

	/* PHY DLL */
	ldr	r0, =APB_DMC_0_BASE
	ldr	r1, =0x00101000
	str	r1, [r0, #DMC_PHYCONTROL0]
	/* DDR2 */
	ldr	r1, =0x00000086
	str	r1, [r0, #DMC_PHYCONTROL1]

	/* DLL on */
	ldr	r1, =0x00101002
	str	r1, [r0, #DMC_PHYCONTROL0]
	/* DLL start */
	ldr	r1, =0x00101003
	str	r1, [r0, #DMC_PHYCONTROL0]

1:	/* Wait for DLL lock */
	ldr	r1, [r0, #DMC_PHYSTATUS]
	and	r2, r1, #0x7
	cmp	r2, #0x7
	bne	1b

	/* Force DLL lock */
	and	r1, #0x3fc0
	mov	r2, r1, LSL #18
	orr	r2, r2, #0x100000
	orr	r2 ,r2, #0x1000
	orr	r1, r2, #0x3
	str	r1, [r0, #DMC_PHYCONTROL0]

	/* Auto refresh off */
	ldr	r1, =0x0FFF2010
	str	r1, [r0, #DMC_CONCONTROL]
	ldr	r1, =DMC0_MEMCONTROL
	str	r1, [r0, #DMC_MEMCONTROL]
	ldr	r1, =DMC0_MEMCONFIG_0
	str	r1, [r0, #DMC_MEMCONFIG0]
	ldr	r1, =DMC0_MEMCONFIG_1
	str	r1, [r0, #DMC_MEMCONFIG1]
	ldr	r1, =0xFF000000
	str	r1, [r0, #DMC_PRECHCONFIG]
	ldr	r1, =DMC0_TIMINGA_REF
	str	r1, [r0, #DMC_TIMINGAREF]
	ldr	r1, =DMC0_TIMING_ROW
	str	r1, [r0, #DMC_TIMINGROW]
	ldr	r1, =DMC0_TIMING_DATA
	str	r1, [r0, #DMC_TIMINGDATA]
	ldr	r1, =DMC0_TIMING_PWR
	str	r1, [r0, #DMC_TIMINGPOWER]

	/* chip0 deselect */
	ldr	r1, =0x07000000
	str	r1, [r0, #DMC_DIRECTCMD]
	/* chip0 PALL */
	ldr	r1, =0x01000000
	str	r1, [r0, #DMC_DIRECTCMD]
	/* chip0 EMRS2 */
	ldr	r1, =0x00020000
	str	r1, [r0, #DMC_DIRECTCMD]
	/* chip0 EMRS3 */
	ldr	r1, =0x00030000
	str	r1, [r0, #DMC_DIRECTCMD]
	/* chip0 EMRS1 (MEM DLL on, DQS# disable) */
	ldr	r1, =0x00010400
	str	r1, [r0, #DMC_DIRECTCMD]
	/* chip0 MRS (MEM DLL reset) CL=4, BL=4 */
	ldr	r1, =0x00000542
	str	r1, [r0, #DMC_DIRECTCMD]
	/* chip0 PALL */
	ldr	r1, =0x01000000
	str	r1, [r0, #DMC_DIRECTCMD]
	/* chip0 REFA */
	ldr	r1, =0x05000000
	str	r1, [r0, #DMC_DIRECTCMD]
	/* chip0 REFA */
	ldr	r1, =0x05000000
	str	r1, [r0, #DMC_DIRECTCMD]
	/* chip0 MRS (MEM DLL unreset) */
	ldr	r1, =0x00000442
	str	r1, [r0, #DMC_DIRECTCMD]
	/* chip0 EMRS1 (OCD default) */
	ldr	r1, =0x00010780
	str	r1, [r0, #DMC_DIRECTCMD]
	/* chip0 EMRS1 (OCD exit) */
	ldr	r1, =0x00010400
	str	r1, [r0, #DMC_DIRECTCMD]

	/* chip1 deselect */
	ldr	r1, =0x07100000
	str	r1, [r0, #DMC_DIRECTCMD]
	/* chip1 PALL */
	ldr	r1, =0x01100000
	str	r1, [r0, #DMC_DIRECTCMD]
	/* chip1 EMRS2 */
	ldr	r1, =0x00120000
	str	r1, [r0, #DMC_DIRECTCMD]
	/* chip1 EMRS3 */
	ldr	r1, =0x00130000
	str	r1, [r0, #DMC_DIRECTCMD]
	/* chip1 EMRS1 (MEM DLL on, DQS# disable) */
	ldr	r1, =0x00110400
	str	r1, [r0, #DMC_DIRECTCMD]
	/* chip1 MRS (MEM DLL reset) CL=4, BL=4 */
	ldr	r1, =0x00100542
	str	r1, [r0, #DMC_DIRECTCMD]
	/* chip1 PALL */
	ldr	r1, =0x01100000
	str	r1, [r0, #DMC_DIRECTCMD]
	/* chip1 REFA */
	ldr	r1, =0x05100000
	str	r1, [r0, #DMC_DIRECTCMD]
	/* chip1 REFA */
	ldr	r1, =0x05100000
	str	r1, [r0, #DMC_DIRECTCMD]
	/* chip1 MRS (MEM DLL unreset) */
	ldr	r1, =0x00100442
	str	r1, [r0, #DMC_DIRECTCMD]
	/* chip1 EMRS1 (OCD default) */
	ldr	r1, =0x00110780
	str	r1, [r0, #DMC_DIRECTCMD]
	/* chip1 EMRS1 (OCD exit) */
	ldr	r1, =0x00110400
	str	r1, [r0, #DMC_DIRECTCMD]

	/* auto refresh on */
	ldr	r1, =0x0FF02030
	str	r1, [r0, #DMC_CONCONTROL]
	ldr	r1, =0xFFFF00FF
	str	r1, [r0, #DMC_PWRDNCONFIG]
	ldr	r1, =0x00202400
	str	r1, [r0, #DMC_MEMCONTROL]

	mov	pc, lr

.ltorg
