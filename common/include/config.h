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

#define NANOBOOT_BASE (0x21F00000) /* 31 MB */
#define NANOBOOT_SIZE (512 * 1024)
#define NANOBOOT_STACK_TOP (0x22000000) /* 32 MB */
#define NANOBOOT_FIQ_STACK_SIZE (32 * 1024)
#define NANOBOOT_IRQ_STACK_SIZE (32 * 1024)
#define NANOBOOT_ABT_STACK_SIZE (32 * 1024)
#define NANOBOOT_UND_STACK_SIZE (32 * 1024)
#define NANOBOOT_SVC_STACK_SIZE (32 * 1024)
