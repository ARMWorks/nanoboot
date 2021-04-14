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

#include "config.h"

ENTRY(bl1_startup)

MEMORY
{
  ROM (xr)      : ORIGIN = 0x00000000, LENGTH = 64K
  SRAM (xrw)    : ORIGIN = 0xD0020000, LENGTH = 96K
  SDRAM (xrw)   : ORIGIN = 0x20000000, LENGTH = 1536M
}

/* global variables */
PROVIDE (globalBlockSize = 0xD0037480);
PROVIDE (globalSDHCInfoBit = 0xD0037484);
PROVIDE (globalSDMMCBase = 0xD0037488);

/* functions */
PROVIDE (NF8_ReadPage_Adv_addr = 0xD0037F90);
PROVIDE (NF16_ReadPage_Adv_addr = 0xD0037F94);
PROVIDE (CopySDMMCtoMem_addr = 0xD0037F98);
PROVIDE (CopyMMC4_3toMem_addr = 0xD0037F9C);
PROVIDE (CopyOND_ReadMultiPages_addr = 0xD0037FA0);
PROVIDE (CopyOND_ReadMultiPages_Adv_addr = 0xD0037FA4);
PROVIDE (Copy_eSSDtoMem_addr = 0xD0037FA8);
PROVIDE (Copy_eSSDtoMem_Adv_addr = 0xD0037FAC);
PROVIDE (NF8_ReadPage_Adv128p_addr = 0xD0037FB0);

SECTIONS
{
  .header : {
    LONG(0);
    LONG(0);
    LONG(0);
    LONG(0);
  } >SRAM

  .text :
  {
    . = ALIGN(8);
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
  } >SRAM

  .rodata :
  {
    . = ALIGN(8);
    *(.rodata)
    *(.rodata*)
  } >SRAM

  .data :
  {
    . = ALIGN(8);
    *(.data)
    *(.data*)
  } >SRAM

  .bss (NOLOAD) :
  {
    . = ALIGN(8);
    _sbss = .;         /* define a global symbol at bss start */
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)
    _ebss = .;         /* define a global symbol at bss end */
    __bss_end__ = _ebss;
  } >SRAM

  .heap (NOLOAD) :
  {
    . = ALIGN(8);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
  } >SRAM

	.sdram (NOLOAD) : {
    *(.sdram)
		*(.sdram*)
	} >SDRAM

  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }
}
