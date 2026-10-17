#include "emath.h"
#include "math_tables.h"
#include "sne_table.h"

u8 zP, zP1, zQ, zR, zS, zT, zU;
u8 em_x, em_carry;
u8 em_rand[4] = { 0x11, 0x22, 0x33, 0x44 };
u8 em_rand_x, em_rand_c, em_rand_v;

/* DORND: dos sumas encadenadas (secuencia "feeder" + secuencia principal) con el acarreo propagado. */
u8 em_dornd(void)
{
    unsigned s;
    u8 a, x, b, res, c;
    a = em_rand[0];
    c = (u8)(a >> 7);
    a = (u8)((a << 1) | (em_rand_c & 1));
    x = a;
    s = (unsigned)a + em_rand[2] + c;
    em_rand[0] = (u8)s; em_rand[2] = x;
    c = (u8)(s >> 8);
    a = em_rand[1];
    b = em_rand[3];
    s = (unsigned)a + b + c;
    res = (u8)s;
    em_rand_c = (u8)(s >> 8);
    em_rand_v = (u8)(((a ^ res) & (b ^ res) & 0x80) ? 1 : 0);
    em_rand[1] = res; em_rand[3] = a;
    em_rand_x = a;                           /* X = valor anterior de RAND+1 */
    return res;
}

u8 em_dornd2(void) { em_rand_c = 0; return em_dornd(); }

u8 em_multu(void)
{
    unsigned p;
    if (zQ == 0) { zP = 0; return 0; }
    p = (unsigned)zP * zQ;
    zP = (u8)p;
    return (u8)(p >> 8);
}

u8 em_mu11(u8 x)
{
    unsigned p = (unsigned)zP * x;
    zP = (u8)p;
    return (u8)(p >> 8);
}

u8 em_squa2(u8 a)
{
    unsigned p = (unsigned)a * a;
    zP = (u8)p;
    return (u8)(p >> 8);
}

u8 em_squa(u8 a) { return em_squa2((u8)(a & 0x7F)); }

u8 em_mult1(u8 a)
{
    unsigned p;
    u8 sign = (u8)((a ^ zQ) & 0x80);
    u8 ma = (u8)(a & 0x7F), mq = (u8)(zQ & 0x7F);
    if (mq == 0) { zP = 0; return 0; }
    p = (unsigned)ma * mq;
    zP = (u8)p;
    return (u8)(((p >> 8) & 0x7F) | sign);
}

void em_mult12(u8 a)
{
    zS = em_mult1(a);
    zR = zP;
}

/* ADD se transcribe literalmente: con desbordamiento (suma > 15 bits) el original pisa el bit de signo. */
u8 em_add(u8 a)
{
    u8 t1 = a, t = (u8)(a & 0x80), x, u, r;
    unsigned s;
    zT = t;
    if ((a ^ zS) & 0x80) {                   /* signos distintos */
        u = (u8)(zS & 0x7F); zU = u;
        s = (unsigned)zP - zR;               /* SEC; SBC R */
        x = (u8)s;
        {
            int borrow = (zP < zR);
            int hi = (int)(t1 & 0x7F) - (int)u - borrow;
            r = (u8)hi;
            if (hi < 0) {                    /* |A| < |S|: negar el resultado */
                u8 uu = r;
                zU = uu;
                s = (unsigned)(u8)(x ^ 0xFF) + 1;
                x = (u8)s;
                r = (u8)(0 - uu - (u8)(1 - (u8)(s >> 8)));
                r |= 0x80;
            }
        }
        em_x = x;
        return (u8)(r ^ t);
    }
    s = (unsigned)zR + zP;
    x = (u8)s;
    r = (u8)((unsigned)zS + t1 + (s >> 8));
    em_x = x;
    return (u8)(r | t);
}

u8 em_mad(u8 a)
{
    a = em_mult1(a);
    return em_add(a);
}

u8 em_mults(u8 a)
{
    unsigned p;
    u8 t = (u8)(a & 0x80), ma = (u8)(a & 0x7F);
    if (ma == 0) { zP1 = 0; zP = 0; return 0; }
    p = (unsigned)(zP & 31) * ma;
    zP = (u8)p;
    return (u8)(((p >> 8) & 0x7F) | t);
}

u8 em_fmltu(u8 a)
{
    unsigned lo, hi;
    em_carry = 0;
    if (a == 0 || zQ == 0) return 0;
    lo = (unsigned)em_logL[a] + em_logL[zQ];
    hi = (unsigned)em_log[zQ] + em_log[a] + (lo >> 8);
    if (hi < 256) return 0;
    em_carry = 1;
    return em_alogh[(u8)hi];
}

u8 em_mltu2(u8 a)
{
    unsigned long v = ((unsigned long)a << 8) | (u8)(zP ^ 0xFF);
    unsigned long p = v * zQ;
    zP = (u8)p;
    zP1 = (u8)(p >> 8);
    return (u8)(p >> 16);
}

u8 em_tis2(u8 a)
{
    u8 y = a, t, c, co, r, q;
    a &= 0x7F;
    if (a >= zQ) return (u8)((y & 0x80) | 96);
    t = 0xFE;
    do {
        a = (u8)(a << 1);                    /* el acarreo de ASL se pierde, como en el 6502 */
        c = 0;
        if (a >= zQ) { a = (u8)(a - zQ); c = 1; }
        co = (u8)(t >> 7);
        t = (u8)((t << 1) | c);
    } while (co);
    q = (u8)(t >> 2);
    r = (u8)((q >> 1) + q + (q & 1));
    zT = r;
    return (u8)((y & 0x80) | r);
}

u8 em_dvid96(u8 a)
{
    u8 t = (u8)(a & 0x80), t1 = 254, c, co;
    a &= 0x7F;
    do {
        a = (u8)(a << 1);
        c = 0;
        if (a >= 96) { a = (u8)(a - 96); c = 1; }
        co = (u8)(t1 >> 7);
        t1 = (u8)((t1 << 1) | c);
    } while (co);
    zT = t;
    return (u8)(t1 | t);
}

u8 em_tis1(u8 x, u8 a)
{
    zQ = x;
    a ^= 0x80;
    a = em_mad(a);
    return em_dvid96(a);
}

/* ---------------- segunda tanda ---------------- */
u8 zK[4];
u8 zU, zDELTA;

u8 em_ll5(void)
{
    unsigned long v = ((unsigned long)zR << 8) | zQ;
    unsigned long bit = 0x4000UL, res = 0;
    while (bit > v) bit >>= 2;
    while (bit) {
        if (v >= res + bit) { v -= res + bit; res = (res >> 1) + bit; }
        else res >>= 1;
        bit >>= 2;
    }
    zQ = (u8)res;
    return zQ;
}

u8 em_ll28_nocheck(u8 a)
{
    unsigned lo, hi;
    em_carry = 0;
    if (a == 0) { zR = 0; return 0; }
    lo = (unsigned)em_logL[a] - em_logL[zQ];
    hi = (unsigned)em_log[a] - em_log[zQ] - ((lo >> 8) & 1);
    if (!(hi & 0x100)) { em_carry = 1; zR = 255; return 255; }   /* sin prestamo: cociente >= 1 */
    zR = em_alogh[(u8)hi];
    return zR;
}

u8 em_ll28(u8 a)
{
    if (a >= zQ) { em_carry = 1; zR = 255; return 255; }
    return em_ll28_nocheck(a);
}

u8 em_ll61(u8 a)
{
    u8 x = 0, s, r;
    unsigned t;
    if (zQ == 0) { zR = 50; zU = 50; return 50; }
    do { a >>= 1; x++; } while (a >= zQ);
    s = x;
    em_ll28(a);
    x = s; a = zR;
    do {
        t = (unsigned)a << 1;
        a = (u8)t;
        zU = (u8)((zU << 1) | (t >> 8));
        if (zU & 0x80) { zR = 50; zU = 50; return 50; }
        x--;
    } while (x != 0);
    r = a;
    zR = r;
    return r;
}

u8 em_ll38(u8 a)
{
    unsigned s;
    int d;
    em_carry = 0;
    if (((a ^ zS) & 0x80) == 0) {
        s = (unsigned)zQ + zR;
        em_carry = (u8)(s >> 8);
        return (u8)s;
    }
    d = (int)zR - (int)zQ;
    if (d >= 0) return (u8)d;
    zS ^= 0x80;
    s = (unsigned)(u8)(~(u8)d) + 1;
    em_carry = (u8)(s >> 8);
    return (u8)s;
}

u8 em_dvid4(u8 a)
{
    u8 p, rem, c, nc, i;
    unsigned t;
    c = (u8)(a >> 7);
    p = (u8)(a << 1);
    rem = 0;
    for (i = 0; i < 8; i++) {
        t = ((unsigned)rem << 1) | c;
        rem = (u8)t;
        if (rem >= zQ) { rem = (u8)(rem - zQ); c = 1; } else c = 0;
        nc = (u8)(p >> 7);
        p = (u8)((p << 1) | c);
        c = nc;
    }
    zP = p;
    em_ll28_nocheck(rem);
    em_x = 0;
    return p;
}

u8 em_dv41(u8 a)
{
    zQ = a;
    return em_dvid4(zDELTA);
}

u8 em_dv42(u8 z_hi) { return em_dv41(z_hi); }

u8 em_dvidt(u8 a)
{
    u8 acc = 0, c, c2, nc, i;
    u8 q;
    unsigned t;
    zP1 = a;
    zT = (u8)((a ^ zQ) & 0x80);
    c = (u8)(zP >> 7);
    zP = (u8)(zP << 1);
    c2 = (u8)(zP1 >> 7);
    zP1 = (u8)((zP1 << 1) | c);
    q = (u8)(zQ & 0x7F);
    zQ = q;
    for (i = 0; i < 16; i++) {
        t = ((unsigned)acc << 1) | c2;
        c2 = (u8)(t >> 8);
        acc = (u8)t;
        if (acc >= q) { acc = (u8)(acc - q); c = 1; } else c = 0;
        nc = (u8)(zP >> 7);
        zP = (u8)((zP << 1) | c);
        c = nc;
        nc = (u8)(zP1 >> 7);
        zP1 = (u8)((zP1 << 1) | c);
        c2 = nc;
    }
    return (u8)(zP | zT);
}

void em_mult3(u8 a)
{
    unsigned long mag;
    u8 mq = (u8)(zQ & 0x7F);
    unsigned long k;
    if (mq == 0) { zK[0] = zK[1] = zK[2] = zK[3] = 0; em_carry = 0; return; }
    mag = (((unsigned long)(a & 0x7F) << 16) | ((unsigned long)zP1 << 8) | zP) * mq;
    k = mag | ((unsigned long)((a ^ zQ) & 0x80) << 24);
    zK[0] = (u8)k; zK[1] = (u8)(k >> 8); zK[2] = (u8)(k >> 16); zK[3] = (u8)(k >> 24);
    em_carry = 0;
}

/* DVID3B: transcripcion literal; P(2 1 0) = (a zP1 zP), denominador (S R Q) = (z_sign z_hi z_lo) */
void em_dvid3b(u8 a, u8 z_lo, u8 z_hi, u8 z_sign)
{
    u8 p0 = (u8)(zP | 1), p1 = zP1, p2 = a;
    u8 q = z_lo, r = z_hi, y = 0, t, acc, c, c2, c0, cc, i;
    unsigned w;
    t = (u8)((a ^ z_sign) & 0x80);
    acc = (u8)(a & 0x7F);
    for (;;) {
        if (acc >= 64) break;
        c = (u8)(p0 >> 7); p0 = (u8)(p0 << 1);
        c2 = (u8)(p1 >> 7); p1 = (u8)((p1 << 1) | c);
        acc = (u8)((acc << 1) | c2);
        y++;
        if (y == 0) break;
    }
    p2 = acc;
    acc = (u8)(z_sign & 0x7F);
    for (;;) {
        y--;
        c = (u8)(q >> 7); q = (u8)(q << 1);
        c2 = (u8)(r >> 7); r = (u8)((r << 1) | c);
        acc = (u8)((acc << 1) | c2);
        if (acc & 0x80) break;
    }
    q = acc; r = 254; acc = p2;
    for (;;) {
        c = (u8)(acc >> 7); acc = (u8)(acc << 1);
        if (c) {
            acc = (u8)(acc - q);
            cc = 1;
        } else {
            cc = (u8)(acc >= q);
            if (cc) acc = (u8)(acc - q);
        }
        c0 = (u8)(r >> 7); r = (u8)((r << 1) | cc);
        if (!c0) break;
    }
    zK[0] = zK[1] = zK[2] = zK[3] = 0;
    if (y & 0x80) {
        acc = r;
        do {
            w = (unsigned)acc << 1; acc = (u8)w; c = (u8)(w >> 8);
            w = ((unsigned)zK[1] << 1) | c; zK[1] = (u8)w; c = (u8)(w >> 8);
            w = ((unsigned)zK[2] << 1) | c; zK[2] = (u8)w; c = (u8)(w >> 8);
            zK[3] = (u8)((zK[3] << 1) | c);
            y++;
        } while (y != 0);
        zK[0] = acc;
        zK[3] |= t;
    } else if (y == 0) {
        zK[0] = r; zK[3] = t;
    } else {
        acc = r;
        do { acc >>= 1; y--; } while (y != 0);
        zK[0] = acc; zK[3] = t;
    }
    zT = t;
    (void)i; (void)p2;
}

u8 em_fmltu2(u8 a)
{
    zQ = em_sne[a & 31];
    return em_fmltu(zK[0]);
}

u8 em_mlu2(u8 a)
{
    zP = (u8)(a & 0x7F);
    return em_multu();
}

u8 em_mlu1(u8 sy, u8 *y1)
{
    *y1 = sy;
    return em_mlu2(sy);
}
