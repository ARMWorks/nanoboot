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

#pragma once

#include <stdbool.h>
#include <stdint.h>

extern volatile uint32_t globalBlockSize;
extern volatile uint32_t globalSDHCInfoBit;
extern volatile uint32_t globalSDMMCBase;

extern volatile uint32_t NF8_ReadPage_Adv_addr;
extern volatile uint32_t NF16_ReadPage_Adv_addr;
extern volatile uint32_t CopySDMMCtoMem_addr;
extern volatile uint32_t CopyMMC4_3toMem_addr;
extern volatile uint32_t CopyOND_ReadMultiPages_addr;
extern volatile uint32_t CopyOND_ReadMultiPages_Adv_addr;
extern volatile uint32_t Copy_eSSDtoMem_addr;
extern volatile uint32_t Copy_eSSDtoMem_Adv_addr;
extern volatile uint32_t NF8_ReadPage_Adv128p_addr;
extern volatile uint32_t CopySDMMCtoMem_addr;

/**
* This Function copies a block of page to destination memory.( 8-Bit ECC only )
* @param uint32 block : Source block address number to copy.
* @param uint32 page : Source page address number to copy.
* @param uint8 *buffer : Target Buffer pointer.
* @return int - Success or failure.
*/
typedef bool (*NF8_ReadPage_Adv_t)(uint32_t block, uint32_t page,
        uint8_t *buf);
NF8_ReadPage_Adv_t NF8_ReadPage_Adv;

/**
* This Function copies a block of page to destination memory( 4-Bit ECC only )
* @param u32 block : Source block address number to copy.
* @param u32 page : Source page address number to copy.
* @param u8 *buffer : Target Buffer pointer.
* @return int - Success or failure.
*/
typedef bool (*NF16_ReadPage_Adv_t)(uint32_t block, uint32_t page,
        uint8_t *buf);
NF16_ReadPage_Adv_t NF16_ReadPage_Adv;

/**
* This Function copy MMC(MoviNAND/iNand) Card Data to memory.
* Always use EPLL source clock.
* This function works at 20Mhz.
* @param u32 channel
* @param u32 start_block : Source block Address.
* @param u16 blocks : Number of blocks to copy.
* @param u32 *buf : Buffer to copy to.
* @param bool with_init : determined card initialization.
* @return bool - Success or failure.
*/
typedef bool (*CopySDMMCtoMem_t)(uint32_t channel, uint32_t start_block,
        uint16_t blocks, uint32_t *buf, bool init);
CopySDMMCtoMem_t CopySDMMCtoMem;

static inline void irom_init(void)
{
    NF8_ReadPage_Adv = (NF8_ReadPage_Adv_t)NF8_ReadPage_Adv_addr;
    NF16_ReadPage_Adv = (NF16_ReadPage_Adv_t)NF16_ReadPage_Adv_addr;
    CopySDMMCtoMem = (CopySDMMCtoMem_t)CopySDMMCtoMem_addr;
}
