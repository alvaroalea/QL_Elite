/* gfx_ell_update (contorno de la elipse fila a fila desde la tabla de semianchos) = gfx_ellipse_xor de la vieja + gfx_ellipse_xor de la nueva
   (XOR): mismas pantallas, con recorte, elipses fuera de la vista, sin vieja o sin nueva, y las que no cambian */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "platform.h"

static int fails, total;
static void check(int ok, const char *m) { total++; if (!ok) { fails++; if (fails < 10) printf("FALLO: %s\n", m); } }
static unsigned long rs = 12345;
static unsigned rnd(void) { rs = (rs * 1103515245UL + 12345UL) & 0xFFFFFFFFUL; return (unsigned)((rs >> 8) & 0xFFFF); }
static u8 a[32768], b[32768], tab1[640], tab2[640];

int main(void)
{
    int i, n;
    plat_init();
    for (i = 0; i < 3000; i++) {
        int orx = 1 + (int)(rnd() % 248), nrx = (rnd() % 3 == 0) ? orx : 1 + (int)(rnd() % 248);
        int ory = (6 * orx + 2) / 5, nry = (6 * nrx + 2) / 5;
        int ocx = (int)(rnd() % 500) - 120, ocy = (int)(rnd() % 400) - 160;
        int ncx = (rnd() % 3 == 0) ? ocx : ocx + (int)(rnd() % 9) - 4, ncy = (rnd() % 3 == 0) ? ocy : ocy + (int)(rnd() % 9) - 4;
        u8 col = (u8)(1 + rnd() % 7);
        int mode = (int)(rnd() % 8);                     /* 0: sin vieja, 1: sin nueva, resto: las dos */
        if (i % 5 == 0) { orx = 1 + (int)(rnd() % 12); ory = (6 * orx + 2) / 5; nrx = 1 + (int)(rnd() % 12); nry = (6 * nrx + 2) / 5; }
        if (i % 7 == 0) { ocx = 128; ocy = 80; ncx = 128 + (int)(rnd() % 3) - 1; ncy = 80 + (int)(rnd() % 3) - 1; }   /* el planeta centrado, moviendose poco */
        gfx_ell_spans(orx, ory, tab1); gfx_ell_spans(nrx, nry, tab2);
        memset(screen, 0, 32768);
        if (mode != 0) gfx_ellipse_xor(ocx, ocy, orx, ory, col, 0, 0, 255, 159);            /* estado inicial: la vieja dibujada */
        memcpy(b, screen, 32768);
        if (mode != 1) gfx_ellipse_xor(ncx, ncy, nrx, nry, col, 0, 0, 255, 159);            /* referencia: dibujar la nueva (y borrar la vieja: XOR) */
        if (mode != 0) gfx_ellipse_xor(ocx, ocy, orx, ory, col, 0, 0, 255, 159);
        memcpy(a, screen, 32768);                          /* a = referencia */
        memcpy(screen, b, 32768);
        gfx_ell_update(ocx, ocy, mode == 0 ? -1 : ory, tab1, ncx, ncy, mode == 1 ? -1 : nry, tab2, col, 159);
        n = memcmp(a, screen, 32768);
        check(n == 0, "gfx_ell_update = vieja + nueva en XOR");
    }
    /* la elipse sin cambios no toca nada, y borrar lo dibujado deja la pantalla vacia */
    gfx_ell_spans(60, 72, tab1);
    memset(screen, 0, 32768);
    gfx_ell_update(0, 0, -1, tab1, 128, 80, 72, tab1, 1, 159);
    memcpy(b, screen, 32768);
    gfx_ell_update(128, 80, 72, tab1, 128, 80, 72, tab1, 1, 159);
    check(memcmp(b, screen, 32768) == 0, "sin cambios no se toca la pantalla");
    gfx_ell_update(128, 80, 72, tab1, 0, 0, -1, tab1, 1, 159);
    for (i = 0; i < 32768 && screen[i] == 0; i++) ;
    check(i == 32768, "borrar deja la pantalla vacia");
    printf("%d comprobaciones, %d fallos\n", total, fails);
    return fails != 0;
}
