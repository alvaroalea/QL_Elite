/* LOIN (loin_draw), HLOIN (gfx_hloin), VLOIN (gfx_vloin) y PIXEL (gfx_dust) contra el codigo original en el emulador (tools/gen_lines.py):
   mismos trazos sobre una pantalla vacia y los mismos puntos encendidos, en coordenadas del juego. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "platform.h"
#include "lines.h"

u8 *screen;
static u8 buf[32768];
static char line[200000];

int main(int argc, char **argv)
{
    FILE *f = fopen(argc > 1 ? argv[1] : "tests/vectors_lines.txt", "r");
    long cases[4] = { 0, 0, 0, 0 }, fails[4] = { 0, 0, 0, 0 };
    const char *nm = "LHVP";
    screen = buf;
    if (!f) { printf("sin vectores\n"); return 2; }
    while (fgets(line, sizeof line, f)) {
        char *p = line, *sep = strchr(line, ':');
        int k = (int)(strchr(nm, line[0]) - nm), x, y, bad = 0, a[4];
        long want = 0, got = 0;
        if (!sep) continue;
        if (line[0] == 'G' || line[0] == 'R') {                /* puntos APAGADOS tras cleargrap / clearrow 15 y 16 */
            static u8 offp[256][256];
            unsigned v; int used, bad2 = 0;
            memset(buf, 0, sizeof buf);
            for (y = 0; y < 192; y++) for (x = 0; x < 256; x++) gfx_pixel(x, y, C_WHITE);
            if (line[0] == 'G') gfx_fill_lines(0, 136, C_BLACK); else gfx_fill_lines(120, 16, C_BLACK);
            memset(offp, 0, sizeof offp);
            p = sep + 1;
            while (sscanf(p, " %4x%n", &v, &used) == 1) { offp[v >> 8][v & 255] = 1; p += used; }
            for (y = 0; y < 192; y++) for (x = 0; x < 256; x++) if ((gfx_get(x, y) == 0) != offp[x][y]) bad2 = 1;
            printf("  %s  %s\n", line[0] == 'G' ? "cleargrap" : "clearrow ", bad2 ? "FALLO" : "0 fallos");
            if (bad2) fails[0]++;
            continue;
        }
        memset(buf, 0, sizeof buf);
        p++;
        *sep = 0;
        while (1) {
            int m, used = 0;
            if (k == 0) m = sscanf(p, "%d %d %d %d%n", &a[0], &a[1], &a[2], &a[3], &used);
            else m = sscanf(p, "%d %d %d%n", &a[0], &a[1], &a[2], &used);
            if (m < (k == 0 ? 4 : 3)) break;
            p += used;
            if (k == 0) loin_draw_6502((u8)a[0], (u8)a[1], (u8)a[2], (u8)a[3], C_WHITE);
            else if (k == 1) gfx_hloin(a[0], a[1], a[2], C_WHITE);
            else if (k == 2) gfx_vloin(a[0], a[1], a[2], C_WHITE);
            else gfx_dust_6502(a[0], a[1], (u8)a[2]);
        }
        p = sep + 1;
        {   /* los puntos esperados estan ordenados por x y luego y; se comparan con la pantalla */
            static u8 exp[256][256];
            unsigned v;
            int used;
            memset(exp, 0, sizeof exp);
            while (sscanf(p, " %4x%n", &v, &used) == 1) { exp[v >> 8][v & 255] = 1; want++; p += used; }
            for (y = 0; y < SCR_H; y++)
                for (x = 0; x < SCR_W; x++) {
                    int on = gfx_get(x, y) != 0;
                    got += on;
                    if (on != exp[x][y]) {
                        if (!bad && fails[k] < 6) printf("FALLO %c %s: punto (%d, %d) %s\n", line[0], line + 2, x, y, on ? "de mas" : "de menos");
                        bad = 1;
                    }
                }
        }
        cases[k]++;
        if (bad) fails[k]++;
    }
    { int x; for (x = 0; x < 4; x++) printf("  %s  %ld casos, %ld fallos\n", x == 0 ? "LOIN " : x == 1 ? "HLOIN" : x == 2 ? "VLOIN" : "PIXEL", cases[x], fails[x]); }
    return (fails[0] || fails[1] || fails[2] || fails[3]) ? 1 : 0;
}
