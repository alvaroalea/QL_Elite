/* Compares the port's universe with the original 6502 (tests/vectors_univ.txt, from tools/gen_universe.py):
   S/G  seeds of the 8 galaxies and the 256 systems of each (TT24, TT20/TT54, cpl, radius of TT25)
   N    TT111 (nearest system and distance QQ8) and jmp
   V    GVL (QQ26 and availability) and TT151 (prices)
   W    sequences of NWSHP and KILLSHP (with KS2); Z  ZINF */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "universe.h"
#include "state.h"
#include "emath.h"
#include "ships.h"
#include "shipdata.h"
#include "game.h"
#include "tactics.h"
#include "chart.h"
extern u16 orig_slsp;
#include "font.h"

static int total, fails;
static char line[20000];
static int built = -1;

static void check(int ok, const char *what, const char *l)
{
    total++;
    if (!ok) { fails++; if (fails <= 10) printf("FALLO %s: %.240s\n", what, l); }
}
static void hexb(const char *h, u8 *b, int n) { int i; unsigned v; for (i = 0; i < n; i++) { sscanf(h + 2 * i, "%2x", &v); b[i] = (u8)v; } }
static int tok_n(char **a, char *s) { int n = 0; char *p = strtok(s, " \n"); while (p) { a[n++] = p; p = strtok(0, " \n"); } return n; }
static void galaxy(int g) { if (built != g) { uni_build_galaxy(g + 1); built = g; } }

static void g_case(char **a, const char *l)
{
    int g = atoi(a[1]), i = atoi(a[2]);
    u8 q[6];
    const uni_system *p;
    unsigned rad;
    galaxy(g);
    p = &uni_galaxy[i];
    hexb(a[3], q, 6);
    rad = (unsigned)(((q[5] & 15) + 11) * 256 + q[3]);
    check(p->seed0 == (u16)(q[0] | (q[1] << 8)) && p->goatsoupseed.a == q[2] && p->goatsoupseed.b == q[3] &&
          p->goatsoupseed.c == q[4] && p->goatsoupseed.d == q[5], "semillas del sistema", l);
    check(p->economy == atoi(a[4]) && p->govtype == atoi(a[5]) && p->techlev == atoi(a[6]) && p->population == atoi(a[7]) &&
          p->productivity == (u16)(atoi(a[8]) | (atoi(a[9]) << 8)), "TT24", l);
    check(p->radius == rad && p->x == q[3] && p->y == q[1], "radio y coordenadas", l);
    check(!strcmp(p->name, a[10]), "nombre (cpl)", l);
}

static void s_case(char **a, const char *l)
{
    u8 q[6], o[6];
    hexb(a[2], q, 6);
    uni_galaxy_seed(atoi(a[1]) + 1, o);
    check(!memcmp(q, o, 6), "semilla de la galaxia (Ghy)", l);
}

static int twins;
static void n_case(char **a, const char *l)
{
    /* N g x y x0 y0 | zz q15 qq9 qq10 qq8 qq3 qq0 qq1 zz2 (TT111 again from (qq9, qq10)) */
    int g = atoi(a[1]), idx, zz = atoi(a[7]);
    uni_system cur;
    const uni_system *p;
    galaxy(g);
    idx = uni_nearest((u8)atoi(a[2]), (u8)atoi(a[3]));
    p = &uni_galaxy[idx];
    memset(&cur, 0, sizeof cur); cur.x = (u8)atoi(a[4]); cur.y = (u8)atoi(a[5]);
    check(idx == zz && p->x == atoi(a[9]) && p->y == atoi(a[10]), "TT111: sistema mas cercano", l);
    check(uni_distance(&cur, p) == (unsigned)atoi(a[11]) && p->economy == atoi(a[12]), "TT111: distancia QQ8", l);
    check(p->x == atoi(a[13]) && p->y == atoi(a[14]), "jmp", l);
    check(uni_nearest(p->x, p->y) == atoi(a[15]), "TT111 desde el elegido (gemelo de menor indice)", l);
    if (uni_nearest(p->x, p->y) != idx) twins++;
}


static void h_case(char **a, const char *l)
{
    /* H g x y x0 y0 ev0 | qq0 qq1 qq2 ev qq28 tek gov: hyp1+3 (TT18) after the selection with TT111 and TTX111's safehouse */
    int g = atoi(a[1]), idx;
    const uni_system *p;
    u8 q2[6], c2[6];
    galaxy(g);
    idx = uni_nearest((u8)atoi(a[2]), (u8)atoi(a[3]));
    game_enter_system(idx, 1);                           /* jmp + QQ28, tek and gov of the new system */
    p = &uni_galaxy[idx];
    c2[0] = (u8)(p->seed0 & 255); c2[1] = (u8)(p->seed0 >> 8);
    c2[2] = p->goatsoupseed.a; c2[3] = p->goatsoupseed.b; c2[4] = p->goatsoupseed.c; c2[5] = p->goatsoupseed.d;
    hexb(a[10], q2, 6);
    check(qq0 == atoi(a[8]) && qq1 == atoi(a[9]), "hyp1: QQ0 y QQ1", l);
    check(!memcmp(q2, c2, 6), "hyp1: QQ2 = semillas del sistema (safehouse)", l);
    check(atoi(a[11]) == 0, "hyp1: EV = 0 (flight.c: enc_ev = 0 al llegar)", l);
    check(qq3 == atoi(a[12]) && qq5 == atoi(a[13]) && qq4 == atoi(a[14]), "hyp1: QQ28, tek y gov", l);
}

static void p_case(char **a, const char *l)
{
    /* P x0 y0 x y | qq9 qq10 */
    qq0 = (u8)atoi(a[1]); qq1 = (u8)atoi(a[2]); qq9 = (u8)atoi(a[3]); qq10 = (u8)atoi(a[4]);
    chart_ping_set();
    check(qq9 == atoi(a[6]) && qq10 == atoi(a[7]), "ping", l);
}

static void v_case(char **a, const char *l)
{
    /* V rnd c0 eco | q26 avl rnd prices */
    u8 avl[17], r2[4];
    uni_system sys;
    uni_market m;
    int i, ok = 1, q26;
    char *p;
    hexb(a[1], em_rand, 4); em_rand_c = (u8)atoi(a[2]);
    q26 = em_dornd();
    hexb(a[6], avl, 17); hexb(a[7], r2, 4);
    memset(&sys, 0, sizeof sys); sys.economy = (u8)atoi(a[3]);
    uni_gen_market((unsigned)q26, &sys, &m);
    check(q26 == atoi(a[5]) && !memcmp(r2, em_rand, 4), "GVL: QQ26", l);
    for (i = 0; i < 17; i++) if (m.quantity[i] != avl[i]) ok = 0;
    check(ok, "GVL: disponibilidad", l);
    ok = 1; p = a[8];
    for (i = 0; i < 17; i++) { if ((int)m.price[i] != atoi(p)) ok = 0; p = strchr(p, ','); if (!p) break; p++; }
    check(ok, "TT151: precios", l);
}

static void w_case(char **a, int na, const char *l)
{
    /* W ops... | frin many junk tp tally1 mstg slots */
    int k, bar, i, ok = 1;
    char exp[2000], got[2000], *s;
    state_clear(); ships_clear(); mstg = 0xFF; tp = 0; tally[1] = 0;
    for (bar = 1; bar < na && strcmp(a[bar], "|"); bar++) {
        s = a[bar];
        if (s[0] == 'n') {
            int t = atoi(s + 1), c;
            char *h = strchr(s, ':') + 1;
            hexb(h, inwk, NI);
            c = atoi(strrchr(s, ':') + 1);
            k = ship_new((u8)t);
            if ((k >= 0) != c) ok = 0;
        } else {
            mstg = (u8)atoi(strchr(s, ':') + 1);
            ship_kill(atoi(s + 1));
        }
    }
    exp[0] = 0;
    for (i = bar + 1; i < na; i++) { strcat(exp, a[i]); if (i + 1 < na) strcat(exp, " "); }
    got[0] = 0;
    for (i = 0; i <= NOSH; i++) sprintf(got + strlen(got), "%02x", frin[i]);
    strcat(got, " ");
    for (i = 0; i < 33; i++) sprintf(got + strlen(got), "%02x", many[i]);
    sprintf(got + strlen(got), " %d %d %d %d ", junk_count, tp, tally[1], mstg);
    for (k = 0; k < NOSH; k++) {
        for (i = 0; i < NI; i++) sprintf(got + strlen(got), "%02x", !frin[k] ? 0 : state_slot(k)[i]);
        if (k + 1 < NOSH) strcat(got, ",");
    }
    sprintf(got + strlen(got), " %u", (unsigned)orig_slsp);
    check(ok && !strcmp(exp, got), "NWSHP/KILLSHP", l);
    if (fails && fails <= 8 && strcmp(exp, got)) printf("  6502: %s\n     C: %s\n", exp, got);
}

static void f_case(char **a, const char *l)
{
    /* F c bytes: the original's FONT (bit 0 = leftmost dot) versus font8x8 (bit 7 = left) */
    u8 b[8];
    int c = atoi(a[1]), i, j, ok = 1;
    hexb(a[2], b, 8);
    for (i = 0; i < 8; i++) for (j = 0; j < 7; j++) if (((b[i] >> j) & 1) != ((font8x8[c - 32][i] >> (7 - j)) & 1)) ok = 0;
    check(ok, "letter: fuente", l);
}

static void z_case(char **a, const char *l)
{
    u8 e[NI];
    hexb(a[1], inwk, NI);
    ship_zinf();
    hexb(a[3], e, NI);
    check(!memcmp(e, inwk, NI), "ZINF", l);
}

int main(void)
{
    FILE *f = fopen("tests/vectors_univ.txt", "r");
    char *a[64], copy[300];
    int cnt[128], na;
    memset(cnt, 0, sizeof cnt);
    if (!f) { printf("falta tests/vectors_univ.txt (make test_univ6502)\n"); return 1; }
    while (fgets(line, sizeof line, f)) {
        memcpy(copy, line, sizeof copy - 1); copy[sizeof copy - 1] = 0;
        na = tok_n(a, line);
        if (na < 2) continue;
        cnt[(int)a[0][0]]++;
        switch (a[0][0]) {
        case 'S': s_case(a, copy); break;
        case 'G': g_case(a, copy); break;
        case 'N': n_case(a, copy); break;
        case 'V': v_case(a, copy); break;
        case 'H': h_case(a, copy); break;
        case 'P': p_case(a, copy); break;
        case 'W': w_case(a, na, copy); break;
        case 'Z': z_case(a, copy); break;
        case 'F': f_case(a, copy); break;
        }
    }
    fclose(f);
    printf("TT111 desde el elegido: %d de %d casos pasan al gemelo de menor indice (igual que el original)\n", twins, cnt['N']);
    printf("S %d, G %d, N %d, H %d, P %d, V %d, W %d, Z %d, F %d casos; %d comprobaciones, %d fallos\n",
           cnt['S'], cnt['G'], cnt['N'], cnt['H'], cnt['P'], cnt['V'], cnt['W'], cnt['Z'], cnt['F'], total, fails);
    return fails != 0;
}
