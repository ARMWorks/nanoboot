#include <common.h>
#include <asm/io.h>

#include <nand.h>
#include <s5pv210.h>

#include <stdbool.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>


typedef struct controller_data controller_data_t;
struct controller_data {
    bool configured;
};

/* Nand flash oob definition for SLC 512B page size */
static struct nand_ecclayout nand_oob_slc_8 = {
	.eccbytes = 3,
	.eccpos = {
        0, 1, 2
    },
	.oobfree = {
		{
        .offset = 3,
		.length = 2
        },
		{
        .offset = 6,
		.length = 2
        }
    }
};

/* Nand flash oob definition for SLC 512B page size */
static struct nand_ecclayout nand_oob_slc_16 = {
	.eccbytes = 4,
	.eccpos = {
		1, 2, 3, 4,
	},
	.oobfree = {
		{
		.offset = 6,
		.length = 10
		}
	}
};

/* Nand flash oob definition for SLC 2KiB page size */
static struct nand_ecclayout nand_oob_slc_64 = {
	.eccbytes = 16,
	.eccpos = {
		40, 41, 42, 43, 44, 45, 46, 47,
		48, 49, 50, 51, 52, 53, 54, 55,
	},
	.oobfree = {
		{
		.offset = 2,
		.length = 38
		}
	}
};

/* Nand flash oob definition for MLC 2KiB page size */
static struct nand_ecclayout nand_oob_mlc_64 = {
	.eccbytes = 32,
	.eccpos = {
		32, 33, 34, 35, 36, 37, 38, 39,
		40, 41, 42, 43, 44, 45, 46, 47,
		48, 49, 50, 51, 52, 53, 54, 55,
		56, 57, 58, 59, 60, 61, 62, 63,
	},
	.oobfree = {
		{
		.offset = 2,
		.length = 28
		}
	}
};

/* Nand flash oob definition for 4KiB page size with 8-bit ECC */
static struct nand_ecclayout nand_oob_mlc_128 = {
	.eccbytes = 104,
	.eccpos = {
		 24,  25,  26,  27,  28,  29,  30,  31,
		 32,  33,  34,  35,  36,  37,  38,  39,
		 40,  41,  42,  43,  44,  45,  46,  47,
		 48,  49,  50,  51,  52,  53,  54,  55,
		 56,  57,  58,  59,  60,  61,  62,  63,
		 64,  65,  66,  67,  68,  69,  70,  71,
		 72,  73,  74,  75,  76,  77,  78,  79,
		 80,  81,  82,  83,  84,  85,  86,  87,
		 88,  89,  90,  91,  92,  93,  94,  95,
		 96,  97,  98,  99, 100, 101, 102, 103,
		104, 105, 106, 107, 108, 109, 110, 111,
		112, 113, 114, 115, 116, 117, 118, 119,
		120, 121, 122, 123, 124, 125, 126, 127,
    },
	.oobfree = {
		{
		.offset = 2,
		.length = 22
		}
	}
};

/* Nand flash oob definition for 8KiB page size with 16-bit ECC */
static struct nand_ecclayout nand_oob_mlc_512 = {
	.eccbytes = 416,
	.eccpos = {
         96,  97,  98,  99, 100, 101, 102, 103,
        104, 105, 106, 107, 108, 109, 110, 111,
        112, 113, 114, 115, 116, 117, 118, 119,
        120, 121, 122, 123, 124, 125, 126, 127,
        128, 129, 130, 131, 132, 133, 134, 135,
        136, 137, 138, 139, 140, 141, 142, 143,
        144, 145, 146, 147, 148, 149, 150, 151,
        152, 153, 154, 155, 156, 157, 158, 159,
        160, 161, 162, 163, 164, 165, 166, 167,
        168, 169, 170, 171, 172, 173, 174, 175,
        176, 177, 178, 179, 180, 181, 182, 183,
        184, 185, 186, 187, 188, 189, 190, 191,
        192, 193, 194, 195, 196, 197, 198, 199,
        200, 201, 202, 203, 204, 205, 206, 207,
        208, 209, 210, 211, 212, 213, 214, 215,
        216, 217, 218, 219, 220, 221, 222, 223,
        224, 225, 226, 227, 228, 229, 230, 231,
        232, 233, 234, 235, 236, 237, 238, 239,
        240, 241, 242, 243, 244, 245, 246, 247,
        248, 249, 250, 251, 252, 253, 254, 255,
        256, 257, 258, 259, 260, 261, 262, 263,
        264, 265, 266, 267, 268, 269, 270, 271,
        272, 273, 274, 275, 276, 277, 278, 279,
        280, 281, 282, 283, 284, 285, 286, 287,
        288, 289, 290, 291, 292, 293, 294, 295,
        296, 297, 298, 299, 300, 301, 302, 303,
        304, 305, 306, 307, 308, 309, 310, 311,
        312, 313, 314, 315, 316, 317, 318, 319,
        320, 321, 322, 323, 324, 325, 326, 327,
        328, 329, 330, 331, 332, 333, 334, 335,
        336, 337, 338, 339, 340, 341, 342, 343,
        344, 345, 346, 347, 348, 349, 350, 351,
        352, 353, 354, 355, 356, 357, 358, 359,
        360, 361, 362, 363, 364, 365, 366, 367,
        368, 369, 370, 371, 372, 373, 374, 375,
        376, 377, 378, 379, 380, 381, 382, 383,
        384, 385, 386, 387, 388, 389, 390, 391,
        392, 393, 394, 395, 396, 397, 398, 399,
        400, 401, 402, 403, 404, 405, 406, 407,
        408, 409, 410, 411, 412, 413, 414, 415,
        416, 417, 418, 419, 420, 421, 422, 423,
        424, 425, 426, 427, 428, 429, 430, 431,
        432, 433, 434, 435, 436, 437, 438, 439,
        440, 441, 442, 443, 444, 445, 446, 447,
        448, 449, 450, 451, 452, 453, 454, 455,
        456, 457, 458, 459, 460, 461, 462, 463,
        464, 465, 466, 467, 468, 469, 470, 471,
        472, 473, 474, 475, 476, 477, 478, 479,
        480, 481, 482, 483, 484, 485, 486, 487,
        488, 489, 490, 491, 492, 493, 494, 495,
        496, 497, 498, 499, 500, 501, 502, 503,
        504, 505, 506, 507, 508, 509, 510, 511,
    },
	.oobfree = {
		{
        .offset = 2,
		.length = 94
        }
    }
};

static int hw_calculate_ecc16(struct mtd_info *mtd, const uint8_t *dat,
        uint8_t *ecc_code)
{
    struct nand_chip *nand = mtd_to_nand(mtd);
    nand->write_buf(mtd, ecc_code, nand->ecc.bytes);

    while (readl(NFECCSTAT) & NFECCSTAT_ECCBUSY)
        ;

    memcpy(ecc_code, (void *) NFECCPRGECC0, 26);

    return 0;
}

static int hw_correct_ecc16(struct mtd_info *mtd, uint8_t *dat,
        uint8_t *read_ecc, uint8_t *calc_ecc)
{
    return -1;
}

void hwctl_ecc16(struct mtd_info *mtd, int mode)
{
    struct nand_chip *nand = mtd_to_nand(mtd);

    switch (mode) {
    case NAND_ECC_READ:
        writel((readl(NFECCCONF) & ~NFECCCONF_MSGLENGTH_MASK) |
                NFECCCONF_MSGLENGTH(nand->ecc.size - 1),
                NFECCCONF);
        writel((readl(NFECCCONT) & ~NFECCCONT_ENCODE) | NFECCCONT_INITMECC,
                NFECCCONT);
        break;
    case NAND_ECC_WRITE:
        writel((readl(NFECCCONF) & ~NFECCCONF_MSGLENGTH_MASK) |
                NFECCCONF_MSGLENGTH(nand->ecc.size - 1), NFECCCONF);
        writel(readl(NFECCCONT) | NFECCCONT_ENCODE | NFECCCONT_INITMECC,
                NFECCCONT);
        break;
    }
}

int hw_read_page_ecc16(struct mtd_info *mtd, struct nand_chip *chip,
        uint8_t *buf, int oob_required, int page)
{
    int i, eccsize = chip->ecc.size;
    int eccbytes = chip->ecc.bytes;
    int eccsteps = chip->ecc.steps;
    uint8_t *p = buf;
    uint8_t *ecc_code = chip->buffers->ecccode;
    uint32_t *eccpos = chip->ecc.layout->eccpos;
    unsigned int max_bitflips = 0;
    int ret;

    /* Read the OOB area first */
    ret = nand_read_oob_op(chip, page, 0, chip->oob_poi, mtd->oobsize);
    if (ret)
        return ret;

    for (i = 0; i < chip->ecc.total; i++) {
        ecc_code[i] = chip->oob_poi[eccpos[i]];
    }

    for (i = 0; eccsteps; eccsteps--, i += eccbytes, p += eccsize) {
        int stat;

        chip->ecc.hwctl(mtd, NAND_ECC_READ);

        ret = nand_read_data_op(chip, p, eccsize, false);
        if (ret)
            return ret;

        ret = nand_write_data_op(chip, &ecc_code[i], eccbytes, false);
        if (ret)
            return ret;

        stat = chip->ecc.correct(mtd, p, &ecc_code[i], NULL);
        if (stat == -EBADMSG &&
                (chip->ecc.options & NAND_ECC_GENERIC_ERASED_CHECK)) {
            stat = nand_check_erased_ecc_chunk(p, eccsize, &ecc_code[i],
                    eccbytes, NULL, 0, chip->ecc.strength);
        }

        if (stat < 0) {
            mtd->ecc_stats.failed++;
        } else {
            mtd->ecc_stats.corrected += stat;
            max_bitflips = max_t(unsigned int, max_bitflips, stat);
        }
    }
    return max_bitflips;
}

static void select_chip(struct mtd_info *mtd, int cs)
{
    struct nand_chip *nand = mtd_to_nand(mtd);
    controller_data_t *controller = nand_get_controller_data(nand);

    if (cs == -1 && mtd->oobsize != 0 && !controller->configured) {
        nand->ecc.size = 512;
        switch (mtd->oobsize) {
        case 8:
            nand->ecc.layout = &nand_oob_slc_8;
            break;
        case 16:
            nand->ecc.layout = &nand_oob_slc_16;
            break;
        case 64:
            if (nand_is_slc(nand)) {
                nand->ecc.layout = &nand_oob_slc_64;
            } else {
                nand->ecc.layout = &nand_oob_mlc_64;
            }
            break;
        case 128:
            nand->ecc.layout = &nand_oob_mlc_128;
            break;
        case 512:
        case 640:
            nand->ecc.layout = &nand_oob_mlc_512;
            nand->ecc.mode = NAND_ECC_HW;
            nand->ecc.bytes = 26;
            nand->ecc.strength = 16;
            nand->ecc.read_page = hw_read_page_ecc16;
            nand->ecc.calculate = hw_calculate_ecc16;
            nand->ecc.correct = hw_correct_ecc16;
            nand->ecc.hwctl = hwctl_ecc16;
            writel(NFECCCONF_MSGLENGTH(nand->ecc.size - 1) |
                    NFECCCONF_MODE_16BIT, NFECCCONF);
            break;
        }

        controller->configured = true;
    }

    switch (cs) {
    case -1:
        writel(readl(NFCONT) | NFCONT_CS0 | NFCONT_CS1 | NFCONT_CS2 |
                NFCONT_CS3, NFCONT);
        break;

    case 0:
        writel((readl(NFCONT) | (NFCONT_CS1 | NFCONT_CS2 |
                NFCONT_CS3)) & ~NFCONT_CS0, NFCONT);
        break;

    case 1:
        writel((readl(NFCONT) | (NFCONT_CS0 | NFCONT_CS2 |
                NFCONT_CS3)) & ~NFCONT_CS1, NFCONT);
        break;

    case 2:
        writel((readl(NFCONT) | (NFCONT_CS0 | NFCONT_CS1 |
                NFCONT_CS3)) & ~NFCONT_CS2, NFCONT);
        break;

    case 3:
        writel((readl(NFCONT) | (NFCONT_CS0 | NFCONT_CS1 |
                NFCONT_CS2)) & ~NFCONT_CS3, NFCONT);
        break;
    }
}

static void cmd_ctrl(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{
    if (cmd == NAND_CMD_NONE) {
        return;
    }

    if (ctrl & NAND_CLE) {
        writeb(cmd, NFCMMD);
    } else {
        writeb(cmd, NFADDR);
    }
}

static int dev_ready(struct mtd_info *mtd)
{
    return readl(NFSTAT) & NFSTAT_RnB;
}

int board_nand_init(struct nand_chip *nand)
{
    /* Setup NFCONF and NFCONT */
    writel(NFCONF_TACLS(1) | NFCONF_TWRPH0(4) | NFCONF_TWRPH1(1), NFCONF);
    writel(NFCONT_CS0 | NFCONT_MODE, NFCONT);

    /* Configure pinmux for NAND */
    writel((readl(MP01CON_REG) & ~(0xF << 8) & ~(0xF << 12)) |
            (0x3 << 8) | (0x3 << 12), MP01CON_REG);
    writel((readl(MP03CON_REG) & ~(0xF << 16) & ~(0xF << 12) & ~(0xF << 8) &
            ~(0xF << 4) & ~(0xF << 0)) | (0x2 << 16) | (0x2 << 12) |
            (0x2 << 8) | (0x2 << 4) | (0x2 << 0), MP03CON_REG);

    controller_data_t *controller = calloc(sizeof(controller_data_t), 1);
    nand_set_controller_data(nand, controller);

    nand->select_chip = select_chip;
    nand->cmd_ctrl = cmd_ctrl;
    nand->dev_ready = dev_ready;
    nand->ecc.mode = NAND_ECC_SOFT;
    return 0;
}
