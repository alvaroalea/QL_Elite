/* Volcado de referencia para tools/test_ellasm.py: gfx_ellipse_xor y gfx_hline_xor en C (sin ASM_LINE).
     E cx cy rx ry c clx0 cly0 clx1 cly1      H x0 x1 y c      V x y0 y1 c      P x y c      C col row ncols nrows (pantalla llena de $FF antes)      F cx cy hw hh c clx0 cly0 clx1 cly1
   Por cada caso escribe los bytes de pantalla no nulos ("offset:valor ..."). */
#include <stdio.h>
#include <string.h>
#include "platform.h"
u8 *screen;
static u8 buf[32768];
int main(int argc, char **argv)
{
    FILE *f = fopen(argv[1], "r");
    char t[4];
    int v[9], j;
    screen = buf;
    while (fscanf(f, "%3s", t) == 1) {
        memset(buf, 0, sizeof buf);
        if (t[0] == 'E') {
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
        } else {
            for (j = 0; j < 4; j++) fscanf(f, "%d", &v[j]);
            gfx_hline_xor(v[0], v[1], v[2], (u8)v[3]);
        }
        for (j = 0; j < 32768; j++) if (buf[j]) printf("%d:%d ", j, buf[j]);
        printf("\n");
    }
    return 0;
}
