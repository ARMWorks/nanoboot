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

#include "cache.h"
#include "mmu.h"
#include "system.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>


uint32_t *main_tlb;
uint32_t *exc_tlb;
uint32_t *exc_phys;

__attribute__((target("arm")))
void mmu_load(void *tlb)
{
	/* set all domains for unchecked access (manager) */
    asm("mov r1, #0xFFFFFFFF\n\t"
        "mcr p15, 0, r1, c3, c0, 0" ::: "r1");

	/* load tlb */
	asm("mcr p15, 0, %0, c2, c0, 0" :: "r" (tlb) : "memory");
}

__attribute__((target("arm")))
void mmu_enable(void)
{
	uint32_t reg;
    reg = get_sctlr();
	reg |= SCTLR_M;
    dsb();
    set_sctlr(reg);
}

__attribute__((target("arm")))
void mmu_disable(void)
{
	uint32_t reg;
    reg = get_sctlr();
	reg &= ~SCTLR_M;
    dsb();
    set_sctlr(reg);
}

uint32_t mmu_init(void)
{
	main_tlb = memalign(16384, 16384);
    exc_tlb = memalign(1024, 1024);
    exc_phys = memalign(65536, 65536);
    memset(main_tlb, 0, 16384);
    memset(exc_tlb, 0, 1024);
    memset(exc_phys, 0, 65536);

    /* 0x20000000 - 0x3FFFFFFF DRAM0 */
    for (int i = 0x200; i <= 0x3FF; i++) {
        main_tlb[i]  = ((i & 0xFF0) << 20); /* 16M base address */
        main_tlb[i] |= (1 << 18); /* supersection */
        main_tlb[i] |= (3 << 10); /* unrestricted access */
        main_tlb[i] |= (3 << 2); /* cache/buffer */
        main_tlb[i] |= (2 << 0); /* section entry */
    }

    /* 0x40000000 - 0x7FFFFFFF DRAM1 */
    for (int i = 0x400; i <= 0x7FF; i++) {
        main_tlb[i]  = ((i & 0xFF0) << 20); /* 16M base address */
        main_tlb[i] |= (1 << 18); /* supersection */
        main_tlb[i] |= (3 << 10); /* unrestricted access */
        main_tlb[i] |= (3 << 2); /* cache/buffer */
        main_tlb[i] |= (2 << 0); /* section entry */
    }

    /* 0x80000000 - 0x87FFFFFF SROM0 */
    for (int i = 0x800; i <= 0x87F; i++) {
        main_tlb[i]  = ((i & 0xFF0) << 20); /* 16M base address */
        main_tlb[i] |= (1 << 18); /* supersection */
        main_tlb[i] |= (3 << 10); /* unrestricted access */
        main_tlb[i] |= (3 << 2); /* cache/buffer */
        main_tlb[i] |= (2 << 0); /* section entry */
    }

    /* 0x88000000 - 0x8FFFFFFF SROM1 */
    for (int i = 0x880; i <= 0x8FF; i++) {
        main_tlb[i]  = ((i & 0xFF0) << 20); /* 16M base address */
        main_tlb[i] |= (1 << 18); /* supersection */
        main_tlb[i] |= (3 << 10); /* unrestricted access */
        main_tlb[i] |= (3 << 2); /* cache/buffer */
        main_tlb[i] |= (2 << 0); /* section entry */
    }

    /* 0x90000000 - 0x97FFFFFF SROM2 */
    for (int i = 0x900; i <= 0x97F; i++) {
        main_tlb[i]  = ((i & 0xFF0) << 20); /* 16M base address */
        main_tlb[i] |= (1 << 18); /* supersection */
        main_tlb[i] |= (3 << 10); /* unrestricted access */
        main_tlb[i] |= (3 << 2); /* cache/buffer */
        main_tlb[i] |= (2 << 0); /* section entry */
    }

    /* 0x98000000 - 0x9FFFFFFF SROM3 */
    for (int i = 0x980; i <= 0x9FF; i++) {
        main_tlb[i]  = ((i & 0xFF0) << 20); /* 16M base address */
        main_tlb[i] |= (1 << 18); /* supersection */
        main_tlb[i] |= (3 << 10); /* unrestricted access */
        main_tlb[i] |= (3 << 2); /* cache/buffer */
        main_tlb[i] |= (2 << 0); /* section entry */
    }

    /* 0xA0000000 - 0xA7FFFFFF SROM4 */
    for (int i = 0xA00; i <= 0xA7F; i++) {
        main_tlb[i]  = ((i & 0xFF0) << 20); /* 16M base address */
        main_tlb[i] |= (1 << 18); /* supersection */
        main_tlb[i] |= (3 << 10); /* unrestricted access */
        main_tlb[i] |= (3 << 2); /* cache/buffer */
        main_tlb[i] |= (2 << 0); /* section entry */
    }

    /* 0xA8000000 - 0xAFFFFFFF SROM5 */
    for (int i = 0xA80; i <= 0xAFF; i++) {
        main_tlb[i]  = ((i & 0xFF0) << 20); /* 16M base address */
        main_tlb[i] |= (1 << 18); /* supersection */
        main_tlb[i] |= (3 << 10); /* unrestricted access */
        main_tlb[i] |= (3 << 2); /* cache/buffer */
        main_tlb[i] |= (2 << 0); /* section entry */
    }

    /* 0xB0000000 - 0xBFFFFFFF NAND */
    for (int i = 0xB00; i <= 0xBFF; i++) {
        main_tlb[i]  = ((i & 0xFF0) << 20); /* 16M base address */
        main_tlb[i] |= (1 << 18); /* supersection */
        main_tlb[i] |= (3 << 10); /* unrestricted access */
        main_tlb[i] |= (1 << 2); /* sharable device */
        main_tlb[i] |= (2 << 0); /* section entry */
    }

    /* 0xC0000000 - 0xCFFFFFFF MP3 SRAM */
    for (int i = 0xC00; i <= 0xCFF; i++) {
        main_tlb[i]  = ((i & 0xFF0) << 20); /* 16M base address */
        main_tlb[i] |= (1 << 18); /* supersection */
        main_tlb[i] |= (3 << 10); /* unrestricted access */
        main_tlb[i] |= (3 << 2); /* cache/buffer */
        main_tlb[i] |= (2 << 0); /* section entry */
    }

    /* 0xD0000000 - 0xD7FFFFFF IROM/IRAM */
    for (int i = 0xD00; i <= 0xD7F; i++) {
        main_tlb[i]  = ((i & 0xFF0) << 20); /* 16M base address */
        main_tlb[i] |= (1 << 18); /* supersection */
        main_tlb[i] |= (3 << 10); /* unrestricted access */
        main_tlb[i] |= (3 << 2); /* cache/buffer */
        main_tlb[i] |= (2 << 0); /* section entry */
    }

    /* 0xD8000000 - 0xDFFFFFFF DMZ ROM */
    for (int i = 0xD80; i <= 0xDFF; i++) {
        main_tlb[i]  = ((i & 0xFF0) << 20); /* 16M base address */
        main_tlb[i] |= (1 << 18); /* supersection */
        main_tlb[i] |= (3 << 10); /* unrestricted access */
        main_tlb[i] |= (3 << 2); /* cache/buffer */
        main_tlb[i] |= (2 << 0); /* section entry */
    }

    /* 0xE0000000 - 0xFFEFFFFF SFRs */
    for (int i = 0xE00; i <= 0xFEF; i++) {
        main_tlb[i]  = ((i & 0xFF0) << 20); /* 16M base address */
        main_tlb[i] |= (1 << 18); /* supersection */
        main_tlb[i] |= (3 << 10); /* unrestricted access */
        main_tlb[i] |= (1 << 4); /* execute never */
        main_tlb[i] |= (1 << 2); /* shareable device */
        main_tlb[i] |= (2 << 0); /* section entry */
    }
    for (int i = 0xFF0; i <= 0xFFE; i++) {
        main_tlb[i]  = (i << 20); /* 1M base address */
        main_tlb[i] |= (1 << 15); /* execute never */
        main_tlb[i] |= (3 << 10); /* unrestricted access */
        main_tlb[i] |= (1 << 2); /* shareable device */
        main_tlb[i] |= (2 << 0); /* section entry */
    }

	/* 0xFFF00000 - 0xFFFFFFFF Page table */
	main_tlb[0xFFF]  = (uint32_t) exc_tlb;
	main_tlb[0xFFF] |= (1 << 0); /* page table */
	/* 0xFFFF0000 - 0xFFFFFFFF Exception vectors */
	for (int i = 0xF0; i <= 0xFF; i++) {
		exc_tlb[i]  = (uint32_t) exc_phys;
		exc_tlb[i] |= (3 << 4); /* unrestricted access */
		exc_tlb[i] |= (3 << 2); /* cache/buffer */
		exc_tlb[i] |= (1 << 0); /* large page descriptor */
	}

    return (uint32_t) main_tlb;
}
