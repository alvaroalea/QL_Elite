/* TACTICS against the ORIGINAL 6502 (vectors from tools/gen_tactics.py): ship AI, station, rock, missiles, Thargons;
   INWK, the 8 K% slots, RAND, RAT/RAT2/CNT/CNT2, ECMA, shields, energy, TALLY, messages and the child ship are compared. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tactics.h"
#include "dockit.h"
#include "combat.h"
#include "damage.h"
#include "state.h"
#include "emath.h"
#include "ship3d.h"
#include "ships.h"
#include "space.h"
#include "shipdata.h"
#include "game.h"

static int fails, total;
static char cur[20000], got[2000], mlog[400];
static void check(int ok, const char *what)
{
    total++;
    if (!ok) { if (fails++ < 8 || getenv("ALLF")) printf("FALLO %s: %.200s\n   C: %s\n", what, cur, got); }
}
static int hexb(const char *s, u8 *out, int n)
{
    int i; unsigned v;
    for (i = 0; i < n; i++) { if (sscanf(s + 2 * i, "%2x", &v) != 1) return 0; out[i] = (u8)v; }
    return 1;
}
static void mh(u8 t) { char b[16]; sprintf(b, "%sMESS:%d", mlog[0] ? "," : "", t); strcat(mlog, b); }
static u8 bp[32];

int main(void)
{
    FILE *f = fopen("tests/vectors_tactics.txt", "r");
    static char a[40][700];
    static u8 iw[NI], sl[8 * NI], iwo[NI], slo[8 * NI], r[4], r2[4];
    int cnt = 0, kids = 0;
    if (!f) { printf("falta tests/vectors_tactics.txt (make test_tactics con ELITE_SRC)\n"); return 1; }
    plat_init();
    state_clear();
    combat_mess_hook = mh;
    while (fgets(cur, sizeof cur, f)) {
        int na = 0, i, ok, child = -1; char *pp = cur, *e, explog[400];
        if ((e = strchr(cur, '\n')) != 0) *e = 0;
        while (na < 40 && sscanf(pp, "%699s", a[na]) == 1) { pp = strstr(pp, a[na]) + strlen(a[na]); na++; }
        if (a[0][0] == 'K') {                        /* K xx4 frin ais | ais_out */
            u8 fr[8], ai[8], ao[8];
            hexb(a[2], fr, 8); hexb(a[3], ai, 8); hexb(a[4], ao, 8);
            ships_clear();
            for (i = 0; i < 8; i++) { frin[i] = fr[i]; state_slot(i)[32] = ai[i]; }
            ks2(atoi(a[1]));
            ok = 1;
            for (i = 0; i < 8; i++) ok = ok && state_slot(i)[32] == ao[i];
            got[0] = 0; check(ok, "KS2");
            continue;
        }
        if (a[0][0] == 'M') {                        /* M ky(U,T,M) nomsl mstg msar | mstg msar log */
            u8 ky[3];
            hexb(a[1], ky, 3);
            nomsl = (u8)atoi(a[2]); mstg = (u8)atoi(a[3]); msar = (u8)atoi(a[4]);
            if (ky[0]) missile_unarm();
            if (ky[1]) missile_target();
            ok = (ky[2] && !(mstg & 0x80)) == (strstr(a[7], "FRMIS") != 0);    /* M fires FRMIS if there is a target */
            sprintf(got, "mstg %d msar %d", mstg, msar);
            check(ok && mstg == atoi(a[5]) && msar == atoi(a[6]), "teclas de misil (MA4..MA24)");
            continue;
        }
        if (a[0][0] == 'E') {                        /* E ecmp ecma energy | ecmp ecma energy */
            ecmp = (u8)atoi(a[1]); ecma = (u8)atoi(a[2]); energy = (u8)atoi(a[3]);
            ecm_frame();
            sprintf(got, "%d %d %d", ecmp, ecma, energy);
            check(ecmp == atoi(a[4]) && ecma == atoi(a[5]) && energy == atoi(a[6]), "ECM (MA16..MA66)");
            continue;
        }
        if (na < 30 || a[0][0] != 'T') continue;
        /* T typ iw slots bp shu1 cops thg sspr fist ecma rnd tl t0 t1 | inwk slots rnd rat rat2 cnt cnt2 ecma fsh ash en tl t0 t1 log nw */
        hexb(a[2], iw, NI); hexb(a[3], sl, 8 * NI); memset(bp, 0, sizeof bp); hexb(a[4], bp, 20); hexb(a[11], r, 4);
        hexb(a[16], iwo, NI); hexb(a[17], slo, 8 * NI); hexb(a[18], r2, 4);
        ships_clear();
        for (i = 0; i < 8; i++) memcpy(state_slot(i), sl + i * NI, NI);
        frin[0] = 128; frin[1] = 129; frin[2] = (u8)atoi(a[1]);   /* the child lasers go to slot 3 */
        for (i = 0; i <= NTY; i++) many[i] = 0;
        many[SHIPT_SHUTTLE + 1] = (u8)atoi(a[5]); many[SHIPT_VIPER] = (u8)atoi(a[6]); many[SHIPT_THARGOID] = (u8)atoi(a[7]);
        sspr = (u8)atoi(a[8]); fist = (u8)atoi(a[9]); ecma = (u8)atoi(a[10]);
        memcpy(inwk, iw, NI); zinf = state_slot(2); zxx0 = bp; ztype = (u8)atoi(a[1]);
        fsh = ash = energy = 255; dly = 0; player_dead = 0;
        tallyl = (u8)atoi(a[12]); tally[0] = (u8)atoi(a[13]); tally[1] = (u8)atoi(a[14]);
        t_rat = t_rat2 = t_cnt = t_cnt2 = 0;
        memcpy(em_rand, r, 4);
        mlog[0] = 0;
        tactics();
        if (frin[3]) child = 3;
        ok = memcmp(inwk, iwo, NI) == 0 && memcmp(em_rand, r2, 4) == 0;
        for (i = 0; i < 8; i++) if (i != child) ok = ok && memcmp(state_slot(i), slo + i * NI, NI) == 0;
        ok = ok && t_rat == atoi(a[19]) && t_rat2 == atoi(a[20]) && t_cnt == atoi(a[21]) && t_cnt2 == atoi(a[22]) &&
             ecma == atoi(a[23]) && fsh == atoi(a[24]) && ash == atoi(a[25]) && energy == atoi(a[26]) &&
             tallyl == atoi(a[27]) && tally[0] == atoi(a[28]) && tally[1] == atoi(a[29]);
        /* 6502 messages (without ECBLB or LASNOISE, which are not logged in C) */
        { char *p = a[30], *q = explog; explog[0] = 0;
          if (strcmp(p, "-") != 0) {
              while (*p) { char tok[32]; int l = (int)strcspn(p, ","); memcpy(tok, p, (size_t)l); tok[l] = 0;
                  if (strncmp(tok, "MESS:", 5) == 0) { if (q != explog) *q++ = ','; strcpy(q, tok); q += strlen(tok); }
                  p += l; if (*p) p++; } } }
        ok = ok && strcmp(explog, mlog) == 0;
        /* child ship */
        if (strcmp(a[31], "-") == 0) ok = ok && child < 0;
        else {
            int t; char h[200]; u8 kw[NI];
            sscanf(a[31], "%d/%199s", &t, h); hexb(h, kw, NI);
            ok = ok && child == 3 && frin[3] == t;
            for (i = 0; i < NI && child == 3; i++) if (i != 31 && i != 33 && i != 34 && i != 35 && i != 36) ok = ok && state_slot(3)[i] == kw[i];
            kids++;
        }
        sprintf(got, "rat %d rat2 %d cnt %d cnt2 %d ecma %d f %d a %d e %d tal %d %d %d rnd %d inwk %d log %s child %d/%d",
                t_rat, t_rat2, t_cnt, t_cnt2, ecma, fsh, ash, energy, tallyl, tally[0], tally[1], memcmp(em_rand, r2, 4) == 0,
                memcmp(inwk, iwo, NI) == 0, mlog, child, child >= 0 ? frin[child] : 0);
        check(ok, "TACTICS");
        cnt++;
    }
    fclose(f);
    printf("6502: TACTICS %d casos (%d con nave hija)\n", cnt, kids);
    /* ---- integration: a hostile, aggressive Mamba ahead; the flight loop lets it attack */
    {
        int fr, n, hits = 0, shots = 0, missiles = 0, dead = 0;
        u8 f0;
        combat_mess_hook = 0;
        state_clear(); game_new(); combat_reset(); player_dead = 0; ecma = 0; mstg = 0xFF;
        ships_arrive();
        ship_zinf(); inwk[7] = 3; inwk[27] = 10; inwk[32] = 0xFF; inwk[36] = 0x04;
        n = ship_new(SHIPT_MAMBA);
        zDELTA = 0; alpha = beta = 0; space_view = 0; mcnt = 0;
        f0 = fsh;
        for (fr = 0; fr < 600 && !dead; fr++) {
            u8 ev = space_step();
            if (ev & SPACE_EV_DEATH) dead = 1;
            for (n = 0; n < NOSH; n++) {
                if (frin[n] == SHIPT_MAMBA && (state_slot(n)[31] & 0x40)) shots++;
                if (frin[n] == SHIPT_MISSILE) missiles++;
            }
            if (fsh < f0) hits++;
            f0 = fsh;
            energy_tick(mcnt);
        }
        strcpy(cur, "integracion"); got[0] = 0;
        check(shots > 0, "la Mamba dispara su laser");
        printf("integracion: %d fotogramas, %d con el laser enemigo encendido, %d golpes, misil en %d fotogramas, %s\n",
               fr, shots, hits, missiles, dead ? "muerte" : "seguimos vivos");
    }
    printf("%d comprobaciones, %d fallos\n", total, fails);
    return fails != 0;
}
