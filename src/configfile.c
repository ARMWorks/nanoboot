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

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fatfs/ff.h"
#include "configfile.h"
#include "panic.h"
#include "config.h"

const char CMDLINE_DEFAULT[] = "console=ttySAC0,115200 root=/dev/mmcblk0p2 "
                               "rootfstype=ext4 rootwait";
const TCHAR KERNEL_DEFAULT[] = _T("zImage");
const unsigned int KERNEL_ADDRESS_DEFAULT = PHYS_SDRAM_1 + 0x8000;
const TCHAR RAMFSFILE_DEFAULT[] = _T("");
const unsigned int RAMFSADDR_DEFAULT = PHYS_SDRAM_1 + 0xA00000;

config_t config;

static void cmdline_set(char *s, int lineno)
{
    strncpy(config.cmdline, s, sizeof(config.cmdline));
    config.cmdline[sizeof(config.cmdline) - 1] = '\0';
}

static void cmdline_append(char *s, int lineno)
{
    strncat(config.cmdline, " ", sizeof(config.cmdline));
    strncat(config.cmdline, s, sizeof(config.cmdline));
}

static void kernel_set(char *s, int lineno)
{
    strncpy(config.kernel, s, sizeof(config.kernel));
    config.kernel[sizeof(config.kernel) - 1] = '\0';
}

static void kernel_address_set(char *s, int lineno)
{
    unsigned int addr = strtoul(s, NULL, 0);
    if ((addr < PHYS_SDRAM_1 + 0x8000)
        || (addr >= PHYS_SDRAM_1 + PHYS_SDRAM_1_SIZE)) {
        panic("config error on line %d: \"kernel_address\" outside of SDRAM "
              "range\n", lineno);
    }

    if (addr >= PHYS_SDRAM_1 + PHYS_SDRAM_1_SIZE - CFG_NANOBOOT_SIZE) {
        panic("config error on line %d: \"kernel_address\" is within "
              "nanoboot's reserved memory", lineno);
    }

    config.kernel_address = addr;
}

static void ramfsaddr_set(char *s, int lineno)
{
    unsigned int addr = strtoul(s, NULL, 0);
    if ((addr < PHYS_SDRAM_1 + 0x8000)
        || (addr >= PHYS_SDRAM_1 + PHYS_SDRAM_1_SIZE)) {
        panic("config error on line %d: \"ramfsaddr\" is outside of SDRAM "
              "range\n", lineno);
    }

    if (addr >= PHYS_SDRAM_1 + PHYS_SDRAM_1_SIZE - CFG_NANOBOOT_SIZE) {
        panic("config error on line %d: \"ramfsaddr\" is within nanoboot's "
              "reserved memory", lineno);
    }

    config.ramfsaddr = addr;
}

static void ramfsfile_set(char *s, int lineno)
{
    strncpy(config.ramfsfile, s, sizeof(config.ramfsfile));
    config.ramfsfile[sizeof(config.ramfsfile) - 1] = '\0';
}

static void quiet(char *s, int lineno)
{
    config.quiet = true;
}

typedef struct {
    const char *name;
    void (*set)(char *s, int lineno);
    void (*append)(char *s, int lineno);
} property_t;

static const property_t properties[] = {
    {"cmdline",        cmdline_set,        cmdline_append},
    {"kernel",         kernel_set,         NULL          },
    {"kernel_address", kernel_address_set, NULL          },
    {"ramfsaddr",      ramfsaddr_set,      NULL          },
    {"ramfsfile",      ramfsfile_set,      NULL          },
    {NULL},
};

typedef struct {
    const char *name;
    void (*call)(char *s, int lineno);
} directive_t;

static const directive_t directives[] = {
    {"quiet", quiet},
    {NULL},
};

static void handle_property(char *s, char *value, bool append, int lineno)
{
    const property_t *property = properties;

    while (property->name) {
        if (strcmp(s, property->name) != 0) {
            property++;
            continue;
        }

        if (append) {
            if (!property->append) {
                panic("config error on line %d: append not allowed for "
                      "\"%s\" property", lineno, s);
            }

            property->append(value, lineno);
        } else {
            property->set(value, lineno);
        }
        return;
    }

    panic("config error on line %d: unknown property \"%s\"\n", lineno, s);
}

static void handle_directive(char *s, int lineno)
{
    char *p = s;
    while (*p && *p != ' ') {
        p++;
    }

    char *arg = p;
    while(*arg == ' ') {
        arg++;
    }
    *p = '\0';

    const directive_t *directive = directives;
    while (directive->name) {
        if (strcmp(s, directive->name) != 0) {
            directive++;
            continue;
        }

        directive->call(arg, lineno);
        return;
    }

    panic("config error on line %d: unknown directive \"%s\"\n", lineno, s);
}

static void parse_line(char *line, int lineno)
{
    ltrim_inplace(line);

    if (*line == '\0' || *line == '#') {
        return;
    }

    char *s = line;

    while (*line && !((*line == '+' && *(line + 1) == '=') || *line == '='
                      || *line == '#')) {
        line++;
    }

    if (*line == '\0' || *line == '#') {
        *line = '\0';
        rtrim_inplace(s);
        handle_directive(s, lineno);
        return;
    }

    bool append = false;
    if (*line == '+' && *(line + 1) == '=') {
        append = true;
        *line = '\0';
        line += 2;
    } else {
        *line++ = '\0';
    }

    rtrim_inplace(s);

    char *value = line;
    ltrim_inplace(value);

    while (*line && *line != '#') {
        line++;
    }

    *line = '\0';
    rtrim_inplace(value);

    handle_property(s, value, append, lineno);
}

void read_configfile(void)
{
    FIL f;
    FRESULT fr;
    char line[512];
    int lineno = 1;

    config.quiet = false;
    strcpy(config.cmdline, CMDLINE_DEFAULT);
    config.kernel_address = KERNEL_ADDRESS_DEFAULT;
    strcpy(config.kernel, KERNEL_DEFAULT);
    config.ramfsaddr = RAMFSADDR_DEFAULT;
    strcpy(config.ramfsfile, RAMFSFILE_DEFAULT);

    fr = f_open(&f, "nanoboot.txt", FA_READ);
    if (fr == FR_OK) {
        while (f_gets(line, sizeof line, &f)) {
            parse_line(line, lineno++);
        }

        f_close(&f);
    }
}
