/* Game Bresenham line (loin_bres_c): endpoints, symmetry (A->B = B->A), exact erasing in XOR, continuity and closeness to the ideal
   line; and "dump" mode to compare ql_line.asm (tools/test_lineasm.py).
     test_bres                 -> tests
     test_bres dump cases.txt  -> for each case "x1 y1 x2 y2 c" writes the non-zero screen bytes ("offset:value ...") */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "platform.h"
#include "lines.h"

u8 *screen;
static u8 buf[32768], buf2[32768];
static int fails, total;
static void check(int ok, const char *m) { total++; if (!ok) { fails++; printf("FALLO: %s\n", m); } }

static int lit(int x, int y) { return gfx_get(x, y) != 0; }

int main(int argc, char **argv)
{
    long i, k;
    screen = buf;
    if (argc > 2 && !strcmp(argv[1], "dump")) {
        FILE *f = fopen(argv[2], "r");
        int a, b, c, d, col;
        long j;
        while (fscanf(f, "%d %d %d %d %d", &a, &b, &c, &d, &col) == 5) {
            memset(buf, 0, sizeof buf);
            loin_bres_c((u8)a, (u8)b, (u8)c, (u8)d, (u8)col);
            for (j = 0; j < 32768; j++) if (buf[j]) printf("%ld:%d ", j, buf[j]);
            printf("\n");
        }
        fclose(f);
        return 0;
    }
    srand(12345);
    for (i = 0; i < 20000; i++) {
        int x1 = rand() & 255, y1 = rand() & 255, x2 = rand() & 255, y2 = rand() & 255, n = 0, x, y, dx, dy, ok;
        if (i % 4 == 0) y2 = y1;                                  /* horizontals */
        if (i % 4 == 1) x2 = x1;                                  /* verticals */
        if (i % 4 == 2) { y2 = y1 + (x2 - x1); if (y2 < 0 || y2 > 255) y2 = y1; }     /* diagonals */
        memset(buf, 0, sizeof buf);
        loin_bres_c((u8)x1, (u8)y1, (u8)x2, (u8)y2, C_WHITE);
        check(lit(x1, y1) && lit(x2, y2), "los dos extremos estan encendidos");
        for (y = 0; y < 256; y++) for (x = 0; x < 256; x++) if (lit(x, y)) n++;
        dx = abs(x2 - x1); dy = abs(y2 - y1);
        check(n == (dx > dy ? dx : dy) + 1, "un punto por cada x (o y)");
        memcpy(buf2, buf, sizeof buf);
        memset(buf, 0, sizeof buf);
        loin_bres_c((u8)x2, (u8)y2, (u8)x1, (u8)y1, C_WHITE);
        check(memcmp(buf, buf2, sizeof buf) == 0, "A->B y B->A encienden lo mismo");
        loin_bres_c((u8)x1, (u8)y1, (u8)x2, (u8)y2, C_WHITE);       /* XOR: drawing again erases, even in the other direction */
        for (k = 0; k < 32768 && buf[k] == 0; k++) ;
        check(k == 32768, "dibujar dos veces borra");
        /* closeness to the ideal line: one dot for each column (or row) and less than 1 dot from the line */
        ok = 1;
        if (dx >= dy && dx > 0) {
            int lo = x1 < x2 ? x1 : x2, hi = x1 < x2 ? x2 : x1;
            loin_bres_c((u8)x1, (u8)y1, (u8)x2, (u8)y2, C_WHITE);
            for (x = lo; x <= hi && ok; x++) {
                double ideal = y1 + (double)(y2 - y1) * (x - x1) / (x2 - x1);
                int cnt = 0;
                for (y = 0; y < 256; y++) if (lit(x, y)) { cnt++; if (fabs(y - ideal) > 0.75) ok = 0; }
                if (cnt != 1) ok = 0;
            }
        } else if (dy > 0) {
            int lo = y1 < y2 ? y1 : y2, hi = y1 < y2 ? y2 : y1;
            loin_bres_c((u8)x1, (u8)y1, (u8)x2, (u8)y2, C_WHITE);
            for (y = lo; y <= hi && ok; y++) {
                double ideal = x1 + (double)(x2 - x1) * (y - y1) / (y2 - y1);
                int cnt = 0;
                for (x = 0; x < 256; x++) if (lit(x, y)) { cnt++; if (fabs(x - ideal) > 0.75) ok = 0; }
                if (cnt != 1) ok = 0;
            }
        }
        check(ok, "cerca de la recta ideal, un punto por columna o fila");
    }
    printf("%d comprobaciones, %d fallos\n", total, fails);
    return fails != 0;
}
