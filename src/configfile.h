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

#ifndef __CONFIGFILE_H__
#define __CONFIGFILE_H__

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    bool quiet;
    char cmdline[1024];
    char kernel[64];
    unsigned int kernel_address;
    unsigned int ramfsaddr;
    char ramfsfile[64];
    size_t ramfssize;
} config_t;

extern config_t config;

void read_configfile(void);

#endif /*__CONFIGFILE_H__*/
