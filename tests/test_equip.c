/* Equipo contra el 6502 ORIGINAL (vectores de tools/gen_equip.py): bomba (BOMBON/BOMBOFF), WARP, DOCKIT, DOKEY con el
   ordenador de atraque, GTHG y FRS1. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "equip.h"
#include "tactics.h"
#include "dockit.h"
#include "combat.h"
#include "state.h"
#include "emath.h"
#include "ship3d.h"
#include "ships.h"
#include "space.h"
#include "shipdata.h"
#include "keys.h"

static int fails, total;
static char cur[20000], got[4000];
static void check(int ok, const char *what)
{
    total++;
    if (!ok) { if (fails++ < 10 || getenv("ALLF")) printf("FALLO %s: %.400s\n   C: %.400s\n", what, cur, got); }
}
static int hexb(const char *s, u8 *out, int n)
{
    int i; unsigned v;
    for (i = 0; i < n; i++) { if (sscanf(s + 2 * i, "%2x", &v) != 1) return 0; out[i] = (u8)v; }
    return 1;
}
static void bl(u8 a, u8 b, u8 c, u8 d) { char t[32]; sprintf(t, "%s%d,%d,%d,%d", got[0] ? " " : "", a, b, c, d); strcat(got, t); }
static char evb[400]; static int evn;
static void ev_msbar(u8 x, u8 col) { char b[24]; sprintf(b, "%sB%d.%d", evn++ ? ";" : "", x, col); strcat(evb, b); }
static void ev_mess(u8 t) { char b[24]; sprintf(b, "%sM%d", evn++ ? ";" : "", t); strcat(evb, b); }
static int same_but(const u8 *a, const u8 *b) { int i; for (i = 0; i < NI; i++) if (i != 31 && i != 33 && i != 34 && i != 35 && i != 36 && a[i] != b[i]) return 0; return 1; }

int main(void)
{
    FILE *f = fopen("tests/vectors_equip.txt", "r");
    static char a[24][800];
    int cnt[7] = { 0, 0, 0, 0, 0, 0, 0 };
    if (!f) { printf("falta tests/vectors_equip.txt (make test_equip con ELITE_SRC)\n"); return 1; }
    plat_init();
    state_clear();
    bomb_line_hook = bl;
    while (fgets(cur, sizeof cur, f)) {
        int na = 0; char *pp = cur, *e;
        if ((e = strchr(cur, '\n')) != 0) *e = 0;
        while (na < 24 && sscanf(pp, "%799s", a[na]) == 1) { pp = strstr(pp, a[na]) + strlen(a[na]); na++; }
        if (na == 0) continue;
        got[0] = 0;
        switch (a[0][0]) {
        case 'B': {
            u8 r[4], tx[10], ty[10], r2[4]; char *lines;
            hexb(a[1], r, 4); hexb(a[3], tx, 10); hexb(a[4], ty, 10); hexb(a[5], r2, 4);
            lines = strstr(cur, a[5]) + strlen(a[5]) + 1;
            memcpy(em_rand, r, 4); combat_view_on = 1;
            bomb_on((u8)atoi(a[2]));
            check(memcmp(bombtbx, tx, 10) == 0 && memcmp(bombtby, ty, 10) == 0 && memcmp(em_rand, r2, 4) == 0 &&
                  strcmp(got, lines) == 0, "BOMBON/BOMBOFF");
            cnt[0]++;
            break; }
        case 'W': {
            u8 pl[9], su[9]; int junk = atoi(a[1]), did, i;
            hexb(a[5], pl, 9); hexb(a[6], su, 9);
            for (i = 0; i <= NOSH; i++) frin[i] = 0;
            frin[2 + junk] = (u8)atoi(a[2]); junk_count = (u8)junk; sspr = (u8)atoi(a[3]); mj = (u8)atoi(a[4]);
            memcpy(state_slot(0), pl, 9); memcpy(state_slot(1), su, 9);
            did = warp();
            sprintf(got, "%d %d %d", ks[8], ks[NI + 8], did);
            check(ks[8] == atoi(a[7]) && ks[NI + 8] == atoi(a[8]) && did == atoi(a[9]) && (!did || (atoi(a[10]) == 1 && atoi(a[11]) == 1)), "WARP");
            mj = 0; sspr = 0; junk_count = 0;
            cnt[1]++;
            break; }
        case 'D': {
            u8 iw[NI], st[NI], pl[9], ex[NI]; int i;
            hexb(a[1], iw, NI); hexb(a[2], st, NI); hexb(a[3], pl, 9); hexb(a[7], ex, NI);
            memcpy(inwk, iw, NI); memcpy(state_slot(1), st, NI); memcpy(state_slot(0), pl, 9);
            ztype = (u8)atoi(a[4]); sspr = (u8)atoi(a[5]); zk3[10] = (u8)atoi(a[6]);
            dockit();
            for (i = 0; i < NI; i++) sprintf(got + 2 * i, "%02x", inwk[i]);
            sprintf(got + 2 * NI, " %d %d %d", t_rat, t_rat2, t_cnt);
            check(memcmp(inwk, ex, NI) == 0 && t_rat == atoi(a[8]) && t_rat2 == atoi(a[9]) && t_cnt == atoi(a[10]), "DOCKIT");
            cnt[2]++;
            break; }
        case 'K': {
            u8 st[NI], pl[9], k0[6], k1[6]; struct controls c; int ok;
            hexb(a[1], st, NI); hexb(a[2], pl, 9); hexb(a[7], k0, 6); hexb(a[11], k1, 6);
            memcpy(state_slot(1), st, NI); memcpy(state_slot(0), pl, 9); sspr = 1; dock_auto = 0xFF;
            zDELTA = (u8)atoi(a[3]); jstx = (u8)atoi(a[4]); jsty = (u8)atoi(a[5]); zk3[10] = (u8)atoi(a[6]);
            memset(&c, 0, sizeof c);
            c.slow_down = (u8)(k0[0] != 0); c.speed_up = (u8)(k0[1] != 0); c.roll_left = (u8)(k0[2] != 0);
            c.roll_right = (u8)(k0[3] != 0); c.pitch_up = (u8)(k0[4] != 0); c.pitch_down = (u8)(k0[5] != 0);
            dokey_auto(&c);
            sprintf(got, "D %d JX %d JY %d keys %d%d%d%d%d%d", zDELTA, jstx, jsty, c.slow_down, c.speed_up, c.roll_left, c.roll_right, c.pitch_up, c.pitch_down);
            ok = zDELTA == atoi(a[8]) && jstx == atoi(a[9]) && jsty == atoi(a[10]);
            ok = ok && c.slow_down == (k1[0] != 0) && c.speed_up == (k1[1] != 0) && c.roll_left == (k1[2] != 0) &&
                 c.roll_right == (k1[3] != 0) && c.pitch_up == (k1[4] != 0) && c.pitch_down == (k1[5] != 0);
            check(ok, "DOKEY (ordenador de atraque)");
            dock_auto = 0; sspr = 0;
            cnt[3]++;
            break; }
        case 'G': {
            u8 r[4], i0[NI], i1[NI], r2[4];
            hexb(a[1], r, 4); hexb(a[4], i0, NI); hexb(a[6], i1, NI); hexb(a[7], r2, 4);
            ships_clear(); frin[0] = frin[1] = frin[2] = 128;
            memcpy(em_rand, r, 4); em_rand_c = (u8)atoi(a[2]);
            gthg();
            check(frin[3] == atoi(a[3]) && frin[4] == atoi(a[5]) && same_but(state_slot(3), i0) && same_but(state_slot(4), i1) &&
                  memcmp(em_rand, r2, 4) == 0, "GTHG");
            cnt[4]++;
            break; }
        case 'F': {
            u8 i0[NI];
            hexb(a[5], i0, NI);
            ships_clear(); frin[0] = frin[1] = frin[2] = 128;
            mstg = (u8)atoi(a[2]); zDELTA = (u8)atoi(a[3]);
            frs1((u8)atoi(a[1]));
            check(frin[3] == atoi(a[4]) && same_but(state_slot(3), i0), "FRS1");
            mstg = 0xFF;
            cnt[5]++;
            break; }
        case 'R': {            /* R ok nomsl delta mstg type msar fr0 k0 | ev nomsl fr slots */
            u8 fr0[9], k0[9 * NI], fr1[9], sl1[9 * NI];
            int i, ok_, k, nf = 0;
            char *ex = a[10];
            hexb(a[7], fr0, 9); hexb(a[8], k0, 9 * NI); hexb(a[12], fr1, 9); for (i = 0; i < 9; i++) hexb(a[13] + i * (2 * NI + 1), sl1 + i * NI, NI);
            ships_clear();
            for (i = 0; i < 9; i++) { frin[i] = fr0[i]; memcpy(state_slot(i), k0 + i * NI, NI); }
            if (!atoi(a[1])) for (i = 3; i < NOSH; i++) if (!frin[i]) frin[i] = 7;       /* sin ranura libre: FRS1 falla */
            nomsl = (u8)atoi(a[2]); zDELTA = (u8)atoi(a[3]); mstg = (u8)atoi(a[4]); ztype = (u8)atoi(a[5]); msar = (u8)atoi(a[6]);
            evn = 0; evb[0] = 0;
            tac_msbar_hook = ev_msbar; combat_mess_hook = ev_mess;
            frmis();
            tac_msbar_hook = 0; combat_mess_hook = 0;
            (void)k; (void)nf;
            /* orden de los eventos del 6502: N (NWSHP), luego M (MESS) o B (MSBAR) y S (SOHISS) */
            sprintf(got, "N%d;%s%s", SHIPT_MISSILE, evb, atoi(a[1]) ? ";S120" : "");
            ok_ = strcmp(got, ex) == 0 && nomsl == atoi(a[11]);
            if (atoi(a[1])) {                    /* el blanco: bytes 28, 30, 32 y 36 de su bloque; la estacion (ranura 1): byte 36 */
                int t = atoi(a[4]);
                u8 *e = sl1 + t * NI, *g = state_slot(t);
                ok_ = ok_ && g[28] == e[28] && g[30] == e[30] && g[32] == e[32] && g[36] == e[36] && state_slot(1)[36] == sl1[NI + 36];
                ok_ = ok_ && msar == 0 && mstg == 0xFF;
            }
            sprintf(got + strlen(got), " nomsl %d msar %d mstg %d", nomsl, msar, mstg);
            check(ok_, "FRMIS");
            mstg = 0xFF;
            cnt[6]++;
            break; }
        }
    }
    fclose(f);
    printf("6502: bomba %d, WARP %d, DOCKIT %d, DOKEY %d, GTHG %d, FRS1 %d, FRMIS %d\n", cnt[0], cnt[1], cnt[2], cnt[3], cnt[4], cnt[5], cnt[6]);
    /* ---- propias: la bomba en el bucle por nave (parte 4), su ciclo (partes 3 y 16), ESCAPE y GTHG en MJP */
    {
        int k, nv, nsst, nthg, ncon, nbomb = 0;
        u8 t0 = tally[0], tl = tallyl;
        strcpy(cur, "propias"); got[0] = 0;
        bomb_line_hook = 0;
        ships_clear();
        ship_zinf(); inwk[7] = 4; frin[0] = 0; nv = ship_new(SHIPT_VIPER);
        ship_zinf(); inwk[7] = 5; nsst = ship_new(SHIPT_CORIOLIS);
        ship_zinf(); inwk[7] = 6; nthg = ship_new(SHIPT_THARGOID);
        ship_zinf(); inwk[7] = 7; ncon = ship_new(SHIPT_CONSTRICTOR);
        bomb = 0x7F; combat_view_on = 1;
        bomb_key();
        check(bomb == 0xFE, "bomba: ASL BOMB");
        ships_step(0);
        check((state_slot(nv)[31] & 0xA0) || frin[nv] != SHIPT_VIPER, "bomba: la Viper muere (explota)");
        check(!(state_slot(nsst)[31] & 0x80) && !(state_slot(nthg)[31] & 0x80) && !(state_slot(ncon)[31] & 0x80),
              "bomba: estacion, Thargoid y Constrictor siguen");
        check(tally[0] != t0 || tallyl != tl, "bomba: EXNO2 suma al TALLY");
        for (k = 0; k < 10 && (bomb & 0x80); k++) { bomb_frame(); nbomb++; }
        check(nbomb == 7 && bomb == 0, "bomba: siete fotogramas (de $FE a 0)");
        bomb_key();
        check(bomb == 0, "bomba: gastada");
        /* MJP: tres Thargoides con sus Thargons */
        ships_clear();
        do gthg(); while (many[SHIPT_THARGOID] <= 2);
        check(many[SHIPT_THARGOID] == 3 && many[SHIPT_THARGON] == 3, "MJP: tres Thargoides y tres Thargons");
        /* ESCAPE: la Cobra se va y queda en una ranura */
        ships_clear(); zDELTA = 3;
        ships_escape();
        check(frin[0] == SHIPT_COBRA_MK_3, "ESCAPE: nuestra Cobra en la ranura 0");
    }
    printf("%d comprobaciones, %d fallos\n", total, fails);
    return fails != 0;
}
