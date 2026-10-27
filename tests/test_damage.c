/* Damage to our ship against the ORIGINAL 6502 (vectors from tools/gen_damage.py): OOPS + OUCH, parts 9 and 10 (collisions,
   scooping, ISDK), part 15 (ENERGY LOW, altitude, cabin, fuel) and one iteration of D1 (DEATH). More own tests. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "combat.h"
#include "damage.h"
#include "state.h"
#include "emath.h"
#include "ship3d.h"
#include "ships.h"
#include "space.h"
#include "shipdata.h"
#include "game.h"
#include "cmdr.h"

static int fails, total;
static char cur[4096];
static void check(int ok, const char *what)
{
    total++;
    if (!ok) { if (fails++ < 12 || getenv("ALLF")) printf("FALLO %s: %.*s\n", what, getenv("ALLF") ? 4000 : 160, cur); }
}
static int hexb(const char *s, u8 *out, int n)
{
    int i; unsigned v;
    for (i = 0; i < n; i++) { if (sscanf(s + 2 * i, "%2x", &v) != 1) return 0; out[i] = (u8)v; }
    return 1;
}
static u8 bp[32];
static char mlog[256];
static void mhook(u8 t)
{
    char b[32];
    sprintf(b, "%sMESS:%d:%d", mlog[0] ? "," : "", t, mess_de);
    if (strlen(mlog) + strlen(b) < sizeof mlog) strcat(mlog, b);
}
static void put_items(const u8 *it) { int x; for (x = 0; x < 22; x++) *ouch_item((u8)x) = it[x]; }
static int same_items(const u8 *it) { int x; for (x = 0; x < 22; x++) if (*ouch_item((u8)x) != it[x]) return 0; return 1; }
/* 6502 log without EXNO3 (the sound is not logged in C) */
static void only_mess(const char *in, char *o)
{
    const char *p = in;
    o[0] = 0;
    if (*p == '-') return;
    while (*p) {
        const char *e = strchr(p, ',');
        size_t l = e ? (size_t)(e - p) : strlen(p);
        if (strncmp(p, "MESS:", 5) == 0) { size_t k = strlen(o); if (k) o[k++] = ','; memcpy(o + k, p, l); o[k + l] = 0; }
        if (!e) break;
        p = e + 1;
    }
}

int main(void)
{
    FILE *f = fopen("tests/vectors_damage.txt", "r");
    char a[40][300], exp[256];
    int cnt[4] = { 0, 0, 0, 0 };
    if (!f) { printf("falta tests/vectors_damage.txt (make test_damage con ELITE_SRC)\n"); return 1; }
    plat_init();
    state_clear();
    combat_mess_hook = mhook;
    while (fgets(cur, sizeof cur, f)) {
        int na = 0; char *pp = cur;
        while (na < 40 && sscanf(pp, "%299s", a[na]) == 1) { pp = strstr(pp, a[na]) + strlen(a[na]); na++; }
        if (na == 0) continue;
        mlog[0] = 0; player_dead = 0; mess_de = 0;
        switch (a[0][0]) {
        case 'O': {
            /* O t c zs f a e items dl rnd | fsh ash energy dead items rnd log */
            u8 it[22], it2[22], r[4], r2[4];
            hexb(a[7], it, 22); hexb(a[9], r, 4); hexb(a[14], it2, 22); hexb(a[15], r2, 4);
            zinf = state_slot(2); zinf[8] = (u8)atoi(a[3]);
            fsh = (u8)atoi(a[4]); ash = (u8)atoi(a[5]); energy = (u8)atoi(a[6]);
            put_items(it); dly = (u8)atoi(a[8]); memcpy(em_rand, r, 4);
            oops((u8)atoi(a[1]), (u8)atoi(a[2]));
            only_mess(a[16], exp);
            check(fsh == atoi(a[10]) && ash == atoi(a[11]) && energy == atoi(a[12]) && player_dead == atoi(a[13]) &&
                  same_items(it2) && memcmp(em_rand, r2, 4) == 0 && strcmp(exp, mlog) == 0, "OOPS");
            cnt[0]++;
            break; }
        case 'C': {
            /* C typ iw bst delta pl st36 b0 f a e items dl rnd c crgo trib | in31 in36 delta fae dd items rnd log */
            u8 iw[NI], pl[9], it[22], it2[22], r[4], r2[4];
            long fae;
            int col, dd;
            hexb(a[2], iw, NI); hexb(a[5], pl, 9); hexb(a[11], it, 22); hexb(a[13], r, 4); hexb(a[22], it2, 22); hexb(a[23], r2, 4);
            ships_clear();
            memcpy(inwk, iw, NI); memcpy(state_slot(2), iw, NI); zinf = state_slot(2);
            memcpy(state_slot(0), pl, 9); state_slot(1)[I_NEWB] = (u8)atoi(a[6]);
            memset(bp, 0, sizeof bp); bp[0] = (u8)atoi(a[7]); zxx0 = bp; ztype = (u8)atoi(a[1]);
            bst = (u8)atoi(a[3]); zDELTA = (u8)atoi(a[4]);
            fsh = (u8)atoi(a[8]); ash = (u8)atoi(a[9]); energy = (u8)atoi(a[10]);
            put_items(it); dly = (u8)atoi(a[12]); memcpy(em_rand, r, 4); em_rand_c = (u8)atoi(a[14]);
            crgo = (u8)atoi(a[15]); tribble[0] = 0; tribble[1] = (u8)atoi(a[16]);
            col = combat_collide();
            dd = (player_dead ? 1 : 0) + (col == COL_DOCK ? 2 : 0);
            fae = atol(a[20]);
            only_mess(a[24], exp);
            if (getenv("ALLF")) printf("  C: 31 %d 36 %d D %d fae %ld dd %d items %d rnd %d log %s exp %s\n", inwk[31], inwk[I_NEWB], zDELTA,
                (long)fsh * 65536L + ash * 256L + energy, dd, same_items(it2), memcmp(em_rand, r2, 4) == 0, mlog, exp);
            check(inwk[31] == atoi(a[17]) && inwk[I_NEWB] == atoi(a[18]) && zDELTA == atoi(a[19]) &&
                  fsh == ((fae >> 16) & 255) && ash == ((fae >> 8) & 255) && energy == (fae & 255) && dd == atoi(a[21]) &&
                  same_items(it2) && memcmp(em_rand, r2, 4) == 0 && strcmp(exp, mlog) == 0, "choques (partes 9 y 10)");
            cnt[1]++;
            break; }
        case 'P': {
            /* P mj mc e au sspr bst delta q14 cab pl su | altit cab q14 dead log */
            u8 pl[9], su[9], d;
            hexb(a[10], pl, 9); hexb(a[11], su, 9);
            ships_clear();
            memcpy(state_slot(0), pl, 9); memcpy(state_slot(1), su, 9);
            mj = (u8)atoi(a[1]); energy = (u8)atoi(a[3]); dock_auto = (u8)atoi(a[4]); sspr = (u8)atoi(a[5]);
            bst = (u8)atoi(a[6]); zDELTA = (u8)atoi(a[7]); qq14 = (u8)atoi(a[8]); cabtmp = (u8)atoi(a[9]); altit = 77;
            dly = 0; zQ = (u8)atoi(a[17]);
            d = damage_checks((u8)atoi(a[2]));
            only_mess(a[16], exp);
            check(altit == atoi(a[12]) && cabtmp == atoi(a[13]) && qq14 == atoi(a[14]) && d == atoi(a[15]) &&
                  player_dead == atoi(a[15]) && strcmp(exp, mlog) == 0, "parte 15");
            mj = 0; sspr = 0; dock_auto = 0;
            cnt[2]++;
            break; }
        case 'X': {
            /* X rnd c delta | type inwk slot31 mcnt lasct rnd */
            u8 r[4], iw[NI], r2[4];
            int n, i, ok;
            hexb(a[1], r, 4); hexb(a[5], iw, NI); hexb(a[9], r2, 4);
            ships_clear();
            frin[0] = frin[1] = frin[2] = 128;                    /* the new ship goes to slot 3, as on the 6502 */
            memcpy(em_rand, r, 4); em_rand_c = (u8)atoi(a[2]); zDELTA = (u8)atoi(a[3]);
            mcnt = 0; lasct = 0;
            n = ships_death_debris_one();
            ok = n == 3 && frin[3] == atoi(a[4]) && state_slot(3)[31] == atoi(a[6]) && mcnt == atoi(a[7]) &&
                 lasct == atoi(a[8]) && memcmp(em_rand, r2, 4) == 0;
            for (i = 0; i < NI; i++) if (i != 31 && i != 33 && i != 34 && i != 35 && i != 36) ok = ok && state_slot(3)[i] == iw[i];
            if (!ok && getenv("ALLF")) { printf("  C: n %d frin3 %d s31 %d mcnt %d lasct %d rnd %02x%02x%02x%02x slot ", n, frin[3], state_slot(3)[31], mcnt, lasct, em_rand[0], em_rand[1], em_rand[2], em_rand[3]);
                for (i = 0; i < NI; i++) printf("%02x", state_slot(3)[i]);
                printf("\n"); }
            check(ok, "D1 (DEATH)");
            cnt[3]++;
            break; }
        }
    }
    fclose(f);
    printf("6502: OOPS %d, choques %d, parte 15 %d, D1 %d\n", cnt[0], cnt[1], cnt[2], cnt[3]);

    /* ---- own tests */
    strcpy(cur, "propias");
    combat_mess_hook = 0;
    zinf = state_slot(2); zinf[8] = 0;
    fsh = 100; ash = 50; energy = 200;
    check(oops(30, 1) == 0 && fsh == 70 && ash == 50 && energy == 200, "OOPS: el escudo delantero absorbe un golpe de delante");
    zinf[8] = 0x80;
    check(oops(30, 1) == 0 && ash == 20 && fsh == 70, "OOPS: el trasero absorbe un golpe de detras");
    fsh = 10; zinf[8] = 0; energy = 200; player_dead = 0;
    check(oops(30, 1) == 0 && fsh == 0 && energy == 180, "OOPS: lo que no para el escudo sale de la energia");
    fsh = 0; energy = 20; player_dead = 0;
    check(oops(30, 1) == 1 && player_dead == 1, "OOPS: sin escudo ni energia, muerte");
    /* ---- integration: a still Viper 40 units ahead; the flight loop crashes it into us */
    {
        int n, f, ev = 0, gone = 0;
        u8 f0;
        game_new(); combat_reset(); player_dead = 0; dly = 0; mj = 0;
        ships_arrive();
        ship_zinf(); inwk[6] = 40; inwk[27] = 0; inwk[29] = 0; inwk[30] = 0;
        n = ship_new(SHIPT_VIPER); state_slot(n)[I_AI] = 0;
        zDELTA = 0; alpha = beta = 0; space_view = 0; mcnt = 1;
        f0 = fsh;
        for (f = 0; f < 60; f++) {
            ev |= space_step();
            gone = 1;
            for (n = 0; n < NOSH; n++) if (frin[n] == SHIPT_VIPER) gone = 0;
            if (gone) break;
        }
        strcpy(cur, "integracion choque");
        check(!(ev & SPACE_EV_DEATH) && fsh < f0 && energy == 255, "choque: el escudo delantero absorbe 128 + energia/2");
        check(gone, "choque: la Viper explota y se quita");
        printf("integracion: choque frontal, escudo delantero %d -> %d, %d fotogramas hasta quitar la Viper\n", f0, fsh, f);
        /* a second collision without shield: OOPS takes it from energy and we may die */
        fsh = 0; energy = 60;
        ship_zinf(); inwk[6] = 40; inwk[27] = 0;
        n = ship_new(SHIPT_ANACONDA); state_slot(n)[I_AI] = 0;
        ev = space_step();
        check((ev & SPACE_EV_DEATH) && player_dead, "choque sin escudo con una Anaconda: muerte");
        /* D1: debris until slot 4 is filled */
        ships_clear(); player_dead = 0;
        ships_death_debris();
        check(frin[4] != 0 && frin[5] == 0, "DEATH: cinco restos (ranuras 0 a 4)");
    }
    printf("%d comprobaciones, %d fallos\n", total, fails);
    return fails != 0;
}
