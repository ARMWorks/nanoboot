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

inline int isspace(int c)
{
    return c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' 
        || c == '\v';
}

inline int isdigit(int c)
{
    return c >= '0' && c <= '9';
}

inline int islower(int c)
{
    return c >= 'a' && c <= 'z';
}

inline int isupper(int c)
{
    return c >= 'A' && c <= 'Z';
}

inline int isalpha(int c)
{
    return isupper(c) || islower(c);
}

inline int isalnum(int c)
{
    return isalpha(c) || isdigit(c);
}

inline int tolower(int c)
{
    return isupper(c) ? c + 32 : c;
}

inline int toupper(int c)
{
    return islower(c) ? c - 32 : c;
}
