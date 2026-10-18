/* Cartas (TT22, TT23, TT16, hm, con TT14, TT15, TT103, TT105, TT128) contra el codigo original en el emulador
   (tools/gen_charts.py): la misma secuencia de HLOIN, VLOIN, PIXEL, CIRCLE2, SUN y cpl con sus argumentos, y QQ9/QQ10. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "platform.h"
#include "chart.h"
#include "sun.h"
#include "lines.h"
#include "universe.h"
#include "game.h"
#include "state.h"

static char line[200000], got[200000];
static int glen;
static void put(const char *s) { size_t n = strlen(s); if (glen + n + 2 < sizeof got) { if (glen) got[glen++] = ' '; memcpy(got + glen, s, n); glen += (int)n; got[glen] = 0; } }
static int colv(u8 c) { return c == C_BLUE ? 16 : c == C_RED ? 20 : c == C_GREEN ? 8 : c == C_WHITE ? 12 : 99; }
static void rec_line(u8 k, u8 a, u8 b, u8 c) { char s[40]; sprintf(s, "%c %d %d %d %d", k ? 'v' : 'h', a, b, c, colv(sun_col)); put(s); }
static void rec_hloin(u8 x1, u8 x2, u8 y, u8 c) { char s[40]; sprintf(s, "h %d %d %d %d", x1, x2, y, colv(c)); put(s); }
static void rec_ev(char k, u8 a, u8 b, u8 c)
{
    char s[40];
    if (k == 'n') sprintf(s, "n %d %d %d %d", a, b, uni_galaxy[c].x, uni_galaxy[c].y);
    else sprintf(s, "%c %d %d %d", k, a, b, c);
    put(s);
}
static char *trim(char *s) { char *e; while (*s == ' ') s++; e = s + strlen(s); while (e > s && (e[-1] == ' ' || e[-1] == '\n')) *--e = 0; return s; }

int main(int argc, char **argv)
{
    FILE *f = fopen(argc > 1 ? argv[1] : "tests/vectors_charts.txt", "r");
    const char *kinds = "GLMHF";
    long cases[5] = { 0 }, fails[5] = { 0 };
    int k, lastg = -1;
    if (!f) { printf("sin vectores\n"); return 2; }
    plat_init();
    chart_line_hook = rec_line; lines_hloin_hook = rec_hloin; chart_event_hook = rec_ev;
    while (fgets(line, sizeof line, f)) {
        char *c1 = strchr(line, ':'), *want, *fin = 0, buf[64];
        int v[8], bad = 0;
        if (!c1) continue;
        *c1 = 0; want = c1 + 1;
        k = (int)(strchr(kinds, line[0]) - kinds);
        if (k >= 2 && k < 4) { char *c2 = strchr(want, ':'); *c2 = 0; fin = trim(c2 + 1); }
        want = trim(want);
        if (k == 4) {                                          /* F g nombre : indice o -1 */
            char nm[32] = "";
            int g, r;
            sscanf(line + 2, "%d %31s", &g, nm);
            if (g != lastg) { uni_build_galaxy(g + 1); lastg = g; }
            r = chart_find_system(nm);
            cases[k]++;
            if (r != atoi(want)) { if (fails[k] < 4) printf("FALLO F %s %s: port %d, original %s\n", line + 2, nm, r, want); fails[k]++; }
            continue;
        }
        if (k < 2) sscanf(line + 2, "%d %d %d %d %d", &v[0], &v[1], &v[2], &v[3], &v[4]);
        else if (k == 2) sscanf(line + 2, "%d %d %d %d %d %d %d", &v[7], &v[0], &v[1], &v[2], &v[3], &v[5], &v[6]);
        else sscanf(line + 2, "%d %d %d %d %d", &v[7], &v[0], &v[1], &v[2], &v[3]);
        if (v[0] != lastg) { uni_build_galaxy(v[0] + 1); lastg = v[0]; }
        cur_sys = v[1]; qq0 = uni_galaxy[v[1]].x; qq1 = uni_galaxy[v[1]].y; qq9 = (u8)v[2]; qq10 = (u8)v[3];
        if (k < 2) qq14 = (u8)v[4];
        glen = 0; got[0] = 0;
        gfx_clear(C_BLACK);
        if (k == 0) chart_draw(0);
        else if (k == 1) chart_draw(1);
        else if (k == 2) chart_tt16(v[7], (u8)v[5], (u8)v[6]);
        else chart_hm(v[7]);
        if (strcmp(got, want)) bad = 1;
        if (fin) { sprintf(buf, "%d %d", qq9, qq10); if (strcmp(buf, fin)) bad = 1; }
        cases[k]++;
        if (bad) {
            if (fails[k] < 4) {
                int d = 0;
                while (got[d] && got[d] == want[d]) d++;
                d = d > 40 ? d - 40 : 0;
                printf("FALLO %s\n  port:     ...%.300s %s\n  original: ...%.300s %s\n", line, got + d, fin ? buf : "", want + d, fin ? fin : "");
            }
            fails[k]++;
        }
    }
    for (k = 0; k < 5; k++) printf("  %c  %ld casos, %ld fallos\n", kinds[k], cases[k], fails[k]);
    for (k = 0; k < 5; k++) if (fails[k]) return 1;
    return 0;
}
