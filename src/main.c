/*
 * Copyright (C) 2015 Jeff Kent <jeff@jkent.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <asm/types.h>
#include <stdio.h>
#include "fatfs/ff.h"
#include "atags.h"
#include "config.h"

FATFS fs;

static void load_image(const char *name, void *load_at)
{
    FRESULT fr;
    FIL f;

    fr = f_open(&f, name, FA_READ);
    if (fr != FR_OK) {
        printf("error opening %s: %d\n", name, (int)fr);
        while (1);
    }

    unsigned int nread;
    fr = f_read(&f, load_at, (8*1024*1024), &nread); 
    if (fr != FR_OK) {
        printf("error reading %s: %d\n", name, (int)fr);
        while (1);
    }

    f_close(&f);

    printf("%s loaded, %d bytes\n", name, nread);
}

void main(void)
{
    FRESULT fr;

    fr = f_mount(&fs, "", 1);
    if (fr != FR_OK) {
        printf("error mounting FAT: %d\n", (int)fr);
        while (1);
    }

    void *exec_at = (void *)PHYS_SDRAM_1 + 0x8000 + (32*1024*1024);
    void *parm_at = (void *)PHYS_SDRAM_1 + 0x100;

    load_image("zImage", exec_at);

    setup_atags(parm_at);
    
    void (*theKernel)(int zero, int arch, u32 params);
    theKernel = (void (*)(int, int, u32))exec_at;

    printf("Jumping to kernel...\n");

    theKernel(0, 1685, (u32)parm_at);

    while (1);
}
