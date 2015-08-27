/*
 * Copyright (C) 2004 Vincent Sanders <vince@arm.linux.org.uk>
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

#include <string.h>
#include "atags.h"
#include "config.h"
#include "configfile.h"

static struct atag *params; /* used to point at the current tag */

static void setup_core_atag(void * address, long pagesize)
{
    params = (struct atag *)address;         /* Initialise parameters to start at given address */

    params->hdr.tag = ATAG_CORE;            /* start with the core tag */
    params->hdr.size = atag_size(atag_core); /* size the tag */

    params->u.core.flags = 1;               /* ensure read-only */
    params->u.core.pagesize = pagesize;     /* systems pagesize (4k) */
    params->u.core.rootdev = 0;             /* zero root device (typicaly overidden from commandline )*/

    params = atag_next(params);              /* move pointer to next tag */
}

static void setup_ramdisk_atag(size_t size)
{
    params->hdr.tag = ATAG_RAMDISK;         /* Ramdisk tag */
    params->hdr.size = atag_size(atag_ramdisk);  /* size tag */

    params->u.ramdisk.flags = 0;            /* Load the ramdisk */
    params->u.ramdisk.size = size;          /* Decompressed ramdisk size */
    params->u.ramdisk.start = 0;            /* Unused */

    params = atag_next(params);              /* move pointer to next tag */
}

static void setup_initrd2_atag(unsigned int start, size_t size)
{
    params->hdr.tag = ATAG_INITRD2;         /* Initrd2 tag */
    params->hdr.size = atag_size(atag_initrd2);  /* size tag */

    params->u.initrd2.start = start;        /* physical start */
    params->u.initrd2.size = size;          /* compressed ramdisk size */

    params = atag_next(params);              /* move pointer to next tag */
}

static void setup_mem_atag(unsigned int start, size_t len)
{
    params->hdr.tag = ATAG_MEM;             /* Memory tag */
    params->hdr.size = atag_size(atag_mem);  /* size tag */

    params->u.mem.start = start;            /* Start of memory area (physical address) */
    params->u.mem.size = len;               /* Length of area */

    params = atag_next(params);              /* move pointer to next tag */
}

static void setup_cmdline_atag(const char * line)
{
    int linelen = strlen(line);

    if(!linelen)
        return;                             /* do not insert a tag for an empty commandline */

    params->hdr.tag = ATAG_CMDLINE;         /* Commandline tag */
    params->hdr.size = (sizeof(struct atag_header) + linelen + 1 + 4) >> 2;

    strcpy(params->u.cmdline.cmdline,line); /* place commandline into tag */

    params = atag_next(params);              /* move pointer to next tag */
}

static void setup_end_atag(void)
{
    params->hdr.tag = ATAG_NONE;            /* Empty tag ends list */
    params->hdr.size = 0;                   /* zero length */
}

void setup_atags(void *parameters)
{
    setup_core_atag(parameters, 4096);
    setup_mem_atag(PHYS_SDRAM_1, PHYS_SDRAM_1_SIZE);
    if (config.device == DEVICE_MINI2451) {
        setup_mem_atag(PHYS_SDRAM_2, PHYS_SDRAM_2_SIZE);
    }
    if (strlen(config.initramfs)) {
        setup_initrd2_atag(config.initramfs_address, config.initramfs_size);
    }
    setup_cmdline_atag(config.cmdline);
    setup_end_atag();
}
