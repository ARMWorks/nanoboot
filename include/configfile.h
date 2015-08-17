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

typedef enum {
    DIRECTIVE_TYPE_NIL = 0,
    DIRECTIVE_TYPE_STR,
} directive_type_t;

typedef union {
    void (*f_nil)(void);
    int (*f_str)(char *s);
} directive_handler_t;

typedef struct {
    const char *name;
    directive_type_t type;
    directive_handler_t handler;
} directive_t;

typedef enum {
    PROPERTY_TYPE_INT = 0,
    PROPERTY_TYPE_STR,
} property_type_t;

typedef union {
    int (*f_int)(int n);
    int (*f_str)(char *s);
} property_handler_t;

typedef struct {
    const char *name;
    property_type_t type;
    property_handler_t handler;    
} property_t;

void read_configfile(void);

#endif /*__CONFIGFILE_H__*/
