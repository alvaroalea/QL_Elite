/* Drawing of planets and suns against the ORIGINAL 6502 (vectors from tools/gen_planet.py): sequences of PLANET frames
   (PROJ, DVID3B2, SUN, CHKON, EDGES, HLOIN2, WPLS, CIRCLE, CIRCLE2, BLINE, LL145, WPLS2, WP1) with the planet or sun
   crossing the view edges; all the lines are compared (LOIN already sorted and SWAP; HLOIN rounded) and the heaps. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sun.h"
#include "ships.h"
#include "ship3d.h"
#include "state.h"
#include "emath.h"
#include "game.h"

static int fails, total;
static char cur[200000], got[200000];
static void check(int ok, const char *what)
{
    total++;
    if (!ok) { if (fails++ < 8 || getenv("ALLF")) printf("FALLO %s: %.300s\n   C: %.300s\n", what, cur, got); }
}
static int hexb(const char *s, u8 *out, int n)
{
    int i; unsigned v;
    for (i = 0; i < n; i++) { if (sscanf(s + 2 * i, "%2x", &v) != 1) return 0; out[i] = (u8)v; }
    return 1;
}
static void app(const char *b) { size_t l = strlen(got); if (l + strlen(b) + 2 < sizeof got) { if (l) strcat(got, " "); strcat(got, b); } }
static void lh(void) { char b[40]; sprintf(b, "L:%d,%d,%d,%d,%d", zx15[0], zx15[1], zx15[2], zx15[3], zswap); app(b); }
static void hh(u8 l, u8 r, u8 y) { char b[40]; sprintf(b, "H:%d,%d,%d", l, r, y); app(b); }

int main(void)
{
    FILE *f = fopen("tests/vectors_planet.txt", "r");
    int ntr = 0, nfr = 0, nl = 0;
    if (!f) { printf("falta tests/vectors_planet.txt (make test_planet con ELITE_SRC)\n"); return 1; }
    plat_init();
    state_clear();
    sun_loin_hook = lh; sun_hloin_hook = hh;
    while (fgets(cur, sizeof cur, f)) {
        char *nlp = strchr(cur, '\n'); if (nlp) *nlp = 0;
        if (cur[0] == 'T') {
            int typ, nf; char rh[16]; u8 r[4];
            sscanf(cur + 2, "%d %d %15s", &typ, &nf, rh); hexb(rh, r, 4);
            ztype = (u8)typ; memcpy(em_rand, r, 4);
            zk3[0] = zk3[1] = 0;
            sun_forget();
            memset(circ_lsx2, 0, 256); memset(circ_lsy2, 0, 256);
            circ_lsx2[0] = 255; circ_lsy2[0] = 255; circ_lsp = 1;
            memset(sun_lso, 0, 256); sun_lsx = 255; sun_set_sunx(0, 0);
            ntr++;
        } else if (cur[0] == 'F') {
            char ih[32], *p; int sw, c; u8 iw[9];
            sscanf(cur + 2, "%31s %d %d", ih, &sw, &c); hexb(ih, iw, 9);
            p = cur + 2; p = strchr(p, ' ') + 1; p = strchr(p, ' ') + 1; p = strchr(p, ' ') + 1;
            memset(inwk, 0, NI); memcpy(inwk, iw, 9);
            zswap = (u8)sw; em_rand_c = (u8)c;
            got[0] = 0;
            planet();
            if (!got[0]) strcpy(got, "-");
            check(strcmp(p, got) == 0, "lineas de PLANET");
            { char *q = p; while ((q = strchr(q, ':')) != 0) { nl++; q++; } }
            nfr++;
        } else if (cur[0] == 'U') {
            int stp, sw; char *p = cur + 2;
            sscanf(p, "%d %d", &stp, &sw); p = strchr(p, ' ') + 1; p = strchr(p, ' ') + 1;
            memset(circ_lsx2, 0, 256); memset(circ_lsy2, 0, 256); circ_lsx2[0] = circ_lsy2[0] = 255; circ_lsp = 1;
            zswap = (u8)sw; got[0] = 0;
            tunnel_draw((u8)stp);
            if (!got[0]) strcpy(got, "-");
            check(strcmp(p, got) == 0, "tunel (HFS1)");
        } else if (cur[0] == 'R') {
            int q0, q1, k, sw, lsp; char *p = cur + 2; int i;
            sscanf(p, "%d %d %d %d %d", &q0, &q1, &k, &sw, &lsp);
            for (i = 0; i < 5; i++) p = strchr(p, ' ') + 1;
            memset(circ_lsx2, 0, 256); memset(circ_lsy2, 0, 256); circ_lsx2[0] = circ_lsy2[0] = 255;
            zk3[0] = (u8)q0; zk4[0] = (u8)q1; zk3[1] = zk4[1] = 0; zK[0] = (u8)k;      /* TT128 */
            circ_lsp = 1; circ_stp = 2; zswap = (u8)sw; got[0] = 0;
            circle2_draw();
            if (!got[0]) strcpy(got, "-");
            check(strcmp(p, got) == 0 && circ_lsp == lsp, "circulo de las cartas (TT128)");
        } else if (cur[0] == 'C') {
            unsigned k3, k4; int k, c;
            sscanf(cur + 2, "%u %u %d %d", &k3, &k4, &k, &c);
            zk3[0] = (u8)k3; zk3[1] = (u8)(k3 >> 8); zk4[0] = (u8)k4; zk4[1] = (u8)(k4 >> 8); zK[0] = (u8)k;
            sprintf(got, "%d", sun_chkon());
            check(sun_chkon() == c, "CHKON");
        } else if (cur[0] == 'E') {
            int lsp, lsx, ok, i; char a[600], b[600], o[500], sx[8], rh[16]; u8 x2[256], y2[256], lso[200], sxv[2], r[4], mine[2];
            sscanf(cur + 2, "%d %599s %599s %499s %d %7s %15s", &lsp, a, b, o, &lsx, sx, rh);
            hexb(a, x2, 256); hexb(b, y2, 256); hexb(o, lso, 200); hexb(sx, sxv, 2); hexb(rh, r, 4);
            sun_get_sunx(mine);
            ok = circ_lsp == lsp && sun_lsx == lsx && memcmp(sun_lso, lso, 200) == 0 && memcmp(em_rand, r, 4) == 0 &&
                 circ_lsx2[0] == x2[0];
            for (i = 1; i < lsp; i++) ok = ok && circ_lsx2[i] == x2[i] && circ_lsy2[i] == y2[i];
            if (!(sun_lsx & 0x80)) ok = ok && memcmp(mine, sxv, 2) == 0;
            sprintf(got, "lsp %d lsx %d rnd %02x%02x%02x%02x", circ_lsp, sun_lsx, em_rand[0], em_rand[1], em_rand[2], em_rand[3]);
            check(ok, "montones al final de la secuencia");
        }
    }
    fclose(f);
    printf("6502: %d secuencias, %d fotogramas, %d lineas\n", ntr, nfr, nl);
    printf("%d comprobaciones, %d fallos\n", total, fails);
    return fails != 0;
}
