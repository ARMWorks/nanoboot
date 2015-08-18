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

#include "s3c2450.h"

void delay_init(void)
{
    FClrFld(TCFG0_REG, fTCFG0_PRE1);
    TCFG0_REG |= TCFG0_PRE1(32);
    FClrFld(TCFG1_REG, fTCFG1_MUX4);
}

void delay_us(unsigned short us)
{
    TCNTB4_REG = us;
    TCON_REG = (TCON_REG & ~(TCON_4_AUTO | TCON_4_ONOFF)) | TCON_4_UPDATE;
    TCON_REG = (TCON_REG & ~TCON_4_UPDATE) | TCON_4_ONOFF;
    while (TCNTO4_REG == 0);
    while (TCNTO4_REG > 0);
    TCON_REG &= ~TCON_4_ONOFF;
}

void delay_ms(unsigned int ms)
{
    while (ms > 65) {
        delay_us(65 * 1000);
        ms -= 65;
    }
    delay_us(ms * 1000);
}
