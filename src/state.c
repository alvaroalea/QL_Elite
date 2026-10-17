#include "state.h"

u8  inwk[NI];
u8  ks[(NOSH + 1) * NI];
u8 *univ[NOSH + 1];
u8  frin[NOSH + 1];
u8  many[NTY + 1];
u8  junk_count, dock_auto, ecmp, mj, mstg = 0xFF;

u8  zP2;
u8  zk3[16];
u8  zx15[6];
u8  zx12[6];
u8  zxx16[18];
u8  zv[2];

u8  qq0, qq1;
u8  qq2[6];
u8  qq3, qq4, qq5;
u8  qq6[2], qq7[2], qq8[2];
u8  qq9, qq10;
u8  qq11, qq12, qq14;
u8  qq15[6];
u8  qq17;
u8  qq19[6];
u8  qq20[17];
u8  qq21[6];
u8  qq22[2];
u8  qq24, qq25, qq28, qq29, qq26;
u8  cash[4];
u8  gcnt;
u8  laser[4];
u8  crgo;
u8  ecm_fitted, bst, bomb, engy, dkcmp, ghyp, escp;
u8  tribble[2], tallyl, nomsl, fist;
u8  avl[17];
u8  tally[2];
u8  tp, cok, svc;

static void zero(u8 *p, int n) { while (n-- > 0) *p++ = 0; }

u8 *state_slot(int n) { return ks + (long)n * NI; }

void state_clear(void)
{
    int i;
    zero(inwk, NI); zero(ks, (NOSH + 1) * NI);
    zero(frin, NOSH + 1); zero(many, NTY + 1);
    junk_count = dock_auto = ecmp = mj = 0; mstg = 0xFF;
    zP2 = 0; zero(zk3, 16); zero(zx15, 6); zero(zx12, 6); zero(zxx16, 18); zero(zv, 2);
    qq0 = qq1 = 0; zero(qq2, 6); qq3 = qq4 = qq5 = 0;
    zero(qq6, 2); zero(qq7, 2); zero(qq8, 2);
    qq9 = qq10 = qq11 = qq12 = qq14 = 0;
    zero(qq15, 6); qq17 = 0; zero(qq19, 6); zero(qq20, 17); zero(qq21, 6); zero(qq22, 2);
    qq24 = qq25 = qq28 = qq29 = qq26 = 0;
    zero(cash, 4); gcnt = 0; zero(laser, 4); crgo = 0;
    ecm_fitted = bst = bomb = engy = dkcmp = ghyp = escp = 0;
    zero(tribble, 2); tallyl = nomsl = fist = 0; zero(avl, 17); zero(tally, 2); tp = cok = svc = 0;
    for (i = 0; i <= NOSH; i++) univ[i] = state_slot(i);
}
