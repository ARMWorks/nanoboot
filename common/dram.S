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

#include "s5pv210.h"

	.arm
	.section .text.early
	.globl	dram_init
dram_init:
	ldr		r0, =APB_DMC_0_BASE			@0xF0000000
	mov		r3, #0

	/* Skip DMC init if DLL is already running */
#if 0
1:	ldr		r1, [r0, #DMC_PHYCONTROL0]
	tst		r1, #(1<<0)					@ DLLSTART
	bne		3f
#endif

1:	ldr		r1, =0x00101000				@ INC=0x10 STARTPOINT=0x10
	str		r1, [r0, #DMC_PHYCONTROL0]
	ldr		r1, =0x00000086				@ REF=8 SHIFTC=6
	str		r1, [r0, #DMC_PHYCONTROL1]
	ldr		r1, =0x00101002				@ INC=0x10 STARTPOINT=0x10 DLLON
	str		r1, [r0, #DMC_PHYCONTROL0]
	ldr		r2, =0x00101003				@ INC=0x10 STARTPOINT=0x10 DLLSTART
	str		r2, [r0, #DMC_PHYCONTROL0]

	/* Wait for DLL lock */
2:	ldr		r1, [r0, #DMC_PHYSTATUS]
	and		r2, r1, #0x7				@ READY
	cmp		r2, #0x7
	bne		2b

	/* Force DLL lock value */
	and		r1, #0x3fc0
	mov		r2, r1, LSL #18
	orr		r2, r2, #0x100000
	orr		r2 ,r2, #0x1000
	orr		r1, r2, #0x3
	str		r1, [r0, #DMC_PHYCONTROL0]

	ldr		r1, =0x0FFF2010				@ TIMEOUT_CNT=0xFFF RD_FETCH=2 OOO
	str		r1, [r0, #DMC_CONCONTROL]
	ldr		r1, =0x00202400				@ BL=2 NUM_CHIP=0 MEM_WIDTH=2 DDR2
	str		r1, [r0, #DMC_MEMCONTROL]

	cmp		r3, #0
	ldreq	r1, =0x20E00323				@ BASE=0x20 MASK=0xE0 COL=3 ROW=2 BANK=3
	ldrne	r1, =0x40C00333				@ BASE=0x40 MASK=0xC0 COL=3 ROW=3 BANK=3
	str		r1, [r0, #DMC_MEMCONFIG0]

	ldreq	r1, =0x00E00323				@ BASE=0x00 MASK=0xE0 COL=3 ROW=2 BANK=3
	ldrne	r1, =0x00E00323				@ BASE=0x00 MASK=0xE0 COL=3 ROW=2 BANK=3
	str		r1, [r0, #DMC_MEMCONFIG1]

	ldr		r1, =0xFF000000				@ TP_CNT=0xFF
	str		r1, [r0, #DMC_PRECHCONFIG]
	ldr		r1, =0x00000618				@ 7.8us * 200MHz = 0x00000618
	str		r1, [r0, #DMC_TIMINGAREF]
	ldr		r1, =0x2B34438A				@ RFC=43 RRD=3 RP=4 RCD=4 RC=14 RAS=10
	str		r1, [r0, #DMC_TIMINGROW]
	ldr		r1, =0x24240000				@ WTR=2 WR=4 RTP=2 CL=4 WL=0 RL=0
	str		r1, [r0, #DMC_TIMINGDATA]
	ldr		r1, =0x0BDC0343				@ FAW=11 XSR=220 XP=3 CKE=4 MRD=3
	str		r1, [r0, #DMC_TIMINGPOWER]

	/* Chip 0 init */
	ldr		r1, =0x07000000				@ NOP
	str		r1, [r0, #DMC_DIRECTCMD]
	ldr		r1, =0x01000000				@ PALL
	str		r1, [r0, #DMC_DIRECTCMD]
	ldr		r1, =0x00020000				@ EMRS2
	str		r1, [r0, #DMC_DIRECTCMD]
	ldr		r1, =0x00030000				@ EMRS3
	str		r1, [r0, #DMC_DIRECTCMD]
	ldr		r1, =0x00010400				@ EMRS1 DLL ON, DQS# DISABLE
	str		r1, [r0, #DMC_DIRECTCMD]
	ldr		r1, =0x00000542				@ MRS DLL RESET, CL4, BL4, WR3
	str		r1, [r0, #DMC_DIRECTCMD]
	ldr		r1, =0x01000000				@ PALL
	str		r1, [r0, #DMC_DIRECTCMD]
	ldr		r1, =0x05000000				@ REFA
	str		r1, [r0, #DMC_DIRECTCMD]
	ldr		r1, =0x05000000				@ REFA
	str		r1, [r0, #DMC_DIRECTCMD]
	ldr		r1, =0x00000442				@ MRS DLL CLEAR RESET
	str		r1, [r0, #DMC_DIRECTCMD]
	ldr		r1, =0x00010780				@ EMRS1 OCD DEFAULT
	str		r1, [r0, #DMC_DIRECTCMD]
	ldr		r1, =0x00010400				@ EMRS1 OCD EXIT
	str		r1, [r0, #DMC_DIRECTCMD]

	ldr		r1, =0x0FF02030				@ TIMEOUT_CNT=0xFF0 RD_FETCH=2 OOO AREF
	str		r1, [r0, #DMC_CONCONTROL]
	ldr		r1, =0xFFFF00FF				@ DSREF=0xFFFF DPWRDN=0xFF
	str		r1, [r0, #DMC_PWRDNCONFIG]
	ldr		r1, =0x00202400				@ BL=2 NUM_CHIP=0 MEM_WIDTH=2 DDR2
	str		r1, [r0, #DMC_MEMCONTROL]

3:	add		r0, r0, #0x01400000
	add		r3, r3, #1
	cmp		r3, #2
	bne		1b

	mov		pc, lr

.ltorg
