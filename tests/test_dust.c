/* Stardust (gfx_dust): 1 dot far (zz >= 128), 1 x 2 between 80 and 127 and 2 x 3 (width x height) near (zz < 80), with the lower
   left corner at (x, y); erase = draw again; clipping at the edges; and the original PIXEL intact (gfx_dust_6502). */
#include <stdio.h>
#include <string.h>
#include "platform.h"
u8 *screen;
static u8 buf[32768];
static int fails, total;
static void check(int ok, const char *m) { total++; if (!ok) { fails++; printf("FALLO: %s\n", m); } }
static int count(void) { int x, y, n = 0; for (y = 0; y < 256; y++) for (x = 0; x < 256; x++) if (gfx_get(x, y)) n++; return n; }
int main(void)
{
    int zz, k;
    screen = buf;
    for (zz = 0; zz < 256; zz++) {
        int w = zz >= 128 ? 1 : (zz >= 80 ? 1 : 2), h = zz >= 128 ? 1 : (zz >= 80 ? 2 : 3), x, y, ok = 1;
        memset(buf, 0, sizeof buf);
        gfx_dust(100, 100, (u8)zz);
        check(count() == w * h, "numero de puntos segun la distancia");
        for (y = 0; y < 256; y++) for (x = 0; x < 256; x++) {
            int want = x >= 100 && x < 100 + w && y > 100 - h && y <= 100;
            if ((gfx_get(x, y) != 0) != want) ok = 0;
        }
        check(ok, "forma: ancho w, alto h, esquina inferior izquierda en (x, y)");
        gfx_dust(100, 100, (u8)zz);
        for (k = 0; k < 32768 && buf[k] == 0; k++) ;
        check(k == 32768, "dibujar dos veces borra");
    }
    memset(buf, 0, sizeof buf);                                    /* edges: does not write outside nor hang */
    gfx_dust(0, 0, 10); gfx_dust(255, 255, 10); gfx_dust(-3, 5, 10); gfx_dust(300, 5, 10); gfx_dust(10, -5, 10); gfx_dust(10, 300, 10);
    check(count() >= 1, "en los bordes pinta lo que cabe");
    memset(buf, 0, sizeof buf);
    gfx_dust_6502(100, 100, 255); check(count() == 2, "PIXEL original: zz >= 128 = 2 puntos");
    memset(buf, 0, sizeof buf);
    gfx_dust_6502(100, 100, 100); check(count() == 3, "PIXEL original: 80..127 = 3 puntos");
    memset(buf, 0, sizeof buf);
    gfx_dust_6502(100, 100, 10); check(count() == 6, "PIXEL original: < 80 = 3 x 2 puntos");
    printf("%d comprobaciones, %d fallos\n", total, fails);
    return fails != 0;
}
