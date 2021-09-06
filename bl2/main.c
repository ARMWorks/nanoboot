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

#include "timer.h"
#include "uart.h"

#include <linux/delay.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/ubi.h>
#include <mtd.h>
#include <nand.h>
#include <ubi_uboot.h>

#include <stdlib.h>


void main(void)
{
    uart0_set_baudrate(460800);
    uart0_puts("BL2");

    nand_init();
    mtd_probe_devices();

    struct mtd_info *mtd = get_nand_dev_by_index(0);
    printf("mtd->name=%s\n", mtd->name);


    /*const char *mtdparts = "nand0:512K(bootloader)ro,-(rootfs)";
    struct mtd_partition *parts = NULL;
    int len;
    mtd_parse_partitions(mtd, &mtdparts, &parts, &len);
    add_mtd_partitions(mtd, parts, len);
    mtd_free_parsed_partitions(parts, len);
    ubi_part("rootfs", NULL);*/

    while (1) {
        uart0_putc('.');
        udelay(1000000);
    }
}
