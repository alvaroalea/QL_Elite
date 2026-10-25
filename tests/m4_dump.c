/* Volcado de referencia en MODO 4 para tools/test_mode4asm.py: las versiones en C (sin ASM_LINE) de las primitivas, con gfx_m4 = 1.
     L x1 y1 x2 y2 c      linea a los 512 puntos (loin_bres_c)
     E cx cy rx ry c clx0 cly0 clx1 cly1    elipse a los 512 puntos      F cx cy hw hh c clx0 cly0 clx1 cly1    rectangulo
     H x0 x1 y c   V x y0 y1 c   P x y c   C col row ncols nrows (pantalla llena de $FF antes)    puntos del juego (2 puntos fisicos de ancho)
     R ocx ocy orx om ncx ncy nrx nm c cly1   cambio de elipse fila a fila (om / nm = 0: no hay vieja / nueva); las tablas salen en palabras
   Por cada caso escribe los bytes de pantalla no nulos ("offset:valor ..."); R antes lleva ory, nry y las tablas ("o=hex n=hex"). */
#include <stdio.h>
#include <string.h>
#include "platform.h"
#include "lines.h"
u8 *screen;
static u8 buf[32768];
static unsigned short t1[700], t2[700];
int main(int argc, char **argv)
{
    FILE *f = fopen(argv[1], "r");
    char t[4];
    int v[10], j;
    screen = buf;
    gfx_set_mode(1);
    while (fscanf(f, "%3s", t) == 1) {
        memset(buf, 0, sizeof buf);
        if (t[0] == 'L') {
            for (j = 0; j < 5; j++) fscanf(f, "%d", &v[j]);
            loin_bres_c((u8)v[0], (u8)v[1], (u8)v[2], (u8)v[3], (u8)v[4]);
        } else if (t[0] == 'E') {
            for (j = 0; j < 9; j++) fscanf(f, "%d", &v[j]);
            gfx_ellipse_xor(v[0], v[1], v[2], v[3], (u8)v[4], v[5], v[6], v[7], v[8]);
        } else if (t[0] == 'F') {
            for (j = 0; j < 9; j++) fscanf(f, "%d", &v[j]);
            gfx_frame_xor(v[0], v[1], v[2], v[3], (u8)v[4], v[5], v[6], v[7], v[8]);
        } else if (t[0] == 'C') {
            for (j = 0; j < 4; j++) fscanf(f, "%d", &v[j]);
            memset(buf, 0xFF, sizeof buf);
            gfx_clear_cells(v[0], v[1], v[2], v[3]);
            for (j = 0; j < 32768; j++) if (buf[j] != 0xFF) printf("%d:%d ", j, buf[j]);
            printf("\n");
            continue;
        } else if (t[0] == 'P') {
            for (j = 0; j < 3; j++) fscanf(f, "%d", &v[j]);
            gfx_xor_pixel(v[0], v[1], (u8)v[2]);
        } else if (t[0] == 'V') {
            for (j = 0; j < 4; j++) fscanf(f, "%d", &v[j]);
            gfx_vline_xor(v[0], v[1], v[2], (u8)v[3]);
        } else if (t[0] == 'R') {
            int ocx, ocy, orx, om, ncx, ncy, nrx, nm, c, cly1, ory, nry;
            fscanf(f, "%d %d %d %d %d %d %d %d %d %d", &ocx, &ocy, &orx, &om, &ncx, &ncy, &nrx, &nm, &c, &cly1);
            ory = (6 * orx + 2) / 5; nry = (6 * nrx + 2) / 5;
            gfx_ell_spans(orx, ory, (u8 *)t1); gfx_ell_spans(nrx, nry, (u8 *)t2);
            memset(buf, 0xFF, sizeof buf);
            gfx_ell_update(ocx, ocy, om ? ory : -1, (u8 *)t1, ncx, ncy, nm ? nry : -1, (u8 *)t2, (u8)c, cly1);
            printf("%d %d o=", ory, nry);
            for (j = 0; j < 2 * (ory + 1); j++) printf("%04x", t1[j]);
            printf(" n=");
            for (j = 0; j < 2 * (nry + 1); j++) printf("%04x", t2[j]);
            printf(" out=");
            for (j = 0; j < 32768; j++) if (buf[j] != 0xFF) printf(" %d:%d", j, buf[j]);
            printf("\n");
            continue;
        } else {
            for (j = 0; j < 4; j++) fscanf(f, "%d", &v[j]);
            gfx_hline_xor(v[0], v[1], v[2], (u8)v[3]);
        }
        for (j = 0; j < 32768; j++) if (buf[j]) printf("%d:%d ", j, buf[j]);
        printf("\n");
    }
    return 0;
}
