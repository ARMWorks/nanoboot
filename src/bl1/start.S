/*
 * Copyright (C) 2006, 2007, 2008  Samsung Electronics
 * Copyright (C) 2015 Jeff Kent <jeff@jkent.net>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "config.h"

.globl _start
_start:
    b reset
    1: b 1b
    1: b 1b
    1: b 1b
    1: b 1b
    1: b 1b
    1: b 1b
    1: b 1b

.globl _bss_start
_bss_start:
    .word __bss_start

.globl _bss_end
_bss_end:
    .word _end

reset:
    /* Switch to SVC32 mode */
    mrs r0, cpsr
    bic r0, r0, #0x1f
    orr r0, r0, #0xd3
    msr cpsr, r0

    /* Turn retention IO power on during sleep wake up. */
    ldr r0, =0x4c00006c
    ldr r1, =0x4c000064
    ldr r2, [r0]
    tst r2, #0x8
    ldreq r2, [r1]
    orreq r2, r2, #0x10000  /* (1<<16) */
    streq r2, [r1]

cpu_init_crit:
    /* Flush v4 I/D caches */
    mov r0, #0
    mcr p15, 0, r0, c7, c7, 0   /* flush v3/v4 cache */
    mcr p15, 0, r0, c8, c7, 0   /* flush v4 TLB */

    /* Disable MMU stuff and caches */
    mrc p15, 0, r0, c1, c0, 0
    bic r0, r0, #0x00002300 /* clear bits 13, 9:8 (--V- --RS) */
    bic r0, r0, #0x00000087 /* clear bits 7, 2:0 (B--- -CAM) */
    orr r0, r0, #0x00000002 /* set bit 2 (A) Align */
    orr r0, r0, #0x00001000 /* set bit 12 (I) I-Cache */
    mcr p15, 0, r0, c1, c0, 0

    /* Setup clocks, uart, memory */ 
    bl lowlevel_init

    /* setup stack */
    ldr sp, =(CFG_NANOBOOT_BASE + CFG_NANOBOOT_SIZE - 0xc)
    mov fp, #0          /* no previous frame, so fp=0 */

    /* Check if we are running in SDRAM */
check_boot_device:
    ldr r0, =0xff000fff
    bic r1, pc, r0      /* r1 <- current base addr of code */
    ldr r2, =CFG_NANOBOOT_BASE
    bic r2, r2, r0      /* r2 <- target base addr in ram   */
    cmp r1, r2          /* compare r1, r2                  */
    beq after_copy      /* r1 == r2 then skip flash copy   */

    bl movi_bl2_copy

after_copy:
/* clear bss */
    ldr r0, _bss_start
    ldr r1, _bss_end
    mov r2, #0
1:  str r2, [r0]
    add r0, r0, #4
    cmp r0, r1
    ble 1b

    ldr pc, _start_main

    .ltorg

_start_main:
    .word main

    .globl raise
raise:
    nop
    mov pc, lr
