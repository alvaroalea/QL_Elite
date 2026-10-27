#include "shipview.h"
#include "state.h"
static u8 heap[256];
int main(void)
{
    int t, k, i, n = 0;
    unsigned short h;
    long j;
    plat_init();
    for (t = 1; t <= 32; t++) {
        for (k = 0; k < 6; k++) {
            int ay = (k * 11 + t * 5) & 63, ax = (k * 7 + t * 3) & 63;
            int z = 5 + (k * 3 + t) % 20;
            state_clear();
            gfx_clear(C_BLACK);
            shipview_draw((u8)t, (k % 3) - 1, (k % 2) - 1, z, ay, ax, heap, state_slot(1), 1);
            h = 0;
            for (j = 0; j < 32768L; j++) h = (unsigned short)(((h << 5) | (h >> 11)) ^ screen[j]);
            screen[40000 - 40000 + 0] = screen[0];
            n++;
            /* stores the hash in a separate area: we use a static array */
            {
                static unsigned short hs[200];
                hs[t * 6 + k - 6] = h;
                if (t == 32 && k == 5) {
                    gfx_clear(C_BLACK);
                    for (i = 0; i < 192; i++) { screen[2 * i] = (u8)hs[i]; screen[2 * i + 1] = (u8)(hs[i] >> 8); }
                }
            }
        }
    }
    plat_present();
    return 0;
}
