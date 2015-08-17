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
#include <string.h>
#include "fatfs/ff.h"
#include "atags.h"
#include "config.h"
#include "configfile.h"
#include "panic.h"

FATFS fs;

static size_t load_image(const char *name, void *load_at)
{
    FRESULT fr;
    FIL f;

    if (!config.quiet) {
        printf("loading %s...", name);
    }

    fr = f_open(&f, name, FA_READ);
    if (fr != FR_OK) {
        panic("error opening %s: %d\n", name, (int)fr);
    }

    size_t size;
    fr = f_read(&f, load_at, (8*1024*1024), &size); 
    if (fr != FR_OK) {
        panic("error reading %s: %d\n", name, (int)fr);
    }

    f_close(&f);

    if (!config.quiet) {
        printf(" loaded %d bytes\n", size);
    }

    return size;
}

void main(void)
{
    FRESULT fr;

    fr = f_mount(&fs, "", 1);
    if (fr != FR_OK) {
        panic("error mounting FAT: %d\n", (int)fr);
    }

    read_configfile();

    void *exec_at = (void *)config.kernel_address;
    void *parm_at = (void *)PHYS_SDRAM_1 + 0x100;

    load_image(config.kernel, exec_at);

    if (strlen(config.ramfsfile)) {
        void *ramfs_at = (void *)config.ramfsaddr;
        config.ramfssize = load_image(config.ramfsfile, ramfs_at);
    }

    setup_atags(parm_at);
    
    void (*theKernel)(int zero, int arch, u32 params);
    theKernel = (void (*)(int, int, u32))exec_at;

    if (!config.quiet) {
        printf("Making jump to kernel...\n");
    }

    theKernel(0, 1685, (u32)parm_at);
}
