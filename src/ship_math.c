#include "ship_math.h"

u8 sm_x, sm_y, sm_c, sm_a;

/* ---------- MVT3: suma con signo-magnitud de 24 bits ---------- */
u8 sm_mvt3(u8 x)
{
    u8 a, t;
    int d, c;
    unsigned s;
    a = zK[3]; zS = a; t = (u8)(a & 0x80); zT = t;
    if (((a ^ inwk[x + 2]) & 0x80) == 0) {                 /* mismos signos: sumar */
        s = (unsigned)zK[1] + inwk[x];       zK[1] = (u8)s;
        s = (unsigned)zK[2] + inwk[x + 1] + (s >> 8); zK[2] = (u8)s;
        s = (unsigned)zK[3] + inwk[x + 2] + (s >> 8);
        a = (u8)(((u8)s & 0x7F) | t); zK[3] = a;
        sm_c = (u8)(s >> 8);
        return a;
    }
    zS = (u8)(zS & 0x7F);                                  /* MV13: signos distintos, restar */
    d = (int)inwk[x] - zK[1];            c = (d >= 0); zK[1] = (u8)d;
    d = (int)inwk[x + 1] - zK[2] - (1 - c); c = (d >= 0); zK[2] = (u8)d;
    d = (int)(inwk[x + 2] & 0x7F) - zS - (1 - c); c = (d >= 0); a = (u8)d;
    a = (u8)((a | 0x80) ^ t); zK[3] = a;
    if (c) { sm_c = 1; return a; }                         /* MV14 */
    d = 1 - (int)zK[1] - (1 - c);        c = (d >= 0); zK[1] = (u8)d;   /* negar el resultado */
    d = 0 - (int)zK[2] - (1 - c);        c = (d >= 0); zK[2] = (u8)d;
    d = 0 - (int)zK[3] - (1 - c);        sm_c = (u8)(d >= 0); a = (u8)(((u8)d & 0x7F) | t); zK[3] = a;
    return a;
}

/* ---------- MAS1 a MAS4 ---------- */
u8 sm_mas1(u8 y, u8 x)
{
    u8 a, a2, c;
    a = inwk[y];      c = (u8)(a >> 7);  zK[1] = (u8)(a << 1);
    a2 = inwk[y + 1];                    zK[2] = (u8)((a2 << 1) | c);  c = (u8)(a2 >> 7);
    zK[3] = (u8)(c << 7);
    a = sm_mvt3(x);
    inwk[x + 2] = a;
    inwk[x] = zK[1]; inwk[x + 1] = zK[2];
    return (u8)(a & 0x7F);
}

u8 sm_mas2(u8 a, u8 y)
{
    a |= ks[y + 2]; a |= ks[y + 5]; a |= ks[y + 8];
    return (u8)(a & 0x7F);
}
u8 sm_mas2m(u8 y) { return sm_mas2(0, y); }

u8 sm_mas3(u8 y)
{
    unsigned s;
    u8 a;
    a = em_squa2(ks[y + 1]); zR = a;
    a = em_squa2(ks[y + 4]);
    s = (unsigned)a + zR;                                  /* SQUA2 sale con C = 0 */
    em_carry = 1;                                          /* MA30: desborda con C = 1 */
    if (s > 255) return 0xFF;
    zR = (u8)s;
    a = em_squa2(ks[y + 7]);
    s = (unsigned)a + zR;
    if (s > 255) return 0xFF;
    em_carry = 0;
    return (u8)s;
}

u8 sm_farof2(u8 a) { return (u8)(a >= inwk[1] && a >= inwk[4] && a >= inwk[7]); }
u8 sm_mas4(u8 a) { return (u8)(a | inwk[1] | inwk[4] | inwk[7]); }

/* ---------- VCSUB / TAS1: K3 = INWK - vector de otra nave ---------- */
void sm_tas1(const u8 *v, u8 y)
{
    u8 a, x;
    a = (u8)(v[y] ^ 0x80); zK[3] = a; y--;
    zK[2] = v[y]; y--;
    zK[1] = v[y];
    zU = y; x = zU;
    a = sm_mvt3(x);
    zk3[x + 2] = a; zk3[x + 1] = zK[2]; zk3[x] = zK[1];
}

void sm_vcsub(const u8 *v)
{
    sm_tas1(v, 2);
    sm_tas1(v, 5);
    sm_tas1(v, 8);
}

void sm_vcsu1(void) { sm_vcsub(ks + NI); }

/* ---------- NORM / TAS2 ---------- */
void sm_norm(void)
{
    unsigned s;
    u8 c;
    zR = em_squa(zx15[0]); zQ = zP;
    zT = em_squa(zx15[1]);
    s = (unsigned)zP + zQ; zQ = (u8)s; c = (u8)(s >> 8);
    s = (unsigned)zT + zR + c; zR = (u8)s;
    zT = em_squa(zx15[2]);
    s = (unsigned)zP + zQ; zQ = (u8)s; c = (u8)(s >> 8);
    s = (unsigned)zT + zR + c; zR = (u8)s;
    em_ll5();
    zx15[0] = em_tis2(zx15[0]);
    zx15[1] = em_tis2(zx15[1]);
    zx15[2] = em_tis2(zx15[2]);
}

u8 sm_tas2(void)
{
    u8 a, c, c2;
    zk3[9] = (u8)(zk3[0] | zk3[3] | zk3[6] | 1);
    a = (u8)(zk3[1] | zk3[4] | zk3[7]);
    for (;;) {                                             /* TAL2 */
        c = (u8)(zk3[9] >> 7); zk3[9] = (u8)(zk3[9] << 1);
        c2 = (u8)(a >> 7);     a = (u8)((a << 1) | c);
        if (c2) break;                                     /* BCS TA2 */
        c = (u8)(zk3[0] >> 7); zk3[0] = (u8)(zk3[0] << 1);
        c2 = (u8)(zk3[1] >> 7); zk3[1] = (u8)((zk3[1] << 1) | c);
        c = (u8)(zk3[3] >> 7); zk3[3] = (u8)(zk3[3] << 1);
        c2 = (u8)(zk3[4] >> 7); zk3[4] = (u8)((zk3[4] << 1) | c);
        c = (u8)(zk3[6] >> 7); zk3[6] = (u8)(zk3[6] << 1);
        c2 = (u8)(zk3[7] >> 7); zk3[7] = (u8)((zk3[7] << 1) | c);
        if (!c2) continue;                                 /* BCC TAL2 */
        break;
    }
    zx15[0] = (u8)((zk3[1] >> 1) | zk3[2]);
    zx15[1] = (u8)((zk3[4] >> 1) | zk3[5]);
    zx15[2] = (u8)((zk3[7] >> 1) | zk3[8]);
    sm_norm();
    return zx15[2];
}

/* ---------- TAS3, TAS4, TAS6 ---------- */
u8 sm_tas3(u8 y)
{
    u8 a;
    zQ = inwk[y]; em_mult12(zx15[0]);
    zQ = inwk[y + 2]; a = em_mad(zx15[1]); zS = a; zR = em_x;
    zQ = inwk[y + 4];
    return em_mad(zx15[2]);
}

u8 sm_tas4(u8 y)
{
    u8 a;
    zQ = ks[NI + y]; em_mult12(zx15[0]);
    zQ = ks[NI + y + 2]; a = em_mad(zx15[1]); zS = a; zR = em_x;
    zQ = ks[NI + y + 4];
    return em_mad(zx15[2]);
}

void sm_tas6(void)
{
    zx15[0] ^= 0x80; zx15[1] ^= 0x80; zx15[2] ^= 0x80;
}

/* ---------- TIS3 y TIDY ---------- */
u8 sm_tis3(u8 a, u8 x, u8 y)
{
    zP2 = a;
    zQ = inwk[10 + x]; em_mult12(inwk[16 + x]);
    zQ = inwk[10 + y]; a = em_mad(inwk[16 + y]);
    zP = em_x;
    y = zP2;
    zQ = inwk[10 + y];
    return em_dvidt((u8)(a ^ 0x80));
}

void sm_tidy(void)
{
    u8 a, x, y;
    zx15[0] = inwk[10]; zx15[1] = inwk[12]; zx15[2] = inwk[14];
    sm_norm();
    inwk[10] = zx15[0]; inwk[12] = zx15[1]; inwk[14] = zx15[2];
    y = 4; a = (u8)(zx15[0] & 0x60);
    if (a != 0) {
        x = 2; a = 0;
        inwk[16] = sm_tis3(a, x, y);
    } else {                                               /* TI1 */
        x = a;
        a = (u8)(zx15[1] & 0x60);
        if (a == 0) { a = y; y = 2; inwk[20] = sm_tis3(a, x, y); }   /* TI2 */
        else        { inwk[18] = sm_tis3(2, x, y); }
    }
    zx15[0] = inwk[16]; zx15[1] = inwk[18]; zx15[2] = inwk[20];   /* TI3 */
    sm_norm();
    inwk[16] = zx15[0]; inwk[18] = zx15[1]; inwk[20] = zx15[2];
    zQ = inwk[12]; em_mult12(inwk[20]);
    inwk[22] = (u8)(em_tis1(inwk[14], inwk[18]) ^ 0x80);
    em_mult12(inwk[16]);
    inwk[24] = (u8)(em_tis1(inwk[10], inwk[20]) ^ 0x80);
    em_mult12(inwk[18]);
    inwk[26] = (u8)(em_tis1(inwk[12], inwk[16]) ^ 0x80);
    for (x = 14; x < 128; x = (u8)(x - 2)) inwk[9 + x] = 0;       /* X = 14, 12, ..., 0 */
}

/* ---------- SPS1 a SPS4 ---------- */
void sm_sps3(u8 x)
{
    u8 y = ks[2 + x];
    zk3[x] = ks[1 + x];
    zk3[x + 1] = (u8)(y & 0x7F);
    zk3[x + 2] = (u8)(y & 0x80);
}

void sm_sps1(void)
{
    sm_sps3(0); sm_sps3(3); sm_sps3(6);
    sm_tas2();
}

void sm_sps4(void)
{
    int x;
    for (x = 8; x >= 0; x--) zk3[x] = ks[NI + x];
    sm_tas2();
}

u8 sm_sps2(u8 a)
{
    u8 x, y;
    y = (u8)((a >> 7) << 7);
    x = (u8)(a << 1);
    zQ = 20;
    em_dvid4(x);
    x = zP;
    if (y & 0x80) { sm_y = 0xFF; x = (u8)((x ^ 0xFF) + 1); }
    else          { sm_y = 0; }
    sm_x = x;
    return x;
}

/* ---------- PLS6 / PROJ ---------- */
u8 sm_pls6(u8 a)
{
    u8 x;
    unsigned d;
    zP2 = a;
    em_dvid3b(a, (u8)(inwk[6] | 1), inwk[7], inwk[8]);
    if (((zK[3] & 0x7F) | zK[2]) != 0) { sm_c = 1; return 1; }      /* PL21 */
    x = zK[1];
    sm_x = x;
    if (x >= 4) { sm_c = 1; return 1; }
    if (!(zK[3] & 0x80)) { sm_c = 0; return 0; }
    d = (unsigned)(u8)(zK[0] ^ 0xFF) + 1;
    zK[0] = (u8)d;
    x = (u8)((u8)(x ^ 0xFF) + (d >> 8));
    sm_x = x; sm_c = 0;
    return 0;
}

u8 sm_proj(void)
{
    unsigned s;
    zP = inwk[0]; zP1 = inwk[1];
    if (sm_pls6(inwk[2])) { sm_a = (u8)((zK[3] & 0x7F) | zK[2]); return 1; }
    s = (unsigned)zK[0] + VIEW_CX; zk3[0] = (u8)s;
    s = (unsigned)sm_x + (s >> 8); zk3[1] = (u8)s;
    zP = inwk[3]; zP1 = inwk[4];
    if (sm_pls6((u8)(inwk[5] ^ 0x80))) { sm_a = (u8)((zK[3] & 0x7F) | zK[2]); return 1; }
    if (view_yscale) {                                      /* vista nativa: desplazamiento vertical x 6/5 */
        int v = (int)(((unsigned)sm_x << 8) | zK[0]);
        v += v / 5;
        zK[0] = (u8)v; sm_x = (u8)(((unsigned)v >> 8) & 0xFF);
    }
    s = (unsigned)zK[0] + VIEW_CY; zk4[0] = (u8)s;
    s = (unsigned)sm_x + (s >> 8); zk4[1] = (u8)s;
    sm_a = (u8)s; sm_c = 0;
    return 0;
}

/* ---------- Dinero ---------- */
u8 sm_mcash(u8 x, u8 y)
{
    unsigned s;
    s = (unsigned)x + cash[3]; cash[3] = (u8)s;
    s = (unsigned)y + cash[2] + (s >> 8); cash[2] = (u8)s;
    s = (unsigned)cash[1] + (s >> 8); cash[1] = (u8)s;
    s = (unsigned)cash[0] + (s >> 8); cash[0] = (u8)s;
    sm_c = 0;
    return 0;
}

u8 sm_lcash(u8 x, u8 y)
{
    int d, c;
    d = (int)cash[3] - x;                 c = (d >= 0); cash[3] = (u8)d;
    d = (int)cash[2] - y - (1 - c);       c = (d >= 0); cash[2] = (u8)d;
    d = (int)cash[1] - (1 - c);           c = (d >= 0); cash[1] = (u8)d;
    d = (int)cash[0] - (1 - c);           c = (d >= 0); cash[0] = (u8)d;
    if (c) { sm_c = 1; return 1; }                         /* TT113 */
    return sm_mcash(x, y);                                 /* sin saldo: restituir */
}

u8 sm_gcash(void)
{
    unsigned a, v;
    a = em_multu();
    v = (((a & 0xFF) << 8) | zP) & 0xFFFF;
    v = (v << 2) & 0xFFFF;
    sm_y = (u8)(v >> 8); zP = (u8)v; sm_x = zP;
    return sm_x;
}

/* ---------- SCALEX / SCALEY ---------- */
u8 sm_scaley2(u8 a)
{
    u8 t3 = a, x;
    int d, c;
    unsigned s;
    a = (u8)(a >> 2);
    d = (int)a - t3; c = (d >= 0); x = (u8)d;
    s = (unsigned)(u8)(x ^ 0xFF) + 1 + c;
    sm_c = (u8)(s >> 8);
    return (u8)s;
}

u8 sm_scaley(u8 a) { return sm_scaley2((u8)(a >> 1)); }

u8 sm_scalex(u8 a)
{
    unsigned s;
    a = sm_scaley2(a);
    s = (unsigned)a + 32 + sm_c;
    sm_c = (u8)(s >> 8);
    return (u8)s;
}
