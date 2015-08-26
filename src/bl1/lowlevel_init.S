/*
 * Copyright (C) 1999, 2000, 2001 Erik Mouw (J.A.K.Mouw@its.tudelft.nl) and
 *                                Jan-Derk Bakker (J.D.Bakker@its.tudelft.nl)
 * Copyright (C) 2006, 2008 Samsung Electronics
 * Copyright (C) 2015 Jeff Kent <jeff@jkent.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include "config.h"
#include "s3c2450.h"

_TEXT_BASE:
    .word CFG_NANOBOOT_BASE

    .globl lowlevel_init
lowlevel_init:
    mov r12, lr

    /* Turn off LED (GPB5 active low) */
    ldr r0, =ELFIN_GPIO_BASE
    ldr r2, [r0, #GPBCON_OFFSET]
    bic r2, #0xC00
    orr r1, r2, #0x400
    str r1, [r0, #GPBCON_OFFSET]

    ldr r2, [r0, #GPBDAT_OFFSET]
    bic r2, #0x20
    orr r1, r2, #0x20
    str r1, [r0, #GPBDAT_OFFSET]

    /* Disable Watchdog */
    ldr r0, =ELFIN_WATCHDOG_BASE
    mov r1, #0
    str r1, [r0]

    /* Disable main and sub interrupts */
    ldr r0, =ELFIN_INTERRUPT_BASE
    mvn r1, #0x0
    str r1, [r0, #INTMSK_OFFSET]
    str r1, [r0, #INTSUBMSK_OFFSET]

    /* Set all interrupts as IRQ */
    mov r1, #0x0
    str r1, [r0, #INTMOD_OFFSET]

    bl system_clock_init
    bl uart_asm_init

    /* Check if we are running in SDRAM */
    ldr r0, =0xff000fff
    bic r1, pc, r0      /* r1 <- current base addr of code */
    ldr r2, _TEXT_BASE
    bic r2, r2, r0      /* r2 <- target base addr in ram   */
    cmp r1, r2          /* compare r1, r2                  */
    beq 1f              /* r1 == r2 then skip sdram init   */

    adr r0, sdram_bank_set_val
    bl sdr_ctrl_asm_init

1:  /* Set hsmmc drive strength */
    ldr r0, =ELFIN_GPIO_BASE
    ldr r1, [r0, #DSC3_OFFSET]
    orr r1, r1, #0x0c
    str r1, [r0, #DSC3_OFFSET]

#ifdef CONFIG_PM
    /* Check if this is a wake-up from sleep */
    ldr r0, =INFORM0_REG
    ldr r1, [r0]
    ldr r2, =0x2BED
    cmp r2, r1

    ldreq r0, =INFORM1_REG
    ldreq r1, [r0]
    moveq pc, r1
#endif

    mov pc, r12

    .ltorg

/*
 * void system_clock_init(void)
 * Initialize core clock and bus clock.
 */
system_clock_init:
    ldr r0, =ELFIN_CLOCK_POWER_BASE

    ldr r1, [r0, #CLKDIV0CON_OFFSET]
    bic r1, r1, #0x37       /* clear HCLKDIV, PREDIV, PCLKDIV */
    bic r1, r1, #(0xf<<9)   /* clear ARMCLKDIV */
    ldr r2, =CLK_DIV_VAL
    orr r1, r1, r2
    str r1, [r0, #CLKDIV0CON_OFFSET]

    ldr r1, [r0, #CLKDIV1CON_OFFSET]
    bic r1, r1, #(0xF<<4)
    orr r1, r1, #(0x5<<4)   /* USBHOSTDIV = 1, HSMMCDIV_1 = 1 */
    str r1, [r0, #CLKDIV1CON_OFFSET]

    /* Set MPLL Locktime */
    ldr r1, =0xe10
    str r1, [r0, #LOCKCON0_OFFSET]

    ldr r1, =MPLL_VAL
    str r1, [r0, #MPLLCON_OFFSET]

    ldr r1, =0x1780
    str r1, [r0, #LOCKCON1_OFFSET]

    /* Set EPLL */
    ldr r1, =EPLL_VAL
    str r1, [r0, #EPLLCON_OFFSET]

    ldr r1, [r0, #CLKSRCCON_OFFSET]
    orr r1, r1, #0x40
    str r1, [r0, #CLKSRCCON_OFFSET]

    /* Set CLKSRC to fit MPLL and USB */
    ldr r1, [r0, #CLKSRCCON_OFFSET]
    orr r1, r1, #0x10
    str r1, [r0, #CLKSRCCON_OFFSET]

    mov pc, lr

    .ltorg

/*
 * void uart_asm_init(void)
 * Initialize UART in asm mode, 115200bps fixed.
 */
uart_asm_init:
    /* Set GPIO to enable UART */
    ldr r0, =ELFIN_GPIO_BASE
    ldr r1, =0x0000aaaa
    str r1, [r0, #GPHCON_OFFSET]

    ldr r0, =ELFIN_UART_BASE
    mov r1, #0x0
    str r1, [r0, #UFCON_OFFSET]
    str r1, [r0, #UMCON_OFFSET]

    mov r1, #0x3
    str r1, [r0, #ULCON_OFFSET]

    ldr r1, =0x245
    str r1, [r0, #UCON_OFFSET]

    ldr r1, =0x3

    /* PCLK Is Fixed to 66Mhz */
    ldr r1, =0x23
    str r1, [r0, #UBRDIV_OFFSET]

    ldr r1, =0x0888
    str r1, [r0, #UDIVSLOT_OFFSET]

    mov pc, lr

    .ltorg

/*
 * struct sdram_val {
 *  ulong bankcfg_val;  r5, val of bankcfg
 *  ulong bankcon1_val; r6, val of bankcon1
 *  ulong bankcon2_val; r7, val of bankcon2
 *  ulong bankcon3_val; r8, val of bankcon3
 *  ulong refresh_val;  r9, val of refresh
 * }
 */

sdram_bank_set_val:
    .word CFG_BANK_CFG_VAL
    .word CFG_BANK_CON1_VAL
    .word CFG_BANK_CON2_VAL
    .word CFG_BANK_CON3_VAL
    .word CFG_BANK_REFRESH_VAL

sdram_bank_sel_val_alt:
    .word CFG_BANK_CFG_VAL_ALT
    .word CFG_BANK_CON1_VAL_ALT

/*
 * sdr_ram_asm_init: Initialize memory controller
 *
 * r0: input argument that contains memory setting value
 * r1: BANKCON1 Value
 * r2: BANKCON1 Value Modified
 * r3: Temp value
 * r4: MEMCON Base Address
 * r5-r11: setting value
 *
 */

    .globl sdr_ctrl_asm_init
sdr_ctrl_asm_init:
    mov r13, #2

    /* Set GPK port when using x32 bus width. */
    ldr r2,=GPKCON_REG
    ldr r1,=0xaaaaaaaa  @ set Sdata[31:16]
    str r1, [r2]

    /* read setting value from structure */
    ldr r4, =ELFIN_MEMCTL_BASE
    ldmia r0!, {r5-r11}
    stmia r4!, {r5-r9}

#ifdef CONFIG_PM
    /* compare INFORM2 register to confirm the dram type*/
    ldr r1, =INFORM2_REG
    ldr r1, [r1]
    ldr r2, =0xDA
    cmp r2, r1

    ldreq r4, =ELFIN_MEMCTL_BASE
    beq 1002f
#endif

1000:
    ldr r4, =ELFIN_MEMCTL_BASE

    ldr r1, [r4, #BANKCON1_OFFSET]
    ldr r2, =0x40000000         /* DDR DSQInDLL Value Setting */
    orr r1, r1, r2
    bic r1, r1, #INIT_MASK

    /* Step 1: issue PALL */
    orr r2, r1, #INIT_PALL
    str r2, [r4, #BANKCON1_OFFSET]

    /* Step 2: write 0xff into the refresh timer */
    mov r3, #0xff
    str r3, [r4, #REFRESH_OFFSET]

    /* Step 3: wait more than 120 clk */
    mov r3, #0x100
1:  subs r3, r3, #1
    bne 1b

    /* Step 4: issue MRS */
    orr r2, r1, #INIT_MRS
    str r2, [r4, #BANKCON1_OFFSET]

    /* Step 5: nornal operation value into the refresh timer */
    str r9, [r4, #REFRESH_OFFSET]

    /* Step 6: issue EMRS */
    orr r2, r1, #INIT_EMRS
    str r2, [r4, #BANKCON1_OFFSET]

    /* Step 7: Normal Mode */
    orr r2, r1, #INIT_NORMAL
    str r2, [r4, #BANKCON1_OFFSET]

    ldr r6, =(PHYS_SDRAM_1+0x4)
    ldr r7, =0x24564236
    swp r8, r7, [r6]
    swp r5, r8, [r6]
    cmp r7, r5
    beq 1001f

    sub r13, r13, #1
    cmp r13, #0
    beq 1004f

1002:
    stmia r4!, {r10-r11}
    b 1000b

/* boyko : Memory Type DDR2 */
1004:
    ldr r4, =ELFIN_MEMCTL_BASE

    /* Step 1: BANKCFG Setting */
    ldr r2, =CFG_BANK_CFG_VAL_DDR2
    str r2, [r4, #BANKCFG_OFFSET]

    ldr r1, =0x44000040
    str r1, [r4, #BANKCON1_OFFSET]

    /* Step 2: BANKCON2 Setting */
    ldr r2, =CFG_BANK_CON2_VAL_DDR2
    str r2, [r4, #BANKCON2_OFFSET]

    /* Step 3: issue PALL */
    orr r2, r1, #INIT_PALL
    str r2, [r4, #BANKCON1_OFFSET]

    /* Step 4: Issue a EMRS2 command */
    ldr r2, =0x80000000
    str r2, [r4, #BANKCON3_OFFSET]

    orr r2, r1, #INIT_EMRS
    str r2, [r4, #BANKCON1_OFFSET]

    /* Step 5: Issue a EMRS3 command */
    ldr r2, =0xc0000000
    str r2, [r4, #BANKCON3_OFFSET]

    orr r2, r1, #INIT_EMRS
    str r2, [r4, #BANKCON1_OFFSET]

    /* Step 6: Issue a EMRS1 command */
    ldr r2, =0x44000000
    str r2, [r4, #BANKCON3_OFFSET]

    orr r2, r1, #INIT_EMRS
    str r2, [r4, #BANKCON1_OFFSET]

    /* Step 7: issue MRS */
    ldr r2, =0x44000130
    str r2, [r4, #BANKCON3_OFFSET]

    orr r2, r1, #INIT_MRS
    str r2, [r4, #BANKCON1_OFFSET]

    /* Step 8: issue PALL */
    orr r2, r1, #INIT_PALL
    str r2, [r4, #BANKCON1_OFFSET]

    /* Step 9: write 0xff into the refresh timer */
    mov r3, #0xff
    str r3, [r4, #REFRESH_OFFSET]

    /* Step 10: wait more than 120 clk */
    mov r3, #0x200
10: subs r3, r3, #1
    bne 10b

    /* Step 11: issue MRS */
    ldr r2, =0x44000030
    str r2, [r4, #BANKCON3_OFFSET]

    orr r2, r1, #INIT_MRS
    str r2, [r4, #BANKCON1_OFFSET]

    /* Step 12: Issue a EMRS1 command */
    ldr r2, =0x47800030
    str r2, [r4, #BANKCON3_OFFSET]

    orr r2, r1, #INIT_EMRS
    str r2, [r4, #BANKCON1_OFFSET]

    ldr r2, =0x44000030
    str r2, [r4, #BANKCON3_OFFSET]

    orr r2, r1, #INIT_EMRS
    str r2, [r4, #BANKCON1_OFFSET]

    /* Step 13: write 0x400 into the refresh timer */
    mov r3, #0x400
    str r3, [r4, #REFRESH_OFFSET]

    /* Step 14: Normal Mode */
    orr r2, r1, #INIT_NORMAL
    str r2, [r4, #BANKCON1_OFFSET]
/* boyko : END - Memory Type DDR2 */

1001:
    mov pc, lr

    .ltorg

    .globl cleanDCache
cleanDCache:
    mrc p15, 0, pc, c7, c10, 3  /* test/clean D-Cache */
    bne cleanDCache
    mov pc, lr

    .globl cleanFlushDCache
cleanFlushDCache:
    mrc p15, 0, pc, c7, c14, 3  /* test/cleanflush D-Cache */
    bne cleanFlushDCache
    mov pc, lr

    .globl cleanFlushCache
cleanFlushCache:
    mrc p15, 0, pc, c7, c14, 3  /* test/cleanflush D-Cache */
    bne cleanFlushCache
    mcr p15, 0, r0, c7, c5, 0   /* flush I-Cache */
    mov pc, lr

    .ltorg
