/* PLUT (move.c) y, en su momento, ESCAPE, MJP y TT110 contra el codigo original en el emulador (tools/gen_flight.py). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "platform.h"
#include "move.h"
#include "state.h"
#include "flight.h"
#include "stars.h"
#include "shipdata.h"
#include "space.h"
#include "tactics.h"
#include "ships.h"
#include "dash.h"
#include "combat.h"
#include "emath.h"

static char line[4000];
static void hexs(char *d, const u8 *b, int n) { int i; for (i = 0; i < n; i++) sprintf(d + 2 * i, "%02x", b[i]); }
static void rdh(const char *s, u8 *b, int n) { int i; char t[3]; t[2] = 0; for (i = 0; i < n; i++) { t[0] = s[2 * i]; t[1] = s[2 * i + 1]; b[i] = (u8)strtol(t, 0, 16); } }

int main(int argc, char **argv)
{
    FILE *f = fopen(argc > 1 ? argv[1] : "tests/vectors_flight.txt", "r");
    long cases[8] = { 0 }, fails[8] = { 0 };
    if (!f) { printf("sin vectores\n"); return 2; }
    plat_init();
    while (fgets(line, sizeof line, f)) {
        char *c1 = strchr(line, ':');
        if (!c1) continue;
        *c1 = 0;
        if (line[0] == 'U') {
            int v;
            char ib[100], buf[100], want[100];
            sscanf(line + 2, "%d %99s", &v, ib);
            rdh(ib, inwk, NI);
            plut((u8)v);
            hexs(buf, inwk, NI);
            sscanf(c1 + 1, "%99s", want);
            cases[0]++;
            if (strcmp(buf, want)) { if (fails[0] < 4) printf("FALLO U vista %d\n  port: %s\n  orig: %s\n", v, buf, want); fails[0]++; }
        } else if (line[0] == 'L') {
            int tek, fist0, c0, wf, wd, wq, dodo, bad = 0, i;
            (void)wq; (void)dodo;
            char cg[16], rs[16], wr[16], *p, *q;
            u8 b[NI];
            sscanf(line + 2, "%d %d %15s %15s %d", &tek, &fist0, cg, rs, &c0);
            p = c1 + 1;
            sscanf(p, "%d %d %d %15s %d", &wf, &wd, &wq, wr, &dodo);
            state_clear(); ships_clear();
            qq5 = (u8)tek; fist = (u8)fist0; qq12 = 0xFF; rdh(cg, b, 3); qq20[3] = b[0]; qq20[6] = b[1]; qq20[10] = b[2];
            rdh(rs, em_rand, 4); em_rand_c = (u8)c0; zDELTA = 77;
            space_launch();
            { char hs[16]; sprintf(hs, "%02x%02x%02x%02x", em_rand[0], em_rand[1], em_rand[2], em_rand[3]); if (strcmp(hs, wr)) bad = 1; }
            if (fist != wf || zDELTA != wd) bad = 1;
            q = strchr(p, ':'); q++;
            for (i = 0; i < 2; i++) {                          /* planeta y estacion: tipo e INWK (salvo bytes 31, 33, 34, 35 y, del planeta, 36) */
                int t = atoi(q), k;
                u8 w[NI];
                char *colon = strchr(q, ':');
                rdh(colon + 1, w, NI);
                if (i == 0 && frin[0] != t) bad = 1;
                if (i == 1 && frin[1] != t) bad = 1;                 /* tipo 2 (Coriolis) aunque el plano sea el del Dodo */
                for (k = 0; k < NI; k++) {
                    if (k == 31 || k == 33 || k == 34 || k == 35) continue;
                    if (i == 0 && k == 36) continue;
                    if (state_slot(i)[k] != w[k]) bad = 1;
                }
                q = strchr(colon + 1, ' ');
                if (!q) break;
                q++;
            }
            cases[2]++;
            if (bad) { if (fails[2] < 4) printf("FALLO L tek=%d\n", tek); fails[2]++; }
        } else if (line[0] == 'J') {
            int q1, mc, ec, sp, wmj, wnostm, wq1, wthg, wecma, bad = 0;
            sscanf(line + 2, "%d %d %d %d", &q1, &mc, &ec, &sp);
            { char *p = strchr(c1 + 1, ':'); sscanf(p + 1, "%d %d %d %d %d", &wmj, &wnostm, &wq1, &wthg, &wecma); }
            ships_clear();
            qq1 = (u8)q1; many[SHIPT_THARGOID] = (u8)mc; ecma = (u8)ec; sspr = (u8)sp; mj = 0; stars_nostm = 12; space_view = 3;
            misjump();
            if (mj != wmj || stars_nostm != wnostm || qq1 != wq1 || many[SHIPT_THARGOID] != wthg || ecma != wecma || space_view != 0) bad = 1;
            cases[1]++;
            if (bad) { if (fails[1] < 4) printf("FALLO J: mj=%d nostm=%d qq1=%d thg=%d ecma=%d vista=%d (original %d %d %d %d %d)\n", mj, stars_nostm, qq1, many[SHIPT_THARGOID], ecma, space_view, wmj, wnostm, wq1, wthg, wecma); fails[1]++; }
        }
    }
    printf("  U  %ld casos, %ld fallos\n  J  %ld casos, %ld fallos\n  L  %ld casos, %ld fallos\n", cases[0], fails[0], cases[1], fails[1], cases[2], fails[2]);
    return (fails[0] || fails[1] || fails[2]) ? 1 : 0;
}
