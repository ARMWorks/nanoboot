#ifndef __MOVI_H__
#define __MOVI_H__

#include "asm/types.h"

#define MOVI_INIT_REQUIRED	0

#define	TCM_BASE		0x40004000

/* TCM function for bl2 load */
/* int CopyMovitoMem(u32 a, u16 b, u32 *c, u32 d);
 * @param u32 a  - source block address
 * @param u16 b  - number of blocks to copy -- needs to be a multiple of 2
 * @param u32* c - buffer to copy to
 * @param bool d - perform card initialization
 * @return bool  - success or failure
 */
#define CopyMovitoMem(a,b,c,d)		(((int(*)(u32, u16, u32 *, u32))(*((u32 *)(TCM_BASE + 0x8))))(a,b,c,d))

/* size information */
#define SS_SIZE			(8 * 1024)
#define eFUSE_SIZE		(1 * 1024)	// 0.5k eFuse, 0.5k reserved`

/* movinand definitions */
#define MOVI_BLKSIZE		512
#define MOVI_RW_MAXBLKS		40960
#define MOVI_TOTAL_BLKCNT	*((volatile unsigned int*)(TCM_BASE - 0x4))
#define MOVI_HIGH_CAPACITY	*((volatile unsigned int*)(TCM_BASE - 0x8))

/* partition information */
#define PART_SIZE_BL		(      256 * 1024)

#define MOVI_LAST_BLKPOS	(MOVI_TOTAL_BLKCNT - (eFUSE_SIZE / MOVI_BLKSIZE))
#define MOVI_BL1_BLKCNT		(SS_SIZE / MOVI_BLKSIZE)
#define MOVI_BL2_BLKCNT		(PART_SIZE_BL / MOVI_BLKSIZE)
#define MOVI_BL2_POS		(MOVI_LAST_BLKPOS - MOVI_BL1_BLKCNT - MOVI_BL2_BLKCNT)

#endif /*__MOVI_H__*/
