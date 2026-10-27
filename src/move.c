#include "move.h"
#include "space.h"
#include "state.h"
#include "emath.h"
#include "ship_math.h"
#include "ship3d.h"
#include "shipdata.h"
#include "ships.h"

u8 move_xsav;
void (*move_tactics_hook)(void) = 0;

static u8 k2[4];                          /* K2..K2+3 */
u8 move_p2;                               /* P+2 */
u8 move_rat2;                             /* RAT2 of MV45/MVS5 */

/* MVT1: (x_sg x_hi x_lo) = (x_sg x_hi x_lo) +/- (A R); A = sign (bit 7) and high part (bits 0-6) of the amount; X = 0, 3 or 6 */
void mvt1(u8 a, u8 x)
{
    u8 s, t;
    unsigned u;
    int d, cc;
    s = (u8)(a & 0x7F);
    t = (u8)(a & 0x80);
    if (!((t ^ inwk[x + 2]) & 0x80)) {           /* same sign: add */
        u = (unsigned)zR + inwk[x]; inwk[x] = (u8)u;
        u = (unsigned)s + inwk[x + 1] + (u >> 8); inwk[x + 1] = (u8)u;
        u = (unsigned)inwk[x + 2] + (u >> 8);
        inwk[x + 2] = (u8)((u8)u | t);
        return;
    }
    d = (int)inwk[x] - zR; cc = (d >= 0); inwk[x] = (u8)d;                    /* MV10: subtract */
    d = (int)inwk[x + 1] - s - (1 - cc); cc = (d >= 0); inwk[x + 1] = (u8)d;
    d = (int)(inwk[x + 2] & 0x7F) - (1 - cc); cc = (d >= 0);
    inwk[x + 2] = (u8)((((u8)d) | 0x80) ^ t);
    if (cc) return;                                                           /* BCS MV11 */
    d = 1 - (int)inwk[x] - 1; cc = (d >= 0); inwk[x] = (u8)d;                 /* crossed zero: negate */
    d = 0 - (int)inwk[x + 1] - (1 - cc); cc = (d >= 0); inwk[x + 1] = (u8)d;
    d = 0 - (int)inwk[x + 2] - (1 - cc);
    inwk[x + 2] = (u8)(((u8)d & 0x7F) | t);
}

/* MVT6: (A P+2 P+1) = (x_sg x_hi x_lo of INWK+X) + (A P+2 P+1) with signs; returns the sign in A */
u8 mvt6(u8 a, u8 x)
{
    u8 y = a;
    unsigned s;
    int d, cc;
    if (!((y ^ inwk[x + 2]) & 0x80)) {
        s = (unsigned)zP1 + inwk[x]; zP1 = (u8)s;
        s = (unsigned)move_p2 + inwk[x + 1] + (s >> 8); move_p2 = (u8)s;
        return y;
    }
    d = (int)inwk[x] - zP1; cc = (d >= 0); zP1 = (u8)d;                       /* MV50 */
    d = (int)inwk[x + 1] - move_p2 - (1 - cc); cc = (d >= 0); move_p2 = (u8)d;
    if (cc) return (u8)(y ^ 0x80);
    d = 1 - (int)zP1 - 1; cc = (d >= 0); zP1 = (u8)d;                         /* MV51 */
    d = 0 - (int)move_p2 - (1 - cc); move_p2 = (u8)d;
    return y;
}

/* MLTU2 transcribed literally (16 rounds of ROR/ADC) to obtain the carry with which the 6502 returns: it is the last bit shifted out of
   ROR P. Leaves (A P+1 P) as em_mltu2 */
static u8 mltu2(u8 a, u8 *carry)
{
    u8 x, c, t, p1;
    unsigned s;
    a = (u8)(a ^ 0xFF);
    c = (u8)(a & 1); a = (u8)(a >> 1); p1 = a; a = 0;      /* EOR #$FF; LSR A; STA P+1; LDA #0 */
    t = zP; zP = (u8)((c << 7) | (t >> 1)); c = (u8)(t & 1);   /* ROR P */
    for (x = 16; x; x--) {
        if (!c) {                                            /* BCS MU21 not taken */
            s = (unsigned)a + zQ;                            /* ADC Q (con C = 0) */
            c = (u8)(s >> 8); a = (u8)s;
            t = a; a = (u8)((c << 7) | (a >> 1)); c = (u8)(t & 1);   /* ROR A */
        } else {
            c = (u8)(a & 1); a = (u8)(a >> 1);               /* MU21: LSR A */
        }
        t = p1; p1 = (u8)((c << 7) | (p1 >> 1)); c = (u8)(t & 1);    /* ROR P+1 */
        t = zP; zP = (u8)((c << 7) | (zP >> 1)); c = (u8)(t & 1);    /* ROR P */
    }
    zP1 = p1;
    *carry = c;
    return a;
}

/* MVS4: rotates an orientation vector (INWK+Y) by roll (ALPHA) and pitch (BETA) */
void mvs4(u8 y)
{
    u8 a, x;
    zQ = alpha;
    zR = inwk[y + 2]; zS = inwk[y + 3]; zP = inwk[y];
    a = (u8)(inwk[y + 1] ^ 0x80); a = em_mad(a); x = em_x;
    inwk[y + 3] = a; inwk[y + 2] = x; zP = x;
    zR = inwk[y]; zS = inwk[y + 1];
    a = inwk[y + 3]; a = em_mad(a); x = em_x;
    inwk[y + 1] = a; inwk[y] = x; zP = x;
    zQ = beta;
    zR = inwk[y + 2]; zS = inwk[y + 3]; zP = inwk[y + 4];
    a = (u8)(inwk[y + 5] ^ 0x80); a = em_mad(a); x = em_x;
    inwk[y + 3] = a; inwk[y + 2] = x; zP = x;
    zR = inwk[y + 4]; zS = inwk[y + 5];
    a = inwk[y + 3]; a = em_mad(a); x = em_x;
    inwk[y + 5] = a; inwk[y + 4] = x;
}

/* MVS5: rotates the vector pair (INWK+X, INWK+Y) by 3.6 degrees */
void mvs5(u8 x, u8 y)
{
    u8 a, t, c, i, q;
    int d, cc;
    a = (u8)((inwk[x + 1] & 0x7F) >> 1); t = a;
    d = (int)inwk[x] - t; cc = (d >= 0); zR = (u8)d;
    d = (int)inwk[x + 1] - (1 - cc); zS = (u8)d;
    zP = inwk[y];
    t = (u8)(inwk[y + 1] & 0x80);
    a = (u8)(inwk[y + 1] & 0x7F);
    for (i = 0; i < 4; i++) { c = (u8)(a & 1); a = (u8)(a >> 1); zP = (u8)((zP >> 1) | (c << 7)); }
    a = (u8)(a | t); a = (u8)(a ^ move_rat2);
    q = x;
    a = em_add(a);
    zK[1] = a; zK[0] = em_x;
    x = q;
    a = (u8)((inwk[y + 1] & 0x7F) >> 1); t = a;
    d = (int)inwk[y] - t; cc = (d >= 0); zR = (u8)d;
    d = (int)inwk[y + 1] - (1 - cc); zS = (u8)d;
    zP = inwk[x];
    t = (u8)(inwk[x + 1] & 0x80);
    a = (u8)(inwk[x + 1] & 0x7F);
    for (i = 0; i < 4; i++) { c = (u8)(a & 1); a = (u8)(a >> 1); zP = (u8)((zP >> 1) | (c << 7)); }
    a = (u8)(a | t); a = (u8)(a ^ 0x80); a = (u8)(a ^ move_rat2);
    a = em_add(a);
    inwk[y + 1] = a; inwk[y] = em_x;
    x = q;
    inwk[x] = zK[0]; inwk[x + 1] = zK[1];
}

/* MV45 y MV5 */
void mv45(void)
{
    u8 a, c;
    zR = zDELTA;
    mvt1(0x80, 6);                                /* z -= DELTA */
    if ((ztype & 0x81) == 129) return;            /* the sun does not rotate */
    mvs4(9); mvs4(15); mvs4(21);
    move_rat2 = (u8)(inwk[30] & 0x80);
    a = (u8)(inwk[30] & 0x7F);
    if (a != 0) {                                 /* pitch counter */
        c = (u8)(a >= 0x7F);
        if (!c) a = (u8)(a - 1);
        inwk[30] = (u8)(a | move_rat2);
        mvs5(15, 9); mvs5(17, 11); mvs5(19, 13);
    }
    move_rat2 = (u8)(inwk[29] & 0x80);                 /* MV8 */
    a = (u8)(inwk[29] & 0x7F);
    if (a != 0) {                                 /* roll counter */
        c = (u8)(a >= 0x7F);
        if (!c) a = (u8)(a - 1);
        inwk[29] = (u8)(a | move_rat2);
        mvs5(15, 21); mvs5(17, 23); mvs5(19, 25);
    }
    if ((inwk[31] & 0xA0) != 0) { inwk[31] = (u8)(inwk[31] & 0xEF); return; }    /* MV5 / MVD1 */
    inwk[31] = (u8)(inwk[31] | 0x10);
    space_scan(ztype);
}

/* MV40 up to just before MV45 (separate so it can be compared with the original) */
void mv40_pre(void)
{
    u8 a, t;
    unsigned s;
    int d, cc;
    zQ = (u8)(alpha ^ 0x80);
    zP = inwk[0]; zP1 = inwk[1];
    em_mult3(inwk[2]);
    sm_mvt3(3);
    k2[1] = zK[1]; zP = zK[1]; k2[2] = zK[2]; zP1 = zK[2];
    zQ = beta;
    k2[3] = zK[3];
    em_mult3(zK[3]);
    sm_mvt3(6);
    zP = zK[1]; inwk[6] = zK[1]; zP1 = zK[2]; inwk[7] = zK[2]; inwk[8] = zK[3];
    a = (u8)(zK[3] ^ 0x80);
    em_mult3(a);
    t = (u8)(zK[3] & 0x80);
    a = (u8)(t ^ k2[3]);
    if (a & 0x80) goto mv1;
    s = (unsigned)zK[0] + k2[0];
    s = (unsigned)zK[1] + k2[1] + (s >> 8); inwk[3] = (u8)s;
    s = (unsigned)zK[2] + k2[2] + (s >> 8); inwk[4] = (u8)s;
    s = (unsigned)zK[3] + k2[3] + (s >> 8); a = (u8)s;
    goto mv2;
mv1:
    d = (int)zK[0] - k2[0]; cc = (d >= 0);
    d = (int)zK[1] - k2[1] - (1 - cc); cc = (d >= 0); inwk[3] = (u8)d;
    d = (int)zK[2] - k2[2] - (1 - cc); cc = (d >= 0); inwk[4] = (u8)d;
    zP = (u8)(k2[3] & 0x7F);
    d = (int)(zK[3] & 0x7F) - zP - (1 - cc); cc = (d >= 0); zP = (u8)d; a = zP;
    if (cc) goto mv2;
    d = 1 - (int)inwk[3] - 1; cc = (d >= 0); inwk[3] = (u8)d;
    d = 0 - (int)inwk[4] - (1 - cc); cc = (d >= 0); inwk[4] = (u8)d;
    d = 0 - (int)zP - (1 - cc);
    a = (u8)((u8)d | 0x80);
mv2:
    a = (u8)(a ^ t);
    inwk[5] = a;
    zQ = alpha;
    zP = inwk[3]; zP1 = inwk[4];
    em_mult3(inwk[5]);
    sm_mvt3(0);
    inwk[0] = zK[1]; inwk[1] = zK[2]; inwk[2] = zK[3];
}

/* MV40: planets and suns: the position rotates with our roll and pitch and then continues through MV45 */
void mv40(void)
{
    mv40_pre();
    mv45();
}

/* MVEIT */
void mveit(void)
{
    u8 a, c, c3, max;
    unsigned s;
    int d, cc;
    const u8 *bp;
    if ((inwk[31] & 0xA0) != 0) goto mv30;
    if (((mcnt ^ move_xsav) & 15) == 0) sm_tidy();
    if (ztype & 0x80) { mv40(); return; }
    if (!(inwk[32] & 0x80)) goto mv30;
    if (ztype == SHIPT_MISSILE) goto mv26;
    if (((mcnt ^ move_xsav) & 7) != 0) goto mv30;
mv26:
    if (move_tactics_hook) move_tactics_hook();
mv30:
    space_scan(ztype);
    /* part 2: advance along the nose (speed INWK+27) */
    zQ = (u8)(inwk[27] << 2);
    a = em_fmltu((u8)(inwk[10] & 0x7F)); zR = a; mvt1((u8)(inwk[10] & 0x80), 0);
    a = em_fmltu((u8)(inwk[12] & 0x7F)); zR = a; mvt1((u8)(inwk[12] & 0x80), 3);
    a = em_fmltu((u8)(inwk[14] & 0x7F)); zR = a; mvt1((u8)(inwk[14] & 0x80), 6);
    a = (u8)(inwk[27] + inwk[28]);                       /* speed += acceleration */
    if (a & 0x80) a = 0;
    bp = ship_bp_for(ztype);
    max = bp[15];
    if (a >= max) a = max;
    inwk[27] = a; inwk[28] = 0;
    /* part 3: rotation of the position by our roll (ALP1, ALP2) and pitch (BET1, BET2) */
    zQ = alp1;
    zP = (u8)(inwk[0] ^ 0xFF);
    move_p2 = mltu2(inwk[1], &c);
    a = (u8)(alp2[1] ^ inwk[2]); a = mvt6(a, 3);
    k2[3] = a; k2[1] = zP1; zP = (u8)(zP1 ^ 0xFF); k2[2] = move_p2;
    zQ = bet1;
    move_p2 = mltu2(move_p2, &c);
    a = (u8)(k2[3] ^ bet2[0]); a = mvt6(a, 6);
    inwk[8] = a;
    inwk[6] = zP1; zP = (u8)(zP1 ^ 0xFF); inwk[7] = move_p2;
    move_p2 = mltu2(move_p2, &c3);                                 /* MLTU2: Q is still BET1 */
    inwk[5] = k2[3];
    a = (u8)(k2[3] ^ bet2[0] ^ inwk[8]);
    if (!(a & 0x80)) goto mv43;
    s = (unsigned)zP1 + k2[1] + c3; inwk[3] = (u8)s;                          /* opposite signs: add */
    s = (unsigned)move_p2 + k2[2] + (s >> 8); inwk[4] = (u8)s;
    goto mv44;
mv43:
    d = (int)k2[1] - zP1 - (1 - (int)c3); cc = (d >= 0); inwk[3] = (u8)d;
    d = (int)k2[2] - move_p2 - (1 - cc); cc = (d >= 0); inwk[4] = (u8)d;
    if (cc) goto mv44;
    d = 1 - (int)inwk[3] - 1; cc = (d >= 0); inwk[3] = (u8)d;
    d = 0 - (int)inwk[4] - (1 - cc); inwk[4] = (u8)d;
    inwk[5] = (u8)(inwk[5] ^ 0x80);
mv44:
    zQ = alp1;
    zP = (u8)(inwk[3] ^ 0xFF);
    move_p2 = mltu2(inwk[4], &c);
    a = (u8)(alp2[0] ^ inwk[5]); a = mvt6(a, 0);
    inwk[2] = a; inwk[1] = move_p2; inwk[0] = zP1;
    mv45();
}

/* PLUT: inverts or swaps the axes of INWK according to the view */
void plut(u8 view)
{
    u8 rat, r2, a, x, y, k;
    if (view == 0) return;
    if (view == 1) {                               /* rear view: x, z and the same ones of the three vectors change sign */
        static const u8 flip[8] = { 2, 8, 10, 14, 16, 20, 22, 26 };
        for (k = 0; k < 8; k++) inwk[flip[k]] = (u8)(inwk[flip[k]] ^ 0x80);
        return;
    }
    r2 = (u8)((view - 1 >= 2) ? 0x80 : 0);         /* LDA #0; CPX #2; ROR A (X = VIEW - 1) */
    rat = (u8)(r2 ^ 0x80);
    a = inwk[0]; x = inwk[6]; inwk[6] = a; inwk[0] = x;
    a = inwk[1]; x = inwk[7]; inwk[7] = a; inwk[1] = x;
    x = (u8)(inwk[2] ^ rat);
    inwk[2] = (u8)(inwk[8] ^ r2);
    inwk[8] = x;
    for (k = 0; k < 3; k++) {                      /* PUS1 for the vectors at 9, 15 and 21 */
        y = (u8)(9 + 6 * k);
        a = inwk[y]; x = inwk[y + 4]; inwk[y + 4] = a; inwk[y] = x;
        x = (u8)(inwk[y + 1] ^ rat);
        inwk[y + 1] = (u8)(inwk[y + 5] ^ r2);
        inwk[y + 5] = x;
    }
}
