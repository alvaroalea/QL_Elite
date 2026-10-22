/* Volcado para tools/test_ellrowsasm.py: casos de gfx_ell_update (version en C) con la pantalla de partida a $FF; para cada caso imprime los
   datos, las tablas y los bytes de pantalla que cambian ("indice:valor"). uso: ellrows_dump n semilla */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "platform.h"

static unsigned long rs;
static unsigned rnd(void) { rs = (rs * 1103515245UL + 12345UL) & 0xFFFFFFFFUL; return (unsigned)((rs >> 8) & 0xFFFF); }
static u8 tab1[640], tab2[640];

int main(int argc, char **argv)
{
    int n = argc > 1 ? atoi(argv[1]) : 100, i, k;
    rs = argc > 2 ? (unsigned long)atol(argv[2]) : 1UL;
    plat_init();
    for (k = 0; k < n; k++) {
        int orx = 1 + (int)(rnd() % 248), nrx = (rnd() % 3 == 0) ? orx : 1 + (int)(rnd() % 248);
        int ocx = (int)(rnd() % 500) - 120, ocy = (int)(rnd() % 400) - 160, ncx, ncy, ory, nry, mode = (int)(rnd() % 8), cly1 = (rnd() & 1) ? 159 : 135;
        u8 col = (u8)(1 + rnd() % 7);
        if (k % 4 == 0) { orx = 1 + (int)(rnd() % 14); nrx = 1 + (int)(rnd() % 14); }
        ncx = (rnd() % 3 == 0) ? ocx : ocx + (int)(rnd() % 9) - 4; ncy = (rnd() % 3 == 0) ? ocy : ocy + (int)(rnd() % 9) - 4;
        if (k % 3 == 0) { ocx = 128 + (int)(rnd() % 80) - 40; ocy = 80 + (int)(rnd() % 40) - 20; ncx = ocx + (int)(rnd() % 3) - 1; ncy = ocy + (int)(rnd() % 3) - 1; }
        ory = (6 * orx + 2) / 5; nry = (6 * nrx + 2) / 5;
        gfx_ell_spans(orx, ory, tab1); gfx_ell_spans(nrx, nry, tab2);
        memset(screen, 0xFF, 32768);
        gfx_ell_update(ocx, ocy, mode == 0 ? -1 : ory, tab1, ncx, ncy, mode == 1 ? -1 : nry, tab2, col, cly1);
        printf("CASE %d %d %d %d %d %d %d %d %d", ocx, ocy, mode == 0 ? -1 : ory, ncx, ncy, mode == 1 ? -1 : nry, col, cly1, k);
        printf(" otab="); for (i = 0; i < 2 * (ory + 1); i++) printf("%02x", tab1[i]);
        printf(" ntab="); for (i = 0; i < 2 * (nry + 1); i++) printf("%02x", tab2[i]);
        printf(" out=");
        for (i = 0; i < 32768; i++) if (screen[i] != 0xFF) printf(" %d:%d", i, screen[i]);
        printf("\n");
    }
    /* desplazamientos de 1 y 2 puntos del mismo planeta (centro igual o no): los casos del juego al volar */
    {
        static const int rxs[6] = { 7, 30, 60, 100, 150, 248 };
        static const int dxs[6] = { 0, 0, 0, 1, 2, -1 }, dys[6] = { 1, -1, 2, 0, 1, -2 };
        int r, m, ry;
        for (r = 0; r < 6; r++) for (m = 0; m < 6; m++) {
            ry = (6 * rxs[r] + 2) / 5;
            gfx_ell_spans(rxs[r], ry, tab1);
            memset(screen, 0xFF, 32768);
            gfx_ell_update(128, 80, ry, tab1, 128 + dxs[m], 80 + dys[m], ry, tab1, 1 + (u8)m, 159);
            printf("CASE 128 80 %d %d %d %d %d 159 %d", ry, 128 + dxs[m], 80 + dys[m], ry, 1 + m, 1000 + r * 10 + m);
            printf(" otab="); for (i = 0; i < 2 * (ry + 1); i++) printf("%02x", tab1[i]);
            printf(" ntab="); for (i = 0; i < 2 * (ry + 1); i++) printf("%02x", tab1[i]);
            printf(" out=");
            for (i = 0; i < 32768; i++) if (screen[i] != 0xFF) printf(" %d:%d", i, screen[i]);
            printf("\n");
        }
    }
    return 0;
}
