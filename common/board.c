#include <common.h>
#include <asm/io.h>

#include <nand.h>

static void select_chip(struct mtd_info *mtd, int cs)
{
    if (cs) {
        writel(readl(0xB0E00004) & ~(1<<1), 0xB0E00004);
    } else {
        writel(readl(0xB0E00004) | (1<<1), 0xB0E00004);
    }
}

static void cmd_ctrl(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{
    //struct nand_chip *this = mtd_to_nand(mtd);

    if (cmd == NAND_CMD_NONE) {
        return;
    }

    if (ctrl & NAND_CLE) {
        writeb(cmd, 0xB0E00008);
    } else {
        writeb(cmd, 0xB0E0000C);
    }
}

static int dev_ready(struct mtd_info *mtd)
{
    return readl(0xB0E00028) & 1;
}

int board_nand_init(struct nand_chip *nand)
{
    writel(readl(0xB0E00000) | 1 << 1, 0xB0E00000);
    writel((readl(0xB0E00004) | 1) & ~(1<<16), 0xB0E00004);

    nand->IO_ADDR_R = (void *) 0xB0E00010;
    nand->IO_ADDR_W = (void *) 0xB0E00010;

    nand->select_chip = select_chip;
    nand->cmd_ctrl = cmd_ctrl;
    nand->dev_ready = dev_ready;
    nand->ecc.mode = NAND_ECC_SOFT;
    return 0;
}