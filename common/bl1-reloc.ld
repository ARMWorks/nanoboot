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

#include "config.h"

ENTRY(_entry)

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
  . = 0x23F00000;

	. = ALIGN(4);
  .text :
  {
    __image_copy_start = .;
    LONG(0);
    LONG(0);
    LONG(0);
    LONG(0);
    */startup.o(.text.entry)
    . = ALIGN(32);
    */startup.o(.text.ivt)
    */startup.o(.text)
    *(.text.early)
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
  }

  .rodata :
  {
    . = ALIGN(8);
    *(.rodata)
    *(.rodata*)
  }

  .data :
  {
    . = ALIGN(8);
    *(.data)
    *(.data*)
    . = ALIGN(16);
  	__image_copy_end = .;
  }

  .bss (NOLOAD) :
  {
    . = ALIGN(16);
    _sbss = .;         /* define a global symbol at bss start */
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(16);
    _ebss = .;         /* define a global symbol at bss end */
    __bss_end__ = _ebss;
  }

  .heap (NOLOAD) :
  {
    . = ALIGN(8);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
  }

  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }
}
