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

#include "dwc2_hw.h"

#include <timer.h>

#include <asm/io.h>


void dwc2_phy_on(void)
{
    writel(readl(USB_ISOL) | USB_ISOL_DEVICE, USB_ISOL);
    writel(UPHYCLK_PHYFSEL_24MHZ, UPHYCLK);
    writel(readl(UPHYPWR) & ~UPHYPWR_PHY0, UPHYPWR);
    writel(readl(UPHYRST) | URSTCON_PHY0, UPHYRST);
    udelay(10);
    writel(readl(UPHYRST) & ~URSTCON_PHY0, UPHYRST);
    udelay(80);
}

void dwc2_phy_off(void)
{
    writel(readl(UPHYPWR) | UPHYPWR_PHY0, UPHYPWR);
    writel(readl(USB_ISOL) & ~USB_ISOL_DEVICE, USB_ISOL);
}
