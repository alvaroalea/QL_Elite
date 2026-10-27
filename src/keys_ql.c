/* QL backend: reads a keyboard row with IPC command 9. UNTESTED on hardware.
   8-byte block (format verified with QL community examples): 9,1,0,0,0,0,<row>,2 ; reply in D1.B */
#include "keys.h"

extern long ql_ipc(char *blk);
static long ipc_blk[2];                    /* long to align to an even address */

int ql_read_row(int row)
{
    char *b = (char *)ipc_blk;
    long r;
    b[0] = 9; b[1] = 1; b[2] = 0; b[3] = 0; b[4] = 0; b[5] = 0; b[6] = (char)row; b[7] = 2;
    r = ql_ipc(b);
    return (r < 0) ? 0 : (int)(r & 0xFF);
}
