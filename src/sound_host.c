/* Backend de pruebas: guarda los bloques IPC enviados. */
#include "sound.h"
u8 host_ipc_log[16][24];
int host_ipc_len[16];
int host_ipc_n;
int snd_ipc(const u8 *blk, int len)
{
    int i;
    if (host_ipc_n < 16) {
        for (i = 0; i < len && i < 24; i++) host_ipc_log[host_ipc_n][i] = blk[i];
        host_ipc_len[host_ipc_n] = len;
    }
    host_ipc_n++;
    return 0;
}
