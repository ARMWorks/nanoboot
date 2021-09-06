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

#define NANOBOOT_BASE (0x23800000) /* 56 MB */
#define NANOBOOT_SIZE (512 * 1024)
#define NANOBOOT_STACK_TOP (0x24000000) /* 64 MB */
#define NANOBOOT_FIQ_STACK_SIZE (32 * 1024)
#define NANOBOOT_IRQ_STACK_SIZE (32 * 1024)
#define NANOBOOT_ABT_STACK_SIZE (32 * 1024)
#define NANOBOOT_UND_STACK_SIZE (32 * 1024)
#define NANOBOOT_SVC_STACK_SIZE (32 * 1024)

#define CONFIG_SYS_HZ 1000
#define CONFIG_SYS_MALLOC_LEN 0x780000
#define CONFIG_MTD_UBI_WL_THRESHOLD 4096
#define CONFIG_MTD_UBI_BEB_LIMIT 20
#define CONFIG_MTD_UBI_FASTMAP
#define CONFIG_NR_DRAM_BANKS 2
#define CONFIG_S5P
#define CONFIG_SYS_CACHELINE_SIZE 64
#define CONFIG_NAND_ECC_BCH
#define CONFIG_SYS_MAX_NAND_DEVICE 1
#define CONFIG_SYS_NAND_BASE 0xB0E00010
#define CONFIG_MTD_PARTITIONS
#define CONFIG_MTDIDS_DEFAULT "nand0=nand0"
#define CONFIG_MTDPARTS_DEFAULT "nand0:512K(boot)ro,8M(kernel),-(rootfs)"