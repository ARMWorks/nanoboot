#include "asm/types.h"
#include "fatfs/ff.h"
#include "stdio.h"
#include "atag.h"
#include "config.h"

FATFS fs;

static void load_image(const char *name, void *load_at)
{
    FRESULT fr;
    FIL f;

    fr = f_open(&f, name, FA_READ);
    if (fr != FR_OK) {
        printf("error opening %s: %d\n", name, (int)fr);
        while (1);
    }

    unsigned int nread;
    fr = f_read(&f, load_at, (8*1024*1024), &nread); 
    if (fr != FR_OK) {
        printf("error reading /zImage: %d\n", (int)fr);
        while (1);
    }

    f_close(&f);

    printf("%s loaded, %d bytes\n", name, nread);
}    

void main(void)
{
    FRESULT fr;

    fr = f_mount(&fs, "", 1);
    if (fr != FR_OK) {
        printf("error mounting FAT: %d\n", (int)fr);
        while (1);
    }

    void *exec_at = (void *)PHYS_SDRAM_1 + 0x8000 + (32*1024*1024);
    void *parm_at = (void *)PHYS_SDRAM_1 + 0x100;

    load_image("zImage", exec_at);

    setup_atags(parm_at);
    
    void (*theKernel)(int zero, int arch, u32 params);
    theKernel = (void (*)(int, int, u32))exec_at;

    printf("Jumping to kernel...\n");

    theKernel(0, 1685, (u32)parm_at);

    while (1);
}
