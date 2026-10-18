/* TT15, SIGHT, NLIN2/NLIN4, BOMBEFF2 y DOEXP contra el codigo original en el emulador (tools/gen_draw.py): las mismas llamadas
   a las primitivas de dibujo (HLOIN, VLOIN, LOIN, CPIX) con los mismos argumentos y color, y el mismo estado a la salida. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "platform.h"
#include "chart.h"
#include "sun.h"
#include "cmdr.h"
#include "lines.h"
#include "equip.h"
#include "combat.h"
#include "ships.h"
#include "ship3d.h"
#include "state.h"
#include "emath.h"
#include "sound.h"
#include "game.h"
#include "space.h"
#include "sun.h"

static char line[100000], got[100000];
static int glen;

static void put(const char *s) { size_t n = strlen(s); if (glen + n + 2 < sizeof got) { if (glen) got[glen++] = ' '; memcpy(got + glen, s, n); glen += (int)n; got[glen] = 0; } }
static int colv(u8 c) { return c == C_BLUE ? 16 : c == C_RED ? 20 : c == C_GREEN ? 8 : c == C_WHITE ? 12 : 99; }
static void rec_chart(u8 kind, u8 a, u8 b, u8 c) { char s[40]; sprintf(s, "%c %d %d %d %d", kind ? 'v' : 'h', a, b, c, colv(sun_col)); put(s); }
static void rec_hloin(u8 x1, u8 x2, u8 y, u8 c) { char s[40]; sprintf(s, "h %d %d %d %d", x1, x2, y, colv(c)); put(s); }
static void rec_loin(u8 x1, u8 y1, u8 x2, u8 y2) { char s[40]; sprintf(s, "%d %d %d %d", x1, y1, x2, y2); put(s); }
static void rec_hiss(int y) { char s[16]; sprintf(s, "h %d", y); put(s); snd_sohiss(y); }
static void rec_blip(int x, int y) { char s[24]; sprintf(s, "b %d %d", x, y); put(s); }
static void rec_circ(u8 k3, u8 k4, u8 k, u8 stp) { char s[32]; sprintf(s, "c %d %d %d %d", k3, k4, k, stp); put(s); }
static void rec_cpix(u8 x, u8 y, u8 c) { char s[40]; sprintf(s, "%d %d %d", x, y, c); put(s); }

static int hexb(const char *p, u8 *dst, int max)
{
    int n = 0; unsigned v;
    while (n < max && sscanf(p + 2 * n, "%2x", &v) == 1 && p[2 * n] != ' ') { dst[n] = (u8)v; n++; }
    return n;
}
static void hexs(char *s, const u8 *b, int n) { int i; for (i = 0; i < n; i++) sprintf(s + 2 * i, "%02x", b[i]); }
static char *trim(char *s) { char *e; while (*s == ' ') s++; e = s + strlen(s); while (e > s && (e[-1] == ' ' || e[-1] == '\n' || e[-1] == '\r')) *--e = 0; return s; }

int main(int argc, char **argv)
{
    static u8 heapbuf[64];
    FILE *f = fopen(argc > 1 ? argv[1] : "tests/vectors_draw.txt", "r");
    const char *kinds = "TSNBEKU";
    long cases[7] = { 0 }, fails[7] = { 0 };
    int k;
    if (!f) { printf("sin vectores\n"); return 2; }
    plat_init();
    game_new();
    ships_hooks();
    chart_line_hook = rec_chart; lines_hloin_hook = rec_hloin; bomb_line_hook = rec_loin; ships_cpix_hook = rec_cpix;
    while (fgets(line, sizeof line, f)) {
        char *c1 = strchr(line, ':'), *want, *p, buf[400];
        int a[8], bad = 0;
        if (!c1) continue;
        *c1 = 0; want = trim(c1 + 1);
        k = (int)(strchr(kinds, line[0]) - kinds);
        glen = 0; got[0] = 0; p = line + 2;
        if (k == 0) {                                          /* T local x y size */
            sscanf(p, "%d %d %d %d", &a[0], &a[1], &a[2], &a[3]);
            sun_col = C_GREEN;
            chart_tt15(a[0], (u8)a[1], (u8)a[2], (u8)a[3]);
        } else if (k == 1) {                                   /* S view l0..l3 */
            int i;
            sscanf(p, "%d %d %d %d %d", &a[0], &a[1], &a[2], &a[3], &a[4]);
            for (i = 0; i < 4; i++) laser[i] = (u8)a[1 + i];
            chart_sight((u8)a[0]);
        } else if (k == 2) {                                   /* N 2 a / N 4 0 */
            sscanf(p, "%d %d", &a[0], &a[1]);
            if (a[0] == 2) nlin2((u8)a[1]); else nlin4();
        } else if (k == 3) {                                   /* B rnd qq11 dnoiz c tbx tby : rnd tbx tby : lineas */
            char *c2 = strchr(want, ':'), s1[80];
            int q11, dn, cc;
            u8 r[4], b[10];
            *c2 = 0;
            hexb(p, r, 4); p += 9;
            sscanf(p, "%d %d %d", &q11, &dn, &cc);
            p = strchr(p, ' ') + 1; p = strchr(p, ' ') + 1; p = strchr(p, ' ') + 1;
            hexb(p, bombtbx, 10); hexb(p + 21, bombtby, 10);
            memcpy(em_rand, r, 4);
            combat_view_on = (u8)(q11 == 0);
            snd_enable(dn == 0);
            em_rand_c = (u8)cc;
            bomb_eff2();
            hexs(s1, em_rand, 4); s1[8] = ' '; hexs(s1 + 9, bombtbx, 10); s1[29] = ' '; hexs(s1 + 30, bombtby, 10);
            if (strcmp(trim(want), s1)) { bad = 1; if (fails[k] < 5) printf("FALLO B estado: %s\n  esperado %s\n", s1, trim(want)); }
            (void)b;
            want = trim(c2 + 1);
        } else if (k == 6) {                                   /* U LAUN|LL164 rnd dnoiz c : rnd : sonidos y circulos */
            char ent[8], rs[12], s1[12], *c2 = strchr(want, ':');
            int dn, cc;
            *c2 = 0;
            sscanf(p, "%7s %11s %d %d", ent, rs, &dn, &cc);
            hexb(rs, em_rand, 4);
            snd_enable(dn == 0);
            em_rand_c = (u8)cc;
            sun_hiss_hook = rec_hiss; sun_blip_hook = rec_blip; sun_circle_hook = rec_circ; sun_tt66_hook = 0;
            if (!strcmp(ent, "LAUN")) tunnel_laun(); else tunnel_ll164();
            hexs(s1, em_rand, 4);
            if (strcmp(trim(want), s1)) { bad = 1; if (fails[k] < 5) printf("FALLO U RAND: %s esperado %s\n", s1, trim(want)); }
            want = trim(c2 + 1);
        } else if (k == 5) {                                   /* K : scacol */
            int i; char s[8];
            for (i = 0; i < 34; i++) { sprintf(s, "%d", space_scacol((u8)i)); put(s); }
        } else {                                               /* E inwk heap xx3 k6 rnd : inwk31 heap rnd : cpix */
            char *c2 = strchr(want, ':'), s1[300];
            int nw, k6;
            *c2 = 0;
            nw = hexb(p, inwk, 64); p += 2 * nw + 1;
            hexb(p, heapbuf, 64); p += 129;
            hexb(p, xx3, 64); p += 129;
            sscanf(p, "%d", &k6); p = strchr(p, ' ') + 1;
            hexb(p, em_rand, 4);
            state_slot(0)[6] = (u8)k6;
            zxx19 = heapbuf;
            doexp();
            sprintf(s1, "%02x ", inwk[31]); hexs(s1 + 3, heapbuf, 64); s1[131] = ' '; hexs(s1 + 132, em_rand, 4);
            if (strcmp(trim(want), s1)) { bad = 1; if (fails[k] < 5) printf("FALLO E estado: %s\n  esperado %s\n", s1, trim(want)); }
            want = trim(c2 + 1);
        }
        if (strcmp(got, want)) {
            bad = 1;
            if (fails[k] < 5) { strncpy(buf, line, 300); buf[300] = 0; printf("FALLO %s\n  port:     %.400s\n  original: %.400s\n", buf, got, want); }
        }
        cases[k]++;
        if (bad) fails[k]++;
    }
    for (k = 0; k < 7; k++) printf("  %c  %ld casos, %ld fallos\n", kinds[k], cases[k], fails[k]);
    for (k = 0; k < 7; k++) if (fails[k]) return 1;
    return 0;
}
