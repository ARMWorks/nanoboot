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
