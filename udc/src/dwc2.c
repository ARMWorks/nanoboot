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

#include "dwc2.h"
#include "io.h"
#include "irq.h"
#include "timer.h"
#include "udc.h"
#include "dwc2_hw.h"
#include "uart.h"

#include "linux/list.h"
#include "linux/usb/ch9.h"
#include "s5pv210.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* forward declarations */
static void dwc2_isr(void)
{
    uart0_putc('u');

    /* Clear any pending interrupts */
    writel(0xFFFFFFFF, GINTSTS);
}

static void dwc2_init(udc_t *udc)
{
    /* Configure PHY interface */
    writel((readl(GUSBCFG) & ~(GUSBCFG_ULPI_UTMI_SEL | GUSBCFG_PHYIF16)) |
            (9 << GUSBCFG_USBTRDTIM_SHIFT), GUSBCFG);

    /* Core reset */
    writel(readl(GRSTCTL) | (1 << 0), GRSTCTL);
    while (readl(GRSTCTL) & (1 << 0));
    while (!(readl(GRSTCTL) | (1 << 31)));

    /* Force device mode */
    writel((readl(GUSBCFG) & ~(GUSBCFG_FORCEHOSTMODE)) | GUSBCFG_FORCEDEVMODE,
            GUSBCFG);
    while (readl(GINTSTS) & GINTSTS_CURMODE_HOST);

    /* Stay in disconnected state until gadget is registered */
    writel(readl(DCTL) | DCTL_SFTDISCON, DCTL);

    /* Clear all pending OTG interrupts */
    writel(0xFFFFFFFF, GOTGINT);

    /* Clear any pending interrupts */
    writel(0xFFFFFFFF, GINTSTS);

    /* Mask all endpoint interrupts */
    //writel(0, DAINTMSK);

    /* Unmask interrupts */
    writel(/*GINTSTS_WKUPINT | GINTSTS_OEPINT | GINTSTS_IEPINT |
            GINTSTS_ENUMDONE | */GINTSTS_USBRST /*| GINTSTS_USBSUSP*/, GINTMSK);
    //writel(DIEPMSK_INTKNTXFEMPMSK | DIEPMSK_TIMEOUTMSK | DIEPMSK_AHBERRMSK |
    //        DIEPMSK_XFERCOMPLMSK, DIEPMSK);
    //writel(DOEPMSK_SETUPMSK | DOEPMSK_AHBERRMSK | DOEPMSK_XFERCOMPLMSK,
    //        DOEPMSK);
    writel(/*GAHBCFG_DMA_EN | (GAHBCFG_HBSTLEN_INCR4 << GAHBCFG_HBSTLEN_SHIFT) |*/
            GAHBCFG_GLBL_INTR_EN, GAHBCFG);

#if 0
    /* Setup FIFOs */
    writel(1024, GRXFSIZ);
    writel((1024 << FIFOSIZE_STARTADDR_SHIFT) |
            (1024 << FIFOSIZE_DEPTH_SHIFT), GNPTXFSIZ);
    for (int ep = 1; ep < 16; ep++) {
        writel(((2048 + (384 * ep)) << FIFOSIZE_STARTADDR_SHIFT) |
                (384 << FIFOSIZE_DEPTH_SHIFT), DPTXFSIZN(ep));
    }

    /* Flush FIFOs */
    writel(GRSTCTL_TXFNUM(0x10) | GRSTCTL_TXFFLSH | GRSTCTL_RXFFLSH, GRSTCTL);
    while (readl(GRSTCTL) & (GRSTCTL_TXFFLSH | GRSTCTL_RXFFLSH));

    /* Set HS/FS timeout calibration */
    writel(readl(GUSBCFG) & ~GUSBCFG_TOUTCAL_MASK, GUSBCFG);

    /* Set HS mode */
    writel(DCFG_DEVSPD_HS, DCFG);

    /* Enable interrupts */
    //writel(0, DAINTMSK);

    /* Enable IN and OUT endpoint interrupts */
    writel(readl(GINTMSK) | GINTSTS_OEPINT | GINTSTS_IEPINT | GINTSTS_RXFLVL,
            GINTSTS);

    /* Enable EP0 */
    writel(readl(DAINTMSK) | (1<<16) | (1<<0), DAINTMSK);

    /* Inform register programming complete */
    writel(readl(DCTL) | DCTL_PWRONPRGDONE, DCTL);
    udelay(10);
    writel(readl(DCTL) & ~DCTL_PWRONPRGDONE, DCTL);

    /* Set to read 1 8-byte packet */
    writel(DXEPTSIZ_MC(1) | DXEPTSIZ_PKTCNT(1) | DXEPTSIZ_XFERSIZE(8),
            DOEPTSIZ0);
    writel(DXEPCTL_MPS(0) | DXEPCTL_CNAK | DXEPCTL_EPENA | DXEPCTL_USBACTEP,
            DOEPCTL0);

    /* Enable, but don't activate EP0 IN */
    writel(DXEPCTL_MPS(0) | DXEPCTL_USBACTEP, DIEPCTL0);

    /* Clear global NAKs */
    writel(readl(DCTL) | DCTL_CGOUTNAK | DCTL_CGNPINNAK | DCTL_SFTDISCON,
            DCTL);

    /* Delay 3ms for host to see disconnect */
#endif
    udelay(3000);

    /* Soft connect */
    writel(readl(DCTL) & ~DCTL_SFTDISCON, DCTL);
}

void dwc2_attach(udc_t *udc)
{
    dwc2_phy_on();
    dwc2_init(udc);
    irq_set_handler(IRQ_OTG, dwc2_isr);
    irq_enable(IRQ_OTG);
}

void dwc2_detach(udc_t *udc)
{
    irq_disable(IRQ_OTG);
    dwc2_phy_off();
}

udc_driver_t dwc2_driver = {
    .attach = dwc2_attach,
    .detach = dwc2_detach,
};
