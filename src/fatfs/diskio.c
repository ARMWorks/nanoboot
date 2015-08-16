#include "asm/types.h"
#include "fatfs/diskio.h"
#include "movi.h"
#include "stdio.h"
#include "string.h"

DSTATUS disk_status(BYTE pdrv)
{
    if (pdrv != 0) {
        return STA_NOINIT | STA_NODISK | STA_PROTECT;
    }

    return STA_PROTECT;
}

DSTATUS disk_initialize(BYTE pdrv)
{
    if (pdrv != 0) {
        return STA_NOINIT | STA_NODISK | STA_PROTECT;
    }

    return STA_PROTECT;
}

DRESULT disk_read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count)
{
    if (pdrv != 0) {
        return RES_PARERR;
    }

    int readcount = count;
    if (count & 1) {
        readcount += 1;
    }

    if ((u32)buff & 3 || readcount > count) {
        u32 tmp[readcount * 128];
        if (!CopyMovitoMem(sector, readcount, tmp, 0)) {
            printf("CopyMovitoMem error\n");
            return RES_ERROR;
        }
        memcpy(buff, tmp, count * 512);
    } else {
        if (!CopyMovitoMem(sector, readcount, (u32 *)buff, 0)) {
            return RES_ERROR;
        }
    }

    return RES_OK;
}
