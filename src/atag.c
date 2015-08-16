#include "atag.h"
#include "config.h"
#include "string.h"

static struct atag *params; /* used to point at the current tag */

static void
setup_core_atag(void *address, long pagesize)
{
    params = (struct atag *)address;

    params->hdr.tag = ATAG_CORE;
    params->hdr.size = atag_size(atag_core);

    params->u.core.flags = 1;
    params->u.core.pagesize = pagesize;
    params->u.core.rootdev = 0;

    params = atag_next(params);
}

static void
setup_mem_atag(u32 start, u32 len)
{
    params->hdr.tag = ATAG_MEM;
    params->hdr.size = atag_size(atag_mem);

    params->u.mem.start = start;
    params->u.mem.size = len;

    params = atag_next(params);
}

static void
setup_cmdline_atag(const char *line)
{
    int linelen = strlen(line);
    if (!linelen)
        return;

    params->hdr.tag = ATAG_CMDLINE;
    params->hdr.size = (sizeof(struct atag_header) + linelen + 1 + 4) >> 2;

    strcpy(params->u.cmdline.cmdline, line);

    params = atag_next(params);
}

static void
setup_end_atag(void)
{
    params->hdr.tag = ATAG_NONE;
    params->hdr.size = 0;
}

void
setup_atags(void *parameters)
{
    setup_core_atag(parameters, 4096);
    setup_mem_atag(PHYS_SDRAM_1, PHYS_SDRAM_1_SIZE);
    setup_cmdline_atag("root=/dev/mmcblk0p2 rootfstype=ext4 init=/sbin/init console=ttySAC0,115200");
    setup_end_atag();
}
