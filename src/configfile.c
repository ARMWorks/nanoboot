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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "fatfs/ff.h"
#include "configfile.h"
#include "panic.h"


static int cmdline_property(char *s)
{
    printf("cmdline = %s\n", s);
    return 0;
}

static const property_t properties[] = {
    {"cmdline", PROPERTY_TYPE_STR, {.f_str = cmdline_property}},
    {NULL},
};

static void quiet_directive() {
    printf("quiet!\n");
}

static const directive_t directives[] = {
    {"quiet", DIRECTIVE_TYPE_NIL, {.f_nil = quiet_directive}},
    {NULL},
};

static void handle_property(char *s, char *value, int lineno)
{
    const property_t *property = properties;

    while (property->name) {
        if (strcmp(s, property->name) != 0) {
            property++;
            continue;
        }

        switch (property->type) {
        case PROPERTY_TYPE_INT:
            if (property->handler.f_int(strtoul(value, NULL, 0))) {
                panic("error on line %d: invalid value for \"%s\" property\n",
                       lineno, s);
            }
            break;
        case PROPERTY_TYPE_STR:
            if (property->handler.f_str(value)) {
                panic("error on line %d: invalid value for \"%s\" property\n",
                       lineno, s);
            }
            break;
        }

        return;
    }

    panic("error on line %d: unknown property \"%s\"\n", lineno, s);
}

static void handle_directive(char *s, int lineno)
{
    const directive_t *directive;
    char *p, *arg;

    p = s;
    while (*p && *p != ' ') {
        p++;
    }
    arg = p;
    while(*arg == ' ') {
        arg++;
    }
    *p = '\0';

    directive = directives;
    while (directive->name) {
        if (strcmp(s, directive->name) != 0) {
            directive++;
            continue;
        }

        switch (directive->type) {
        case DIRECTIVE_TYPE_NIL:
            if (strlen(arg) > 0) {
                panic("error on line %d: \"%s\" directive does not take an "
                       "argument\n", lineno, s);
                break;
            }

            directive->handler.f_nil();
            break;

        case DIRECTIVE_TYPE_STR:
            if (directive->handler.f_str(arg)) {
                panic("error on line %d: invalid argument for \"%s\" "
                       "directive", lineno, s);
                break;
            }
            break;
        }
        return;
    }

    panic("error on line %d: unknown directive \"%s\"\n", lineno, s);
}

static void parse_line(char *line, int lineno)
{
    char *s;
    char *value;

    ltrim_inplace(line);

    if (*line == '\0' || *line == '#') {
        return;
    }

    s = line;

    while (*line && *line != '=' && *line != '#') {
        line++;
    }

    if (*line == '\0' || *line == '#') {
        *line = '\0';
        rtrim_inplace(s);
        handle_directive(s, lineno);
        return;
    }

    *line = '\0';
    rtrim_inplace(s);

    line++;

    value = line;
    ltrim_inplace(value);

    while (*line && *line != '#') {
        line++;
    }

    *line = '\0';
    rtrim_inplace(value);

    handle_property(s, value, lineno);
}

void read_configfile(void)
{
    FIL f;
    FRESULT fr;
    char line[512];
    int lineno = 1;

    fr = f_open(&f, "nanoboot.txt", FA_READ);
    if (fr != FR_OK) {
        panic("error opening nanoboot.txt: %d\n", (int)fr);
        while (1);
    }

    while (f_gets(line, sizeof line, &f)) {
        parse_line(line, lineno++);
    }

    f_close(&f);
}
