/* Backend QL: MT.IPCOM con el bloque ya montado por sound.c. SIN PROBAR en hardware. */
#include "sound.h"

extern long ql_ipc(char *blk);
static long ipc_buf[6];                      /* 24 bytes alineados a direccion par */

int snd_ipc(const u8 *blk, int len)
{
    char *b = (char *)ipc_buf;
    int i;
#ifdef NOSOUND
    (void)blk; (void)len; (void)b; (void)i;     /* version sin sonido: no se toca el IPC (el 8049 del ZX8302) en ningun caso */
    return 0;
#else
    for (i = 0; i < len && i < 24; i++) b[i] = (char)blk[i];
    return (ql_ipc(b) < 0) ? -1 : 0;
#endif
}
