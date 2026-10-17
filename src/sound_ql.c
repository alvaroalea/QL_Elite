/* Backend QL: MT.IPCOM con el bloque ya montado por sound.c. SIN PROBAR en hardware. */
#include "sound.h"

extern long ql_ipc(char *blk);
static long ipc_buf[6];                      /* 24 bytes alineados a direccion par */

int snd_ipc(const u8 *blk, int len)
{
    char *b = (char *)ipc_buf;
    int i;
    for (i = 0; i < len && i < 24; i++) b[i] = (char)blk[i];
    return (ql_ipc(b) < 0) ? -1 : 0;
}
