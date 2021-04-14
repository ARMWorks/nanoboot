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

#include "halt.h"
#include "irq.h"
#include "io.h"
#include "s5pv210.h"
#include "system.h"
#include "uart.h"

#include <stdint.h>
#include <stdio.h>


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

//__attribute__((target("arm")))
void enable_irqs(void)
{
    uint32_t reg;
    reg = get_cpsr();
    reg &= ~CPSR_I;
    set_cpsr(reg);
}

//__attribute__((target("arm")))
void disable_irqs(void)
{
    uint32_t reg;
    reg = get_cpsr();
    reg |= CPSR_I;
    set_cpsr(reg);
}

__attribute__((interrupt("UNDEF")))
static void exc_undef(void)
{
    printf("undef\n");
    error_halt();
}

__attribute__((interrupt("SWI")))
static void exc_swi(void)
{
    printf("swi\n");
    error_halt();
}

__attribute__((interrupt("ABORT")))
static void exc_pabort(void)
{
    printf("pabort\n");
    error_halt();
}

__attribute__((interrupt("ABORT")))
static void exc_dabort(void)
{
    printf("dabort\n");
    error_halt();
}

__attribute__((interrupt("IRQ")))
static void exc_irq(void)
{
    void (*isr)(void) = NULL;

    uint32_t vic = 0;
    for (vic = 0; vic < 4; vic++) {
        if (readl(vic_irqstatus[vic]) != 0) {
            isr = (void *) readl(vic_address[vic]);
            break;
        }
    }
    isr();
    writel(0x00000000, vic_address[vic]);
}

__attribute__((interrupt("FIQ")))
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

void irq_set_handler(uint32_t irq, void *handler)
{
    if (irq < 32) {
        writel((uint32_t) handler, ELFIN_VIC0_BASE_ADDR + VIC_VECTADDR_OFFSET(irq));
    } else if (irq < 64) {
        irq &= 0x1F;
        writel((uint32_t) handler, ELFIN_VIC1_BASE_ADDR + VIC_VECTADDR_OFFSET(irq));
    } else if (irq < 96) {
        irq &= 0x1F;
        writel((uint32_t) handler, ELFIN_VIC2_BASE_ADDR + VIC_VECTADDR_OFFSET(irq));
    } else {
        irq &= 0x1F;
        writel((uint32_t) handler, ELFIN_VIC3_BASE_ADDR + VIC_VECTADDR_OFFSET(irq));
    }
}

void irq_enable(uint32_t irq)
{
    if (irq < 32) {
        writel(1 << irq, ELFIN_VIC0_BASE_ADDR + VIC_INTENABLE_OFFSET);
    } else if (irq < 64) {
        irq &= 0x1F;
        writel(1 << irq, ELFIN_VIC1_BASE_ADDR + VIC_INTENABLE_OFFSET);
    } else if (irq < 96) {
        irq &= 0x1F;
        writel(1 << irq, ELFIN_VIC2_BASE_ADDR + VIC_INTENABLE_OFFSET);
    } else if (irq < 128) {
        irq &= 0x1F;
        writel(1 << irq, ELFIN_VIC3_BASE_ADDR + VIC_INTENABLE_OFFSET);
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
        writel(1 << irq, ELFIN_VIC0_BASE_ADDR + VIC_INTENCLEAR_OFFSET);
    } else if (irq < 64) {
        irq &= 0x1F;
        writel(1 << irq, ELFIN_VIC1_BASE_ADDR + VIC_INTENCLEAR_OFFSET);
    } else if (irq < 96) {
        irq &= 0x1F;
        writel(1 << irq, ELFIN_VIC2_BASE_ADDR + VIC_INTENCLEAR_OFFSET);
    } else if (irq < 128) {
        irq &= 0x1F;
        writel(1 << irq, ELFIN_VIC3_BASE_ADDR + VIC_INTENCLEAR_OFFSET);
    } else {
        writel(0xFFFFFFFF, ELFIN_VIC0_BASE_ADDR + VIC_INTENCLEAR_OFFSET);
        writel(0xFFFFFFFF, ELFIN_VIC1_BASE_ADDR + VIC_INTENCLEAR_OFFSET);
        writel(0xFFFFFFFF, ELFIN_VIC2_BASE_ADDR + VIC_INTENCLEAR_OFFSET);
        writel(0xFFFFFFFF, ELFIN_VIC3_BASE_ADDR + VIC_INTENCLEAR_OFFSET);
    }
}
