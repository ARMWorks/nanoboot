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

#include "asm/io.h"
#include "halt.h"
#include "irq.h"
#include "s5pv210.h"
#include "system.h"
#include "uart.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <strings.h>


typedef struct exc_vect_t exc_vect_t;
struct exc_vect_t {
    void (*reset)(void);
    void (*undef)(void);
    void (*swi)(void);
    void (*pabort)(void);
    void (*dabort)(void);
    void (*reserved)(void);
    void (*irq)(void);
    void (*fiq)(void);
} __attribute__((packed));

static const uint32_t vic_irqstatus[] = {
    ELFIN_VIC0_BASE_ADDR + VIC_IRQSTATUS_OFFSET,
    ELFIN_VIC1_BASE_ADDR + VIC_IRQSTATUS_OFFSET,
    ELFIN_VIC2_BASE_ADDR + VIC_IRQSTATUS_OFFSET,
    ELFIN_VIC3_BASE_ADDR + VIC_IRQSTATUS_OFFSET,
};

static const uint32_t vic_address[] = {
    ELFIN_VIC0_BASE_ADDR + VIC_ADDRESS_OFFSET,
    ELFIN_VIC1_BASE_ADDR + VIC_ADDRESS_OFFSET,
    ELFIN_VIC2_BASE_ADDR + VIC_ADDRESS_OFFSET,
    ELFIN_VIC3_BASE_ADDR + VIC_ADDRESS_OFFSET,
};

static void *irq_userdata[128] = { };

__attribute__((target("arm")))
void enable_irqs(void)
{
    uint32_t reg = get_cpsr();
    set_cpsr(reg & ~CPSR_I);
}

__attribute__((target("arm")))
bool disable_irqs(void)
{
    uint32_t reg = get_cpsr();
    set_cpsr(reg | CPSR_I);
    return !!(reg & CPSR_I);
}

__attribute__((target("arm"), interrupt("UNDEF")))
static void exc_undef(void)
{
    printf("undef\n");
    error_halt();
}

__attribute__((target("arm"), interrupt("UNDEF")))
static void exc_swi(void)
{
    printf("swi\n");
    error_halt();
}

__attribute__((target("arm"), interrupt("UNDEF")))
static void exc_pabort(void)
{
    printf("pabort\n");
    error_halt();
}

__attribute__((target("arm"), interrupt("UNDEF")))
static void exc_dabort(void)
{
    printf("dabort\n");
    error_halt();
}

__attribute__((target("arm"), interrupt("UNDEF")))
static void exc_irq(void)
{
    void (*isr)(uint32_t irq, void *pv) = NULL;

    uint32_t vic, irq = 0;
    for (vic = 0; vic < 4; vic++) {
        uint32_t status = readl(vic_irqstatus[vic]);
        if (status != 0) {
            isr = (void *) readl(vic_address[vic]);
            irq = (ffs(status) - 1) + (vic * 32);
            break;
        }
    }

    isr(irq, irq_userdata[irq]);
    writel(0x00000000, vic_address[vic]);
}

__attribute__((target("arm"), interrupt("UNDEF")))
static void exc_fiq(void)
{
    printf("fiq\n");
    error_halt();
}

void irq_init(void)
{
    irq_disable(IRQ_ALL);
    writel(0x00000000, ELFIN_VIC0_BASE_ADDR + VIC_INTSELECT_OFFSET);
    writel(0x00000000, ELFIN_VIC1_BASE_ADDR + VIC_INTSELECT_OFFSET);
    writel(0x00000000, ELFIN_VIC2_BASE_ADDR + VIC_INTSELECT_OFFSET);
    writel(0x00000000, ELFIN_VIC3_BASE_ADDR + VIC_INTSELECT_OFFSET);
    writel(0x00000000, ELFIN_VIC0_BASE_ADDR + VIC_ADDRESS_OFFSET);
    writel(0x00000000, ELFIN_VIC1_BASE_ADDR + VIC_ADDRESS_OFFSET);
    writel(0x00000000, ELFIN_VIC2_BASE_ADDR + VIC_ADDRESS_OFFSET);
    writel(0x00000000, ELFIN_VIC3_BASE_ADDR + VIC_ADDRESS_OFFSET);

    uint32_t *branch = (void *) 0xFFFF0000;
    for (int i = 0; i < 8; i++) {
        *(branch++) = 0xE59FFFF8;
    }

    exc_vect_t *address = (void *) 0xFFFF1000;
    address->reset = error_halt;
    address->undef = exc_undef;
    address->swi = exc_swi;
    address->pabort = exc_pabort;
    address->dabort = exc_dabort;
    address->reserved = error_halt;
    address->irq = exc_irq;
    address->fiq = exc_fiq;

    /* Set high interrupts */
    uint32_t reg;
    reg = get_sctlr();
    reg |= SCTLR_V;
    set_sctlr(reg);
}

void irq_set_handler(uint32_t irq, void *handler, void *pv)
{
    if (irq >= 128) {
        return;
    }

    if (irq < 32) {
        writel((uint32_t) handler, ELFIN_VIC0_BASE_ADDR +
                VIC_VECTADDR_OFFSET(irq & 0x1F));
    } else if (irq < 64) {
        writel((uint32_t) handler, ELFIN_VIC1_BASE_ADDR +
                VIC_VECTADDR_OFFSET(irq & 0x1F));
    } else if (irq < 96) {
        writel((uint32_t) handler, ELFIN_VIC2_BASE_ADDR +
                VIC_VECTADDR_OFFSET(irq & 0x1F));
    } else {
        writel((uint32_t) handler, ELFIN_VIC3_BASE_ADDR +
                VIC_VECTADDR_OFFSET(irq & 0x1F));
    }

    irq_userdata[irq] = pv;
}

void irq_enable(uint32_t irq)
{
    if (irq < 32) {
        writel(1 << (irq & 0x1F), ELFIN_VIC0_BASE_ADDR + VIC_INTENABLE_OFFSET);
    } else if (irq < 64) {
        writel(1 << (irq & 0x1F), ELFIN_VIC1_BASE_ADDR + VIC_INTENABLE_OFFSET);
    } else if (irq < 96) {
        writel(1 << (irq & 0x1F), ELFIN_VIC2_BASE_ADDR + VIC_INTENABLE_OFFSET);
    } else if (irq < 128) {
        writel(1 << (irq & 0x1F), ELFIN_VIC3_BASE_ADDR + VIC_INTENABLE_OFFSET);
    } else {
        writel(0xFFFFFFFF, ELFIN_VIC0_BASE_ADDR + VIC_INTENABLE_OFFSET);
        writel(0xFFFFFFFF, ELFIN_VIC1_BASE_ADDR + VIC_INTENABLE_OFFSET);
        writel(0xFFFFFFFF, ELFIN_VIC2_BASE_ADDR + VIC_INTENABLE_OFFSET);
        writel(0xFFFFFFFF, ELFIN_VIC3_BASE_ADDR + VIC_INTENABLE_OFFSET);
    }
}

void irq_disable(uint32_t irq)
{
    if (irq < 32) {
        writel(1 << (irq & 0x1F), ELFIN_VIC0_BASE_ADDR + VIC_INTENCLEAR_OFFSET);
    } else if (irq < 64) {
        writel(1 << (irq & 0x1F), ELFIN_VIC1_BASE_ADDR + VIC_INTENCLEAR_OFFSET);
    } else if (irq < 96) {
        writel(1 << (irq & 0x1F), ELFIN_VIC2_BASE_ADDR + VIC_INTENCLEAR_OFFSET);
    } else if (irq < 128) {
        writel(1 << (irq & 0x1F), ELFIN_VIC3_BASE_ADDR + VIC_INTENCLEAR_OFFSET);
    } else {
        writel(0xFFFFFFFF, ELFIN_VIC0_BASE_ADDR + VIC_INTENCLEAR_OFFSET);
        writel(0xFFFFFFFF, ELFIN_VIC1_BASE_ADDR + VIC_INTENCLEAR_OFFSET);
        writel(0xFFFFFFFF, ELFIN_VIC2_BASE_ADDR + VIC_INTENCLEAR_OFFSET);
        writel(0xFFFFFFFF, ELFIN_VIC3_BASE_ADDR + VIC_INTENCLEAR_OFFSET);
    }
}

bool irq_is_enabled(uint32_t irq)
{
    if (irq < 32) {
        return !!(readl(ELFIN_VIC0_BASE_ADDR + VIC_INTENABLE_OFFSET) &
            (1 << (irq & 0x1F)));
    } else if (irq < 64) {
        return !!(readl(ELFIN_VIC1_BASE_ADDR + VIC_INTENABLE_OFFSET) &
            (1 << (irq & 0x1F)));
    } else if (irq < 96) {
        return !!(readl(ELFIN_VIC2_BASE_ADDR + VIC_INTENABLE_OFFSET) &
            (1 << (irq & 0x1F)));
    } else if (irq < 128) {
        return !!(readl(ELFIN_VIC3_BASE_ADDR + VIC_INTENABLE_OFFSET) &
            (1 << (irq & 0x1F)));
    } else {
        return false;
    }
}
