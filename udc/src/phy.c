/* SPDX-License-Identifier: GPL-2.0 */
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

#include "udc.h"

#include "io.h"
#include "s5pv210.h"
#include "timer.h"

#include <stdbool.h>

/* Exynos USB PHY registers */
#define CLKPWR_REG(x)           (0xE0100000 + x)
#define HSPHY_REG(x)	        (0xEC100000 + x)

/* PHY power control */
#define UPHYPWR			HSPHY_REG(0x0)

#define UPHYPWR_PHY0_SUSPEND	BIT(0)
#define UPHYPWR_PHY0_PWR	BIT(3)
#define UPHYPWR_PHY0_OTG_PWR	BIT(4)
#define UPHYPWR_PHY0	( \
	UPHYPWR_PHY0_SUSPEND | \
	UPHYPWR_PHY0_PWR | \
	UPHYPWR_PHY0_OTG_PWR)

#define UPHYPWR_PHY1_SUSPEND	BIT(6)
#define UPHYPWR_PHY1_PWR	BIT(7)
#define UPHYPWR_PHY1 ( \
	UPHYPWR_PHY1_SUSPEND | \
	UPHYPWR_PHY1_PWR)

/* PHY clock control */
#define UPHYCLK			HSPHY_REG(0x4)

#define UPHYCLK_PHYFSEL_MASK	(0x3 << 0)
#define UPHYCLK_PHYFSEL_48MHZ	(0x0 << 0)
#define UPHYCLK_PHYFSEL_24MHZ	(0x3 << 0)
#define UPHYCLK_PHYFSEL_12MHZ	(0x2 << 0)

#define UPHYCLK_PHY0_ID_PULLUP	BIT(2)
#define UPHYCLK_PHY0_COMMON_ON	BIT(4)
#define UPHYCLK_PHY1_COMMON_ON	BIT(7)

/* PHY reset control */
#define UPHYRST			HSPHY_REG(0x8)

#define URSTCON_PHY0		BIT(0)
#define URSTCON_OTG_HLINK	BIT(1)
#define URSTCON_OTG_PHYLINK	BIT(2)
#define URSTCON_PHY1_ALL	BIT(3)
#define URSTCON_HOST_LINK_ALL	BIT(4)

/* Isolation, configured in the power management unit */
#define USB_ISOL		CLKPWR_REG(0xe80c)
#define USB_ISOL_DEVICE		BIT(0)
#define USB_ISOL_HOST		BIT(1)


void dwc2_phy_on(void)
{
    writel(readl(USB_ISOL) | USB_ISOL_DEVICE, USB_ISOL);
    writel(UPHYCLK_PHYFSEL_24MHZ, UPHYCLK);
    writel(readl(UPHYPWR) & ~(UPHYPWR_PHY0 | UPHYPWR_PHY0_OTG_PWR), UPHYPWR);
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
