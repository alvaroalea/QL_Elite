/* QL backend: MT.IPCOM with the block already assembled by sound.c. UNTESTED on hardware. */
#include "sound.h"

extern long ql_ipc(char *blk);
static long ipc_buf[6];                      /* 24 bytes aligned to an even address */

int snd_ipc(const u8 *blk, int len)
{
    char *b = (char *)ipc_buf;
    int i;
#ifdef NOSOUND
    (void)blk; (void)len; (void)b; (void)i;     /* silent version: the IPC (the ZX8302's 8049) is never touched */
    return 0;
#else
    for (i = 0; i < len && i < 24; i++) b[i] = (char)blk[i];
    return (ql_ipc(b) < 0) ? -1 : 0;
#endif
}
