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

#pragma once

/* S5PC110 device base addresses */
#define ELFIN_DMA_BASE					(0xE0900000)
#define ELFIN_LCD_BASE					(0xF8000000)
#define ELFIN_USB_HOST_BASE				(0xEC200000)
#define ELFIN_I2C_BASE					(0xE1800000)
#define ELFIN_I2S_BASE					(0xE2100000)
#define ELFIN_ADC_BASE					(0xE1700000)
#define ELFIN_SPI_BASE					(0xE1300000)

#define ELFIN_HSMMC_0_BASE				(0xEB000000)
#define ELFIN_HSMMC_1_BASE				(0xEB100000)
#define ELFIN_HSMMC_2_BASE				(0xEB200000)
#define ELFIN_HSMMC_3_BASE				(0xEB300000)

#define ELFIN_CLOCK_POWER_BASE			(0xE0100000)

#define IO_RET_REL						((1 << 31) | (1 << 29) | (1 << 28))

/* Clock & Power Controller for s5pc110*/
#define APLL_LOCK_OFFSET				(0x00)
#define MPLL_LOCK_OFFSET				(0x08)
#define EPLL_LOCK_OFFSET				(0x10)
#define VPLL_LOCK_OFFSET				(0x20)
#define APLL_CON0_OFFSET				(0x100)
#define APLL_CON1_OFFSET				(0x104)
#define MPLL_CON_OFFSET					(0x108)
#define EPLL_CON_OFFSET					(0x110)
#define VPLL_CON_OFFSET					(0x120)

#define CLK_SRC0_OFFSET					(0x200)
#define CLK_SRC1_OFFSET					(0x204)
#define CLK_SRC2_OFFSET					(0x208)
#define CLK_SRC3_OFFSET					(0x20c)
#define CLK_SRC4_OFFSET					(0x210)
#define CLK_SRC5_OFFSET					(0x214)
#define CLK_SRC6_OFFSET					(0x218)
#define CLK_SRC_MASK0_OFFSET			(0x280)
#define CLK_SRC_MASK1_OFFSET			(0x284)

#define CLK_DIV0_OFFSET					(0x300)
#define CLK_DIV1_OFFSET					(0x304)
#define CLK_DIV2_OFFSET					(0x308)
#define CLK_DIV3_OFFSET					(0x30c)
#define CLK_DIV4_OFFSET					(0x310)
#define CLK_DIV5_OFFSET					(0x314)
#define CLK_DIV6_OFFSET					(0x318)
#define CLK_DIV7_OFFSET					(0x31c)

#define CLK_GATE_IP0_OFFSET				(0x460)
#define CLK_GATE_IP1_OFFSET				(0x464)
#define CLK_GATE_IP2_OFFSET				(0x468)
#define CLK_GATE_IP3_OFFSET				(0x46c)
#define CLK_GATE_IP4_OFFSET				(0x470)
#define CLK_GATE_BLOCK_OFFSET			(0x480)

#define CLK_OUT_OFFSET					(0x500)
#define CLK_DIV_STAT0_OFFSET			(0x1000)
#define CLK_DIV_STAT1_OFFSET			(0x1004)
#define CLK_MUX_STAT0_OFFSET			(0x1100)
#define CLK_MUX_STAT1_OFFSET			(0x1104)
#define SW_RST_OFFSET					(0x2000)

#define ONEDRAM_CFG_OFFSET				(0x6208)

#define OSC_CON_OFFSET					(0x8000)
#define RST_STAT_OFFSET					(0xa000)
#define PWR_CFG_OFFSET					(0xc000)
#define	EINT_WAKEUP_MASK_OFFSET			(0xc004)
#define WAKEUP_MASK_OFFSET				(0xc008)
#define NORMAL_CFG_OFFSET				(0xc010)
#define IDLE_CFG_OFFSET					(0xc020)
#define STOP_CFG_OFFSET					(0xc030)
#define STOP_MEM_CFG_OFFSET				(0xc034)
#define SLEEP_CFG_OFFSET				(0xc040)
#define OSC_FREQ_OFFSET					(0xc100)
#define OSC_STABLE_OFFSET				(0xc104)
#define PWR_STABLE_OFFSET				(0xc108)
#define MTC_STABLE_OFFSET				(0xc110)
#define CLAMP_STABLE_OFFSET				(0xc114)
#define WAKEUP_STAT_OFFSET				(0xc200)
#define BLK_PWR_STAT_OFFSET				(0xc204)
#define BODY_BIAS_CON_OFFSET			(0xc300)
#define ION_SKEW_CON_OFFSET				(0xc310)
#define ION_SKEW_MON_OFFSET				(0xc314)
#define IOFF_SKEW_CON_OFFSET			(0xc320)
#define IOFF_SKEW_MON_OFFSET			(0xc324)
#define OTHERS_OFFSET					(0xe000)
#define OM_STAT_OFFSET					(0xe100)
#define MIE_CONTROL_OFFSET				(0xe800)
#define HDMI_CONTROL_OFFSET				(0xe804)
#define USB_PHY_CONTROL_OFFSET			(0xe80c)
#define DAC_CONTROL_OFFSET				(0xe810)
#define MIPI_DPHY_CONTROL_OFFSET		(0xe814)
#define ADC_CONTROL_OFFSET				(0xe818)
#define PS_HOLD_CONTROL_OFFSET			(0xe81c)

#define INFORM0_OFFSET					(0xf000)
#define INFORM1_OFFSET					(0xf004)
#define INFORM2_OFFSET					(0xf008)
#define INFORM3_OFFSET					(0xf00c)
#define INFORM4_OFFSET					(0xf010)
#define INFORM5_OFFSET					(0xf014)
#define INFORM6_OFFSET					(0xf018)
#define INFORM7_OFFSET					(0xf01c)

#define INF_REG0_OFFSET					(0x00)
#define INF_REG1_OFFSET					(0x04)
#define INF_REG2_OFFSET					(0x08)
#define INF_REG3_OFFSET					(0x0c)
#define INF_REG4_OFFSET					(0x10)
#define INF_REG5_OFFSET					(0x14)
#define INF_REG6_OFFSET					(0x18)
#define INF_REG7_OFFSET					(0x1c)


/*
 * GPIO
 */
#define ELFIN_GPIO_BASE					(0xE0200000)


#define GPA0CON_OFFSET					(0x000)
#define GPA0DAT_OFFSET					(0x004)
#define GPA0PUD_OFFSET					(0x008)
#define GPA0DRV_SR_OFFSET				(0x00C)
#define GPA0CONPDN_OFFSET				(0x010)
#define GPA0PUDPDN_OFFSET				(0x014)

#define GPA1CON_OFFSET					(0x020)
#define GPA1DAT_OFFSET					(0x024)
#define GPA1PUD_OFFSET					(0x028)
#define GPA1DRV_SR_OFFSET				(0x02C)
#define GPA1CONPDN_OFFSET				(0x030)
#define GPA1PUDPDN_OFFSET				(0x034)

#define GPBCON_OFFSET					(0x040)
#define GPBDAT_OFFSET					(0x044)
#define GPBPUD_OFFSET					(0x048)
#define GPBDRV_SR_OFFSET				(0x04C)
#define GPBCONPDN_OFFSET				(0x050)
#define GPBPUDPDN_OFFSET				(0x054)

#define GPC0CON_OFFSET					(0x060)
#define GPC0DAT_OFFSET					(0x064)
#define GPC0PUD_OFFSET					(0x068)
#define GPC0DRV_SR_OFFSET				(0x06C)
#define GPC0CONPDN_OFFSET				(0x070)
#define GPC0PUDPDN_OFFSET				(0x074)

#define GPC1CON_OFFSET					(0x080)
#define GPC1DAT_OFFSET					(0x084)
#define GPC1PUD_OFFSET					(0x088)
#define GPC1DRV_SR_OFFSET				(0x08C)
#define GPC1CONPDN_OFFSET				(0x090)
#define GPC1PUDPDN_OFFSET				(0x094)

#define GPD0CON_OFFSET					(0x0A0)
#define GPD0DAT_OFFSET					(0x0A4)
#define GPD0PUD_OFFSET					(0x0A8)
#define GPD0DRV_SR_OFFSET				(0x0AC)
#define GPD0CONPDN_OFFSET				(0x0B0)
#define GPD0PUDPDN_OFFSET				(0x0B4)

#define GPD1CON_OFFSET					(0x0C0)
#define GPD1DAT_OFFSET					(0x0C4)
#define GPD1PUD_OFFSET					(0x0C8)
#define GPD1DRV_SR_OFFSET				(0x0CC)
#define GPD1CONPDN_OFFSET				(0x0D0)
#define GPD1PUDPDN_OFFSET				(0x0D4)

#define GPE0CON_OFFSET					(0x0E0)
#define GPE0DAT_OFFSET					(0x0E4)
#define GPE0PUD_OFFSET					(0x0E8)
#define GPE0DRV_SR_OFFSET				(0x0EC)
#define GPE0CONPDN_OFFSET				(0x0F0)
#define GPE0PUDPDN_OFFSET				(0x0F4)

#define GPE1CON_OFFSET					(0x100)
#define GPE1DAT_OFFSET					(0x104)
#define GPE1PUD_OFFSET					(0x108)
#define GPE1DRV_SR_OFFSET				(0x10C)
#define GPE1CONPDN_OFFSET				(0x110)
#define GPE1PUDPDN_OFFSET				(0x114)

#define GPF0CON_OFFSET					(0x120)
#define GPF0DAT_OFFSET					(0x124)
#define GPF0PUD_OFFSET					(0x128)
#define GPF0DRV_SR_OFFSET				(0x12C)
#define GPF0CONPDN_OFFSET				(0x130)
#define GPF0PUDPDN_OFFSET				(0x134)

#define GPF1CON_OFFSET					(0x140)
#define GPF1DAT_OFFSET					(0x144)
#define GPF1PUD_OFFSET					(0x148)
#define GPF1DRV_SR_OFFSET				(0x14C)
#define GPF1CONPDN_OFFSET				(0x150)
#define GPF1PUDPDN_OFFSET				(0x154)

#define GPF2CON_OFFSET					(0x160)
#define GPF2DAT_OFFSET					(0x164)
#define GPF2PUD_OFFSET					(0x168)
#define GPF2DRV_SR_OFFSET				(0x16C)
#define GPF2CONPDN_OFFSET				(0x170)
#define GPF2PUDPDN_OFFSET				(0x174)

#define GPF3CON_OFFSET					(0x180)
#define GPF3DAT_OFFSET					(0x184)
#define GPF3PUD_OFFSET					(0x188)
#define GPF3DRV_SR_OFFSET				(0x18C)
#define GPF3CONPDN_OFFSET				(0x190)
#define GPF3PUDPDN_OFFSET				(0x194)

#define GPG0CON_OFFSET					(0x1A0)
#define GPG0DAT_OFFSET					(0x1A4)
#define GPG0PUD_OFFSET					(0x1A8)
#define GPG0DRV_SR_OFFSET				(0x1AC)
#define GPG0CONPDN_OFFSET				(0x1B0)
#define GPG0PUDPDN_OFFSET				(0x1B4)

#define GPG1CON_OFFSET					(0x1C0)
#define GPG1DAT_OFFSET					(0x1C4)
#define GPG1PUD_OFFSET					(0x1C8)
#define GPG1DRV_SR_OFFSET				(0x1CC)
#define GPG1CONPDN_OFFSET				(0x1D0)
#define GPG1PUDPDN_OFFSET				(0x1D4)

#define GPG2CON_OFFSET					(0x1E0)
#define GPG2DAT_OFFSET					(0x1E4)
#define GPG2PUD_OFFSET					(0x1E8)
#define GPG2DRV_SR_OFFSET				(0x1EC)
#define GPG2CONPDN_OFFSET				(0x1F0)
#define GPG2PUDPDN_OFFSET				(0x1F4)

#define GPG3CON_OFFSET					(0x200)
#define GPG3DAT_OFFSET					(0x204)
#define GPG3PUD_OFFSET					(0x208)
#define GPG3DRV_SR_OFFSET				(0x20C)
#define GPG3CONPDN_OFFSET				(0x210)
#define GPG3PUDPDN_OFFSET				(0x214)

#define MP1_0DRV_SR_OFFSET				(0x3CC)
#define MP1_1DRV_SR_OFFSET				(0x3EC)
#define MP1_2DRV_SR_OFFSET				(0x40C)
#define MP1_3DRV_SR_OFFSET				(0x42C)
#define MP1_4DRV_SR_OFFSET				(0x44C)
#define MP1_5DRV_SR_OFFSET				(0x46C)
#define MP1_6DRV_SR_OFFSET				(0x48C)
#define MP1_7DRV_SR_OFFSET				(0x4AC)
#define MP1_8DRV_SR_OFFSET				(0x4CC)

#define MP2_0DRV_SR_OFFSET				(0x4EC)
#define MP2_1DRV_SR_OFFSET				(0x50C)
#define MP2_2DRV_SR_OFFSET				(0x52C)
#define MP2_3DRV_SR_OFFSET				(0x54C)
#define MP2_4DRV_SR_OFFSET				(0x56C)
#define MP2_5DRV_SR_OFFSET				(0x58C)
#define MP2_6DRV_SR_OFFSET				(0x5AC)
#define MP2_7DRV_SR_OFFSET				(0x5CC)
#define MP2_8DRV_SR_OFFSET				(0x5EC)

/* GPH0 */
#define GPH0CON_OFFSET					(0xc00)
#define GPH0DAT_OFFSET					(0xc04)
#define GPH0PUD_OFFSET					(0xc08)
#define GPH0DRV_OFFSET					(0xc0c)

/* GPH1 */
#define GPH1CON_OFFSET					(0xc20)
#define GPH1DAT_OFFSET					(0xc24)
#define GPH1PUD_OFFSET					(0xc28)
#define GPH1DRV_OFFSET					(0xc2c)

/* GPH2 */
#define GPH2CON_OFFSET					(0xc40)
#define GPH2DAT_OFFSET					(0xc44)
#define GPH2PUD_OFFSET					(0xc48)
#define GPH2DRV_OFFSET					(0xc4c)

/* GPH3 */
#define GPH3CON_OFFSET					(0xc60)
#define GPH3DAT_OFFSET					(0xc64)
#define GPH3PUD_OFFSET					(0xc68)
#define GPH3DRV_OFFSET					(0xc6c)


#define GPICON_OFFSET					(0x220)
#define GPIPUD_OFFSET					(0x228)
#define GPIDRV_OFFSET_SR				(0x22C)
#define GPIPUDPDN_OFFSET				(0x234)

#define GPJ0CON_OFFSET					(0x240)
#define GPJ0DAT_OFFSET					(0x244)
#define GPJ0PUD_OFFSET					(0x248)
#define GPJ0DRV_SR_OFFSET				(0x24C)
#define GPJ0CONPDN_OFFSET				(0x250)
#define GPJ0PUDPDN_OFFSET				(0x254)

#define GPJ1CON_OFFSET					(0x260)
#define GPJ1DAT_OFFSET					(0x264)
#define GPJ1PUD_OFFSET					(0x268)
#define GPJ1DRV_SR_OFFSET				(0x26C)
#define GPJ1CONPDN_OFFSET				(0x270)
#define GPJ1PUDPDN_OFFSET				(0x274)

#define GPJ2CON_OFFSET					(0x280)
#define GPJ2DAT_OFFSET					(0x284)
#define GPJ2PUD_OFFSET					(0x288)
#define GPJ2DRV_SR_OFFSET				(0x28C)
#define GPJ2CONPDN_OFFSET				(0x290)
#define GPJ2PUDPDN_OFFSET				(0x294)

#define GPJ3CON_OFFSET					(0x2A0)
#define GPJ3DAT_OFFSET					(0x2A4)
#define GPJ3PUD_OFFSET					(0x2A8)
#define GPJ3DRV_SR_OFFSET				(0x2AC)
#define GPJ3CONPDN_OFFSET				(0x2B0)
#define GPJ3PUDPDN_OFFSET				(0x2B4)

#define GPJ4CON_OFFSET					(0x2C0)
#define GPJ4DAT_OFFSET					(0x2C4)
#define GPJ4PUD_OFFSET					(0x2C8)
#define GPJ4DRV_SR_OFFSET				(0x2CC)
#define GPJ4CONPDN_OFFSET				(0x2D0)
#define GPJ4PUDPDN_OFFSET				(0x2D4)

#define EXT_INT_0_CON					(0xE00)
#define EXT_INT_1_CON					(0xE04)
#define EXT_INT_2_CON					(0xE08)
#define EXT_INT_3_CON					(0xE0C)

#define EXT_INT_0_FLTCON0				(0xE80)
#define EXT_INT_0_FLTCON1				(0xE84)
#define EXT_INT_1_FLTCON0				(0xE88)
#define EXT_INT_1_FLTCON1				(0xE8C)
#define EXT_INT_2_FLTCON0				(0xE90)
#define EXT_INT_2_FLTCON1				(0xE94)
#define EXT_INT_3_FLTCON0				(0xE98)
#define EXT_INT_3_FLTCON1				(0xE9C)
#define EXT_INT_0_MASK					(0xF00)
#define EXT_INT_1_MASK					(0xF04)
#define EXT_INT_2_MASK					(0xF08)
#define EXT_INT_3_MASK					(0xF0C)
#define EXT_INT_0_PEND					(0xF40)
#define EXT_INT_1_PEND					(0xF44)
#define EXT_INT_2_PEND					(0xF48)
#define EXT_INT_3_PEND					(0xF4C)
#define PDNEN							(0xF80)

/*
 * Interrupt
 */
#define ELFIN_VIC0_BASE_ADDR			(0xF2000000)
#define ELFIN_VIC1_BASE_ADDR			(0xF2100000)
#define ELFIN_VIC2_BASE_ADDR			(0xF2200000)
#define ELFIN_VIC3_BASE_ADDR			(0xF2300000)

#define VIC_IRQSTATUS_OFFSET			(0x000)
#define VIC_FIQSTATUS_OFFSET			(0x004)
#define VIC_RAWINTR_OFFSET				(0x008)
#define VIC_INTSELECT_OFFSET			(0x00C)
#define VIC_INTENABLE_OFFSET			(0x010)
#define VIC_INTENCLEAR_OFFSET			(0x014)
#define VIC_SOFTINT_OFFSET				(0x018)
#define VIC_SOFTINTCLEAR_OFFSET			(0x01C)
#define VIC_PROTECTION_OFFSET			(0x020)
#define VIC_SWPRIORITYMASK_OFFSET		(0x024)
#define VIC_PRIORITYDAISY_OFFSET		(0x028)
#define VIC_VECTADDR_OFFSET(n)			(0x100 + ((n) * 4))
#define VIC_VECTPRIORITY_OFFSET(n)		(0x200 + ((n) * 4))
#define VIC_ADDRESS_OFFSET				(0xF00)
#define VIC_PERIPHID0_OFFSET			(0xFE0)
#define VIC_PERIPHID1_OFFSET			(0xFE4)
#define VIC_PERIPHID2_OFFSET			(0xFE8)
#define VIC_PERIPHID3_OFFSET			(0xFEC)
#define VIC_PCELLID0_OFFSET				(0xFF0)
#define VIC_PCELLID1_OFFSET				(0xFF4)
#define VIC_PCELLID2_OFFSET				(0xFF8)
#define VIC_PCELLID3_OFFSET				(0xFFC)

#define ELFIN_TZIC0_BASE_ADDR			(0xF2800000)
#define ELFIN_TZIC1_BASE_ADDR			(0xF2900000)
#define ELFIN_TZIC2_BASE_ADDR			(0xF2A00000)
#define ELFIN_TZIC3_BASE_ADDR			(0xF2B00000)

#define TZIC_FIQSTATUS_OFFSET			(0x000)
#define TZIC_RAWINTR_OFFSET				(0x004)
#define TZIC_INTSELECT_OFFSET			(0x008)
#define TZIC_FIQENABLE_OFFSET			(0x00C)
#define TZIC_FIQENCLEAR_OFFSET			(0x010)
#define TZIC_FIQBYPASS_OFFSET			(0x014)
#define TZIC_PROTECTION_OFFSET			(0x018)
#define TZIC_LOCK_OFFSET				(0x01C)
#define TZIC_LOCKSTATUS_OFFSET			(0x020)
#define TZIC_PERIPHID0_OFFSET			(0xFE0)
#define TZIC_PERIPHID1_OFFSET			(0xFE4)
#define TZIC_PERIPHID2_OFFSET			(0xFE8)
#define TZIC_PERIPHID3_OFFSET			(0xFEC)
#define TZIC_PCELLID0_OFFSET			(0xFF0)
#define TZIC_PCELLID1_OFFSET			(0xFF4)
#define TZIC_PCELLID2_OFFSET			(0xFF8)
#define TZIC_PCELLID3_OFFSET			(0xFFC)

#define IRQ_EINT0						(0)
#define IRQ_EINT1						(1)
#define IRQ_EINT2						(2)
#define IRQ_EINT3						(3)
#define IRQ_EINT4						(4)
#define IRQ_EINT5						(5)
#define IRQ_EINT6						(6)
#define IRQ_EINT7						(7)
#define IRQ_EINT8						(8)
#define IRQ_EINT9						(9)
#define IRQ_EINT10						(10)
#define IRQ_EINT11						(11)
#define IRQ_EINT12						(12)
#define IRQ_EINT13						(13)
#define IRQ_EINT14						(14)
#define IRQ_EINT15						(15)
#define IRQ_EINT16_31					(16)
#define IRQ_MDMA						(18)
#define IRQ_PDMA0						(19)
#define IRQ_PDMA1						(20)
#define IRQ_TIMER0						(21)
#define IRQ_TIMER1						(22)
#define IRQ_TIMER2						(23)
#define IRQ_TIMER3						(24)
#define IRQ_TIMER4						(25)
#define IRQ_SYSTIMER					(26)
#define IRQ_WDT							(27)
#define IRQ_RTC_ALARM					(28)
#define IRQ_RTC_TICK					(29)
#define IRQ_GPIOINT						(30)
#define IRQ_FIMC3						(31)

#define IRQ_CORTEX0						(32 + 0)
#define IRQ_CORTEX1						(32 + 1)
#define IRQ_CORTEX2						(32 + 2)
#define IRQ_CORTEX3						(32 + 3)
#define IRQ_CORTEX4						(32 + 4)
#define IRQ_IEM_APC						(32 + 5)
#define IRQ_IEM_IEC						(32 + 6)
#define IRQ_NFC							(32 + 8)
#define IRQ_CFC							(32 + 9)
#define IRQ_UART0						(32 + 10)
#define IRQ_UART1						(32 + 11)
#define IRQ_UART2						(32 + 12)
#define IRQ_UART3						(32 + 13)
#define IRQ_I2C							(32 + 14)
#define IRQ_SPI0						(32 + 15)
#define IRQ_SPI1						(32 + 16)
#define IRQ_SPI2						(32 + 17)
#define IRQ_AUDIO						(32 + 18)
#define IRQ_I2C_PMIC					(32 + 19)
#define IRQ_I2C_HDMI					(32 + 20)
#define IRQ_HSIRX						(32 + 21)
#define IRQ_HSITX						(32 + 22)
#define IRQ_UHOST						(32 + 23)
#define IRQ_OTG							(32 + 24)
#define IRQ_MSM							(32 + 25)
#define IRQ_HSMMC0						(32 + 26)
#define IRQ_HSMMC1						(32 + 27)
#define IRQ_HSMMC2						(32 + 28)
#define IRQ_MIPI_CSI					(32 + 29)
#define IRQ_MIPI_DSI					(32 + 30)
#define IRQ_ONENAND_AUDI				(32 + 31)

#define IRQ_LCD0						(64 + 0)
#define IRQ_LCD1						(64 + 1)
#define IRQ_LCD2						(64 + 2)
#define IRQ_LCD3						(64 + 3)
#define IRQ_ROTATOR						(64 + 4)
#define IRQ_FIMC_A						(64 + 5)
#define IRQ_FIMC_B						(64 + 6)
#define IRQ_FIMC_C						(64 + 7)
#define IRQ_JPEG						(64 + 8)
#define IRQ_2D							(64 + 9)
#define IRQ_3D							(64 + 10)
#define IRQ_MIXER						(64 + 11)
#define IRQ_HDMI						(64 + 12)
#define IRQ_HDMI_I2C					(64 + 13)
#define IRQ_MFC							(64 + 14)
#define IRQ_TVENC						(64 + 15)
#define IRQ_I2S0						(64 + 16)
#define IRQ_I2S1						(64 + 17)
#define IRQ_I2S2						(64 + 18)
#define IRQ_AC97						(64 + 19)
#define IRQ_PCM0						(64 + 20)
#define IRQ_PCM1						(64 + 21)
#define IRQ_SPDIF						(64 + 22)
#define IRQ_ADC							(64 + 23)
#define IRQ_PENDN						(64 + 24)
#define IRQ_KEYPAD						(64 + 25)
#define IRQ_HASH						(64 + 27)
#define IRQ_FEEDCTRL					(64 + 28)
#define IRQ_PCM2						(64 + 29)
#define IRQ_SDM_IRQ						(64 + 30)
#define IRQ_SDM_FIQ						(64 + 31)

#define IRQ_IPC							(96 + 0)
#define IRQ_HOSTIF						(96 + 1)
#define IRQ_HSMMC3						(96 + 2)
#define IRQ_CEC							(96 + 3)
#define IRQ_TSI							(96 + 4)
#define IRQ_MDNIE0						(96 + 5)
#define IRQ_MDNIE1						(96 + 6)
#define IRQ_MDNIE2						(96 + 7)
#define IRQ_MDNIE3						(96 + 8)
#define IRQ_ADC1						(96 + 9)
#define IRQ_PENDN1						(96 + 10)

#define IRQ_ALL							(128)

/*
 * Watchdog timer
 */
#define ELFIN_WATCHDOG_BASE				(0xE2700000)

#define WTCON_OFFSET					(0x00)
#define WTDAT_OFFSET					(0x08)
#define WTCNT_OFFSET					(0x0C)

/*
 * UART
 */
#define ELFIN_UART_BASE					(0xE2900000)

#define ELFIN_UART0_OFFSET				(0x0000)
#define ELFIN_UART1_OFFSET				(0x0400)
#define ELFIN_UART2_OFFSET				(0x0800)
#define ELFIN_UART3_OFFSET				(0x0c00)

#if defined(CONFIG_SERIAL0)
#define ELFIN_UART_CONSOLE_BASE			(ELFIN_UART_BASE + ELFIN_UART0_OFFSET)
#elif defined(CONFIG_SERIAL1)
#define ELFIN_UART_CONSOLE_BASE			(ELFIN_UART_BASE + ELFIN_UART1_OFFSET)
#elif defined(CONFIG_SERIAL2)
#define ELFIN_UART_CONSOLE_BASE			(ELFIN_UART_BASE + ELFIN_UART2_OFFSET)
#elif defined(CONFIG_SERIAL3)
#define ELFIN_UART_CONSOLE_BASE			(ELFIN_UART_BASE + ELFIN_UART3_OFFSET)
#else
#define ELFIN_UART_CONSOLE_BASE			(ELFIN_UART_BASE + ELFIN_UART0_OFFSET)
#endif

#define ULCON_OFFSET					(0x00)
#define UCON_OFFSET						(0x04)
#define UFCON_OFFSET					(0x08)
#define UMCON_OFFSET					(0x0C)
#define UTRSTAT_OFFSET					(0x10)
#define UERSTAT_OFFSET					(0x14)
#define UFSTAT_OFFSET					(0x18)
#define UMSTAT_OFFSET					(0x1C)
#define UTXH_OFFSET						(0x20)
#define URXH_OFFSET						(0x24)
#define UBRDIV_OFFSET					(0x28)
#define UDIVSLOT_OFFSET					(0x2C)
#define UINTP_OFFSET					(0x30)
#define UINTSP_OFFSET					(0x34)
#define UINTM_OFFSET					(0x38)

#define UTRSTAT_TX_EMPTY				(1 << 2)
#define UTRSTAT_RX_READY				(1 << 0)
#define UART_ERR_MASK					(0xF)


/*
 * PWM timer
 */
#define ELFIN_TIMER_BASE				(0xE2500000)

#define TCFG0_OFFSET					(0x00)
#define TCFG1_OFFSET					(0x04)
#define TCON_OFFSET						(0x08)
#define TCNTB0_OFFSET					(0x0c)
#define TCMPB0_OFFSET					(0x10)
#define TCNTO0_OFFSET					(0x14)
#define TCNTB1_OFFSET					(0x18)
#define TCMPB1_OFFSET					(0x1c)
#define TCNTO1_OFFSET					(0x20)
#define TCNTB2_OFFSET					(0x24)
#define TCMPB2_OFFSET					(0x28)
#define TCNTO2_OFFSET					(0x2c)
#define TCNTB3_OFFSET					(0x30)
#define TCMPB3_OFFSET					(0x34)
#define TCNTO3_OFFSET					(0x38)
#define TCNTB4_OFFSET					(0x3c)
#define TCNTO4_OFFSET					(0x40)
#define TINT_OFFSET						(0x44)

/* TCFG0 register fields */
#define TCFG0_DEAD_ZONE_SHIFT			16
#define TCFG0_DEAD_ZONE_MASK			(0xFF<<16)
#define TCFG0_DEAD_ZONE(_x)				((_x)<<16)
#define TCFG0_PRESCALER1_SHIFT			8
#define TCFG0_PRESCALER1_MASK			(0xFF<<8)
#define TCFG0_PRESCALER1(_x)			((_x)<<8)
#define TCFG0_PRESCALER0_SHIFT			0
#define TCFG0_PRESCALER0_MASK			(0xFF<<0)
#define TCFG0_PRESCALER0(_x)			((_x)<<0)

/* TCFG1 register fields */
#define TCFG1_DIV_MUX4_SHIFT			16
#define TCFG1_DIV_MUX4_MASK				(0xF<<16)
#define TCFG1_DIV_MUX4(_x)				((_x)<<16)
#define TCFG1_DIV_MUX3_SHIFT			12
#define TCFG1_DIV_MUX3_MASK				(0xF<<12)
#define TCFG1_DIV_MUX3(_x)				((_x)<<12)
#define TCFG1_DIV_MUX2_SHIFT			8
#define TCFG1_DIV_MUX2_MASK				(0xF<<8)
#define TCFG1_DIV_MUX2(_x)				((_x)<<8)
#define TCFG1_DIV_MUX1_SHIFT			4
#define TCFG1_DIV_MUX1_MASK				(0xF<<4)
#define TCFG1_DIV_MUX1(_x)				((_x)<<4)
#define TCFG1_DIV_MUX0_SHIFT			0
#define TCFG1_DIV_MUX0_MASK				(0xF<<0)
#define TCFG1_DIV_MUX0(_x)				((_x)<<0)

#define TCFG1_DIV_MUX_SHIFT(_t)			((_t) * 4)
#define TCFG1_DIV_MUX_MASK(_t)			(0xF << (_t))
#define TCFG1_DIV_MUIX(_t, _x)			(_x << ((_t) * 4))

#define MUX_DIV_1						0
#define MUX_DIV_2						1
#define MUX_DIV_4						2
#define MUX_DIV_8						3
#define MUX_DIV_16						4
#define MUX_SCLK_PWM					5

/* TCON register fields */
#define TCON_TIMER4_AUTO_RELOAD			(1<<22)
#define TCON_TIMER4_MANUAL_UPDATE		(1<<21)
#define TCON_TIMER4_START				(1<<20)
#define TCON_TIMER3_AUTO_RELOAD			(1<<19)
#define TCON_TIMER3_INVERT				(1<<18)
#define TCON_TIMER3_MANUAL_UPDATE		(1<<17)
#define TCON_TIMER3_START				(1<<16)
#define TCON_TIMER2_AUTO_RELOAD			(1<<15)
#define TCON_TIMER2_INVERT				(1<<14)
#define TCON_TIMER2_MANUAL_UPDATE		(1<<13)
#define TCON_TIMER2_START				(1<<12)
#define TCON_TIMER1_AUTO_RELOAD			(1<<11)
#define TCON_TIMER1_INVERT				(1<<10)
#define TCON_TIMER1_AMANUAL_UPDATE		(1<<9)
#define TCON_TIMER1_START				(1<<8)
#define TCON_DEAD_ZONE_ENABLE			(1<<4)
#define TCON_TIMER0_AUTO_RELOAD			(1<<3)
#define TCON_TIMER0_INVERT				(1<<2)
#define TCON_TIMER0_MANUAL_UPDATE		(1<<1)
#define TCON_TIMER0_START				(1<<0)

#define TCON_TIMER_AUTO_RELOAD(_x) \
		(1 << ((_x) == 0 ? (_x) + 3 : ((_x * 4) + 7)))
#define TCON_TIMER_INVERT(_x) \
		(1 << ((_x) == 0 ? (_x) + 2 : ((_x * 4) + 6)))
#define TCON_TIMER_MANUAL_UPDATE(_x) \
		(1 << ((_x) == 0 ? (_x) + 1 : ((_x * 4) + 5)))
#define TCON_TIMER_START(_x) \
		(1 << ((_x) == 0 ? (_x) : ((_x * 4) + 4)))

/* TINT register fields */
#define TINT_TIMER4_STATUS				(1<<9)
#define TINT_TIMER3_STATUS				(1<<8)
#define TINT_TIMER2_STATUS				(1<<7)
#define TINT_TIMER1_STATUS				(1<<6)
#define TINT_TIMER0_STATUS				(1<<5)
#define TINT_TIMER4_INT_ENABLE			(1<<4)
#define TINT_TIMER3_INT_ENABLE			(1<<3)
#define TINT_TIMER2_INT_ENABLE			(1<<2)
#define TINT_TIMER1_INT_ENABLE			(1<<1)
#define TINT_TIMER0_INT_ENABLE			(1<<0)

#define TINT_TIMER_STATUS(_x)			(1<<((_x)+5))
#define TINT_TIMER_ENABLE(_x)			(1<<(_x))

#define MP01CON_OFFSET					(0x2E0)
#define MP01DAT_OFFSET					(0x2E4)
#define MP01PUD_OFFSET					(0x2E8)
#define MP01DRV_SR_OFFSET				(0x2EC)
#define MP01CONPDN_OFFSET				(0x2E0)
#define MP01PUDPDN_OFFSET				(0x2E4)

#define MP02CON_OFFSET					(0x300)
#define MP02DAT_OFFSET					(0x304)
#define MP02PUD_OFFSET					(0x308)
#define MP02DRV_SR_OFFSET				(0x30c)
#define MP02CONPDN_OFFSET				(0x310)
#define MP02PUDPDN_OFFSET				(0x314)

#define MP03CON_OFFSET					(0x320)
#define MP03DAT_OFFSET					(0x324)
#define MP03PUD_OFFSET					(0x328)
#define MP03DRV_SR_OFFSET				(0x32c)
#define MP03CONPDN_OFFSET				(0x330)
#define MP03PUDPDN_OFFSET				(0x334)

#define MP06CON_OFFSET					(0x380)
#define MP06DAT_OFFSET					(0x384)
#define MP06PUD_OFFSET					(0x388)
#define MP06DRV_SR_OFFSET				(0x38C)
#define MP06CONPDN_OFFSET				(0x390)
#define MP06PUDPDN_OFFSET				(0x394)

#define MP07CON_OFFSET					(0x3A0)
#define MP07DAT_OFFSET					(0x3A4)
#define MP07PUD_OFFSET					(0x3A8)
#define MP07DRV_SR_OFFSET				(0x3AC)
#define MP07CONPDN_OFFSET				(0x3B0)
#define MP07PUDPDN_OFFSET				(0x3B4)

/*
 * Nand flash controller
 */
#define ELFIN_NAND_BASE					(0xB0E00000)
#define ELFIN_NAND_ECC_BASE				(0xB0E20000)

#define NFCONF_OFFSET					(0x00)
#define NFCONT_OFFSET					(0x04)
#define NFCMMD_OFFSET					(0x08)
#define NFADDR_OFFSET					(0x0c)
#define NFDATA_OFFSET					(0x10)
#define NFMECCDATA0_OFFSET				(0x14)
#define NFMECCDATA1_OFFSET				(0x18)
#define NFSECCDATA0_OFFSET				(0x1c)
#define NFSBLK_OFFSET					(0x20)
#define NFEBLK_OFFSET					(0x24)
#define NFSTAT_OFFSET					(0x28)
#define NFESTAT0_OFFSET					(0x2c)
#define NFESTAT1_OFFSET					(0x30)
#define NFMECC0_OFFSET					(0x34)
#define NFMECC1_OFFSET					(0x38)
#define NFSECC_OFFSET					(0x3c)
#define NFMLCBITPT_OFFSET				(0x40)
#define NFECCCONF_OFFSET				(0x000) // R/W ECC configuration register
#define NFECCCONT_OFFSET				(0x020) // R/W ECC control register
#define NFECCSTAT_OFFSET				(0x030) // R ECC status register
#define NFECCSECSTAT_OFFSET				(0x040) // R ECC sector status register
#define NFECCPRGECC0_OFFSET				(0x090) // R ECC parity code0 register for page program
#define NFECCPRGECC1_OFFSET				(0x094) // R ECC parity code1 register for page program
#define NFECCPRGECC2_OFFSET				(0x098) // R ECC parity code2 register for page program
#define NFECCPRGECC3_OFFSET				(0x09C) // R ECC parity code3 register for page program
#define NFECCPRGECC4_OFFSET				(0x0A0) // R ECC parity code4 register for page program
#define NFECCPRGECC5_OFFSET				(0x0A4) // R ECC parity code5 register for page program
#define NFECCPRGECC6_OFFSET				(0x0A8) // R ECC parity code6 register for page program
#define NFECCERL0_OFFSET				(0x0C0) // R ECC error byte location0 register
#define NFECCERL1_OFFSET				(0x0C4) // R ECC error byte location1 register
#define NFECCERL2_OFFSET				(0x0C8) // R ECC error byte location2 register
#define NFECCERL3_OFFSET				(0x0CC) // R ECC error byte location3 register
#define NFECCERL4_OFFSET				(0x0D0) // R ECC error byte location4 register
#define NFECCERL5_OFFSET				(0x0D4) // R ECC error byte location5 register
#define NFECCERL6_OFFSET				(0x0D8) // R ECC error byte location6 register
#define NFECCERL7_OFFSET				(0x0DC) // R ECC error byte location7 register
#define NFECCERP0_OFFSET				(0x0F0) // R ECC error bit pattern0 register
#define NFECCERP1_OFFSET				(0x0F4) // R ECC error bit pattern1 register
#define NFECCERP2_OFFSET				(0x0F8) // R ECC error bit pattern2 register
#define NFECCERP3_OFFSET				(0x0FC) // R ECC error bit pattern3 register
#define NFECCCONECC0_OFFSET				(0x110) // R/W ECC parity conversion code0 register
#define NFECCCONECC1_OFFSET				(0x114) // R/W ECC parity conversion code1 register
#define NFECCCONECC2_OFFSET				(0x118) // R/W ECC parity conversion code2 register
#define NFECCCONECC3_OFFSET				(0x11C) // R/W ECC parity conversion code3 register
#define NFECCCONECC4_OFFSET				(0x120) // R/W ECC parity conversion code4 register
#define NFECCCONECC5_OFFSET				(0x124) // R/W ECC parity conversion code5 register
#define NFECCCONECC6_OFFSET				(0x128) // R/W ECC parity conversion code6 register

#define NFCONF							(ELFIN_NAND_BASE+NFCONF_OFFSET)
#define NFCONT							(ELFIN_NAND_BASE+NFCONT_OFFSET)
#define NFCMMD							(ELFIN_NAND_BASE+NFCMMD_OFFSET)
#define NFADDR							(ELFIN_NAND_BASE+NFADDR_OFFSET)
#define NFDATA							(ELFIN_NAND_BASE+NFDATA_OFFSET)
#define NFMECCDATA0						(ELFIN_NAND_BASE+NFMECCDATA0_OFFSET)
#define NFMECCDATA1						(ELFIN_NAND_BASE+NFMECCDATA1_OFFSET)
#define NFSECCDATA0						(ELFIN_NAND_BASE+NFSECCDATA0_OFFSET)
#define NFSBLK							(ELFIN_NAND_BASE+NFSBLK_OFFSET)
#define NFEBLK							(ELFIN_NAND_BASE+NFEBLK_OFFSET)
#define NFSTAT							(ELFIN_NAND_BASE+NFSTAT_OFFSET)
#define NFESTAT0						(ELFIN_NAND_BASE+NFESTAT0_OFFSET)
#define NFESTAT1						(ELFIN_NAND_BASE+NFESTAT1_OFFSET)
#define NFMECC0							(ELFIN_NAND_BASE+NFMECC0_OFFSET)
#define NFMECC1							(ELFIN_NAND_BASE+NFMECC1_OFFSET)
#define NFSECC							(ELFIN_NAND_BASE+NFSECC_OFFSET)
#define NFMLCBITPT						(ELFIN_NAND_BASE+NFMLCBITPT_OFFSET)

#define NFECCCONF						(ELFIN_NAND_ECC_BASE+NFECCCONF_OFFSET)
#define NFECCCONT						(ELFIN_NAND_ECC_BASE+NFECCCONT_OFFSET)
#define NFECCSTAT						(ELFIN_NAND_ECC_BASE+NFECCSTAT_OFFSET)
#define NFECCSECSTAT					(ELFIN_NAND_ECC_BASE+NFECCSECSTAT_OFFSET)
#define NFECCPRGECC0					(ELFIN_NAND_ECC_BASE+NFECCPRGECC0_OFFSET)
#define NFECCPRGECC1					(ELFIN_NAND_ECC_BASE+NFECCPRGECC1_OFFSET)
#define NFECCPRGECC2					(ELFIN_NAND_ECC_BASE+NFECCPRGECC2_OFFSET)
#define NFECCPRGECC3					(ELFIN_NAND_ECC_BASE+NFECCPRGECC3_OFFSET)
#define NFECCPRGECC4					(ELFIN_NAND_ECC_BASE+NFECCPRGECC4_OFFSET)
#define NFECCPRGECC5					(ELFIN_NAND_ECC_BASE+NFECCPRGECC5_OFFSET)
#define NFECCPRGECC6					(ELFIN_NAND_ECC_BASE+NFECCPRGECC6_OFFSET)
#define NFECCERL0						(ELFIN_NAND_ECC_BASE+NFECCERL0_OFFSET)
#define NFECCERL1						(ELFIN_NAND_ECC_BASE+NFECCERL1_OFFSET)
#define NFECCERL2						(ELFIN_NAND_ECC_BASE+NFECCERL2_OFFSET)
#define NFECCERL3						(ELFIN_NAND_ECC_BASE+NFECCERL3_OFFSET)
#define NFECCERL4						(ELFIN_NAND_ECC_BASE+NFECCERL4_OFFSET)
#define NFECCERL5						(ELFIN_NAND_ECC_BASE+NFECCERL5_OFFSET)
#define NFECCERL6						(ELFIN_NAND_ECC_BASE+NFECCERL6_OFFSET)
#define NFECCERL7						(ELFIN_NAND_ECC_BASE+NFECCERL7_OFFSET)
#define NFECCERP0						(ELFIN_NAND_ECC_BASE+NFECCERP0_OFFSET)
#define NFECCERP1						(ELFIN_NAND_ECC_BASE+NFECCERP1_OFFSET)
#define NFECCERP2						(ELFIN_NAND_ECC_BASE+NFECCERP2_OFFSET)
#define NFECCERP3						(ELFIN_NAND_ECC_BASE+NFECCERP3_OFFSET)
#define NFECCCONECC0					(ELFIN_NAND_ECC_BASE+NFECCCONECC0_OFFSET)
#define NFECCCONECC1					(ELFIN_NAND_ECC_BASE+NFECCCONECC1_OFFSET)
#define NFECCCONECC2					(ELFIN_NAND_ECC_BASE+NFECCCONECC2_OFFSET)
#define NFECCCONECC3					(ELFIN_NAND_ECC_BASE+NFECCCONECC3_OFFSET)
#define NFECCCONECC4					(ELFIN_NAND_ECC_BASE+NFECCCONECC4_OFFSET)
#define NFECCCONECC5					(ELFIN_NAND_ECC_BASE+NFECCCONECC5_OFFSET)
#define NFECCCONECC6					(ELFIN_NAND_ECC_BASE+NFECCCONECC6_OFFSET)

#define NFCONF_ECC_MLC					(1<<24)

#define NFCONF_ECC_1BIT					(0<<23)
#define NFCONF_ECC_4BIT					(2<<23)
#define NFCONF_ECC_8BIT					(1<<23)

#define NFCONT_ECC_ENC					(1<<18)
#define NFCONT_WP						(1<<16)
#define NFCONT_MECCLOCK					(1<<7)
#define NFCONT_SECCLOCK					(1<<6)
#define NFCONT_INITMECC					(1<<5)
#define NFCONT_INITSECC					(1<<4)
#define NFCONT_INITECC					(NFCONT_INITMECC | NFCONT_INITSECC)
#define NFCONT_CS						(1<<1)
#define NFSTAT_ECCENCDONE				(1<<25)
#define NFSTAT_ECCDECDONE				(1<<24)
#define NFSTAT_RnB						(1<<0)
#define NFESTAT0_ECCBUSY				(1<<31)


/* Access Controller (TZPC) */
#define TZPC_DECPROT0SET_OFFSET			(0x804)
#define TZPC_DECPROT1SET_OFFSET			(0x810)
#define TZPC_DECPROT2SET_OFFSET			(0x81C)
#define TZPC_DECPROT3SET_OFFSET			(0x828)


/*************************************************************
 * OneNAND Controller
 *************************************************************/
#define ELFIN_ONENAND_BASE				(0xB0000000)
#define ELFIN_ONENANDCON_BASE			(ELFIN_ONENAND_BASE + (0x600000)

#define ONENAND_IF_CTRL_OFFSET			(0x100)
#define ONENAND_IF_CMD_OFFSET			(0x104)
#define ONENAND_IF_ASYNC_TIMING_CTRL_OFFSET	(0x108)
#define ONENAND_IF_STATUS_OFFSET		(0x10C)
#define DMA_SRC_ADDR_OFFSET				(0x400)
#define DMA_SRC_CFG_OFFSET				(0x404)
#define DMA_DST_ADDR_OFFSET				(0x408)
#define DMA_DST_CFG_OFFSET				(0x40C)
#define DMA_TRANS_SIZE_OFFSET			(0x414)
#define DMA_TRANS_CMD_OFFSET			(0x418)
#define DMA_TRANS_STATUS_OFFSET			(0x41C)
#define DMA_TRANS_DIR_OFFSET			(0x420)
#define SQC_SAO_OFFSET					(0x600)
#define SQC_CMD_OFFSET					(0x608)
#define SQC_STATUS_OFFSET				(0x60C)
#define SQC_CAO_OFFSET					(0x610)
#define SQC_REG_CTRL_OFFSET				(0x614)
#define SQC_REG_VAL_OFFSET				(0x618)
#define SQC_BRPAO0_OFFSET				(0x620)
#define SQC_BRPAO1_OFFSET				(0x624)
#define INTC_SQC_CLR_OFFSET				(0x1000)
#define INTC_DMA_CLR_OFFSET				(0x1004)
#define INTC_ONENAND_CLR_OFFSET			(0x1008)
#define INTC_SQC_MASK_OFFSET			(0x1020)
#define INTC_DMA_MASK_OFFSET			(0x1024)
#define INTC_ONENAND_MASK_OFFSET		(0x1028)
#define INTC_SQC_PEND_OFFSET			(0x1040)
#define INTC_DMA_PEND_OFFSET			(0x1044)
#define INTC_ONENAND_PEND_OFFSET		(0x1048)
#define INTC_SQC_STATUS_OFFSET			(0x1060)
#define INTC_DMA_STATUS_OFFSET			(0x1064)
#define INTC_ONENAND_STATUS_OFFSET		(0x1068)

/*
 * Memory controller
 */
#define ELFIN_SROM_BASE					(0xE8000000)

#define SROM_BW_REG						__REG(ELFIN_SROM_BASE+(0x0))
#define SROM_BC0_REG					__REG(ELFIN_SROM_BASE+(0x4))
#define SROM_BC1_REG					__REG(ELFIN_SROM_BASE+(0x8))
#define SROM_BC2_REG					__REG(ELFIN_SROM_BASE+(0xC))
#define SROM_BC3_REG					__REG(ELFIN_SROM_BASE+(0x10))
#define SROM_BC4_REG					__REG(ELFIN_SROM_BASE+(0x14))
#define SROM_BC5_REG					__REG(ELFIN_SROM_BASE+(0x18))

/*
 * SDRAM Controller
 */
#define APB_DMC_0_BASE					(0xF0000000)
#define APB_DMC_1_BASE					(0xF1400000)
#define ASYNC_MSYS_DMC0_BASE			(0xF1E00000)

#define DMC_CONCONTROL					(0x00)
#define DMC_MEMCONTROL					(0x04)
#define DMC_MEMCONFIG0					(0x08)
#define DMC_MEMCONFIG1					(0x0C)
#define DMC_DIRECTCMD					(0x10)
#define DMC_PRECHCONFIG					(0x14)
#define DMC_PHYCONTROL0					(0x18)
#define DMC_PHYCONTROL1					(0x1C)
#define DMC_RESERVED					(0x20)
#define DMC_PWRDNCONFIG					(0x28)
#define DMC_TIMINGAREF					(0x30)
#define DMC_TIMINGROW					(0x34)
#define DMC_TIMINGDATA					(0x38)
#define DMC_TIMINGPOWER					(0x3C)
#define DMC_PHYSTATUS					(0x40)
#define DMC_CHIP0STATUS					(0x48)
#define DMC_CHIP1STATUS					(0x4C)
#define DMC_AREFSTATUS					(0x50)
#define DMC_MRSTATUS					(0x54)
#define DMC_PHYTEST0					(0x58)
#define DMC_PHYTEST1					(0x5C)
#define DMC_QOSCONTROL0					(0x60)
#define DMC_QOSCONFIG0					(0x64)
#define DMC_QOSCONTROL1					(0x68)
#define DMC_QOSCONFIG1					(0x6C)
#define DMC_QOSCONTROL2					(0x70)
#define DMC_QOSCONFIG2					(0x74)
#define DMC_QOSCONTROL3					(0x78)
#define DMC_QOSCONFIG3					(0x7C)
#define DMC_QOSCONTROL4					(0x80)
#define DMC_QOSCONFIG4					(0x84)
#define DMC_QOSCONTROL5					(0x88)
#define DMC_QOSCONFIG5					(0x8C)
#define DMC_QOSCONTROL6					(0x90)
#define DMC_QOSCONFIG6					(0x94)
#define DMC_QOSCONTROL7					(0x98)
#define DMC_QOSCONFIG7					(0x9C)
#define DMC_QOSCONTROL8					(0xA0)
#define DMC_QOSCONFIG8					(0xA4)
#define DMC_QOSCONTROL9					(0xA8)
#define DMC_QOSCONFIG9					(0xAC)
#define DMC_QOSCONTROL10				(0xB0)
#define DMC_QOSCONFIG10					(0xB4)
#define DMC_QOSCONTROL11				(0xB8)
#define DMC_QOSCONFIG11					(0xBC)
#define DMC_QOSCONTROL12				(0xC0)
#define DMC_QOSCONFIG12					(0xC4)
#define DMC_QOSCONTROL13				(0xC8)
#define DMC_QOSCONFIG13					(0xCC)
#define DMC_QOSCONTROL14				(0xD0)
#define DMC_QOSCONFIG14					(0xD4)
#define DMC_QOSCONTROL15				(0xD8)
#define DMC_QOSCONFIG15					(0xDC)


/*
* Memory Chip direct command
*/

/****************************************************************
 Definitions for memory configuration
 Set memory configuration
	active_chips		= 1'b0 (1 chip)
	qos_master_chip	= 3'b000(ARID[3:0])
	memory burst		= 3'b010(burst 4)
	stop_mem_clock		= 1'b0(disable dynamical stop)
	auto_power_down	= 1'b0(disable auto power-down mode)
	power_down_prd		= 6'b00_0000(0 cycle for auto power-down)
	ap_bit			= 1'b0 (bit position of auto-precharge is 10)
	row_bits		= 3'b010(# row address 13)
	column_bits		= 3'b010(# column address 10 )

 Set user configuration
	2'b10=SDRAM/mSDRAM, 2'b11=DDR, 2'b01=mDDR

 Set chip select for chip [n]
		row bank control, bank address (0x3000)_0000 ~ (0x37ff)_ffff
		CHIP_[n]_CFG=(0x30F8),	30: ADDR[31:24], F8: Mask[31:24]
******************************************************************/

#define INF_REG_BASE					(0xE010F000)

#define INF_REG0_REG					__REG(INF_REG_BASE+INF_REG0_OFFSET)
#define INF_REG1_REG					__REG(INF_REG_BASE+INF_REG1_OFFSET)
#define INF_REG2_REG					__REG(INF_REG_BASE+INF_REG2_OFFSET)
#define INF_REG3_REG					__REG(INF_REG_BASE+INF_REG3_OFFSET)
#define INF_REG4_REG					__REG(INF_REG_BASE+INF_REG4_OFFSET)
#define INF_REG5_REG					__REG(INF_REG_BASE+INF_REG5_OFFSET)
#define INF_REG6_REG					__REG(INF_REG_BASE+INF_REG6_OFFSET)
#define INF_REG7_REG					__REG(INF_REG_BASE+INF_REG7_OFFSET)

#define NFCONF_VAL						(7<<12)|(7<<8)|(7<<4)|(0<<3)|(0<<2)|(1<<1)|(0<<0)
#define NFCONT_VAL						(0x1)<<23)|(0x1)<<22)|(0<<18)|(0<<17)|(0<<16)|(0<<10)|(0<<9)|(0<<8)|(0<<7)|(0<<6)|(0x2)<<1)|(1<<0)
#define MP03CON_VAL						(1<<29)|(1<<25)|(1<<21)|(1<<17)|(1<<13)|(1<<9)|(1<<5)|(1<<1)

#define ELFIN_TZPC0_BASE				(0xF1500000)
#define ELFIN_TZPC1_BASE				(0xFAD00000)
#define ELFIN_TZPC2_BASE				(0xE0600000)
#define ELFIN_TZPC3_BASE				(0xE1C00000)
