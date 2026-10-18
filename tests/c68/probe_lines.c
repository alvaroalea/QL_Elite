/* Sonda de c68: LOIN (loin_draw, con em_logdiv), HLOIN y VLOIN; la pantalla final se compara con la del PC */
#include "platform.h"
#include "lines.h"
#include "emath.h"
int main(void)
{
    unsigned long r = 12345;
    int i;
    plat_init();
    gfx_clear(C_BLACK);
    for (i = 0; i < 3000; i++) {
        u8 a, b, c, d;
        r = (r * 1103515245UL + 12345UL) & 0xFFFFFFFFUL; a = (u8)(r >> 16);
        r = (r * 1103515245UL + 12345UL) & 0xFFFFFFFFUL; b = (u8)((r >> 16) % 192);
        r = (r * 1103515245UL + 12345UL) & 0xFFFFFFFFUL; c = (u8)(r >> 16);
        r = (r * 1103515245UL + 12345UL) & 0xFFFFFFFFUL; d = (u8)((r >> 16) % 192);
        if (i % 7 == 0) gfx_hloin(a, c, b, (u8)(i & 7));
        else if (i % 11 == 0) gfx_vloin(a, b, d, (u8)(i & 7));
        else loin_draw(a, b, c, d, (u8)((i % 7) + 1));
    }
    plat_present();
    return 0;
}
