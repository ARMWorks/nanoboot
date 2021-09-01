/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 ARM Ltd.
 *
 * ARM and ARM64 barrier instructions
 * split from armv7.h to allow sharing between ARM and ARM64
 *
 * Original copyright in armv7.h was:
 * (C) Copyright 2010 Texas Instruments, <www.ti.com> Aneesh V <aneesh@ti.com>
 *
 * Much of the original barrier code was contributed by:
 *   Valentine Barshak <valentine.barshak@cogentembedded.com>
 */
#ifndef __BARRIERS_H__
#define __BARRIERS_H__

#ifndef __ASSEMBLY__

#define ISB	asm volatile ("isb sy" : : : "memory")
#define DSB	asm volatile ("dsb sy" : : : "memory")
#define DMB	asm volatile ("dmb sy" : : : "memory")

#define isb()	ISB
#define dsb()	DSB
#define dmb()	DMB
#endif	/* __ASSEMBLY__ */
#endif	/* __BARRIERS_H__ */
