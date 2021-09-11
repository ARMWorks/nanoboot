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

#include <stdbool.h>
#include <stdint.h>
#include <strings.h>

#include "asm/io.h"
#include "halt.h"
#include "irq.h"
#include "s5pv210.h"
#include "system.h"
#include "uart.h"


extern void *_ivt;

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

void enable_irqs(void)
{
    uint32_t reg = get_cpsr();
    set_cpsr(reg & ~CPSR_I);
}

bool disable_irqs(void)
{
    uint32_t reg = get_cpsr();
    set_cpsr(reg | CPSR_I);
    return !!(reg & CPSR_I);
}

__attribute__((target("arm"), isr("UNDEF")))
void exc_undef(void)
{
    uart0_puts("undef");
    error_halt();
}

__attribute__((target("arm"), isr("SWI")))
void exc_swi(void)
{
    uart0_puts("swi");
    error_halt();
}

__attribute__((target("arm"), isr("ABORT")))
void exc_pabort(void)
{
    uart0_puts("pabort");
    error_halt();
}

__attribute__((target("arm"), isr("ABORT")))
void exc_dabort(void)
{
    uart0_puts("dabort");
    error_halt();
}

__attribute__((target("arm"), isr("IRQ")))
void exc_irq(void)
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

__attribute__((target("arm"), isr("FIQ")))
void exc_fiq(void)
{
    uart0_puts("fiq");
    error_halt();
}

void irq_init(void)
{
    disable_irqs();
    irq_disable(IRQ_ALL);

    writel(0x00000000, ELFIN_VIC0_BASE_ADDR + VIC_INTSELECT_OFFSET);
    writel(0x00000000, ELFIN_VIC1_BASE_ADDR + VIC_INTSELECT_OFFSET);
    writel(0x00000000, ELFIN_VIC2_BASE_ADDR + VIC_INTSELECT_OFFSET);
    writel(0x00000000, ELFIN_VIC3_BASE_ADDR + VIC_INTSELECT_OFFSET);
    writel(0x00000000, ELFIN_VIC0_BASE_ADDR + VIC_ADDRESS_OFFSET);
    writel(0x00000000, ELFIN_VIC1_BASE_ADDR + VIC_ADDRESS_OFFSET);
    writel(0x00000000, ELFIN_VIC2_BASE_ADDR + VIC_ADDRESS_OFFSET);
    writel(0x00000000, ELFIN_VIC3_BASE_ADDR + VIC_ADDRESS_OFFSET);

    set_vbar((uint32_t) &_ivt);
    enable_irqs();
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
