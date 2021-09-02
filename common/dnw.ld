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

ENTRY(entry)

MEMORY
{
  ROM (xr)      : ORIGIN = 0x00000000, LENGTH = 64K
  SRAM (xrw)    : ORIGIN = 0xD0020000, LENGTH = 96K
  SDRAM (xrw)   : ORIGIN = 0x22000000, LENGTH = 32M
}

SECTIONS
{
  .header :
  {
    LONG(0);
    LONG(0);
    LONG(0);
    LONG(0);
  } >SRAM

  .text :
  {
    . = ALIGN(8);
    */startup.o(.text)
    */startup.o(.text*)
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
  } >SDRAM

  .heap (NOLOAD) :
  {
    . = ALIGN(8);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
  } >SDRAM

  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }
}
