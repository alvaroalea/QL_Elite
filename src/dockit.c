#include "dockit.h"
#include "state.h"
#include "emath.h"
#include "ship_math.h"
#include "ship3d.h"
#include "shipdata.h"
#include "space.h"
#include "ships.h"

u8 t_rat, t_rat2, t_cnt, t_cnt2;

/* TA152 and the tail of TACTICS (TA15, TA11, TA6, PH10E, TA9): CNT = A (nose towards the target); pitch and roll towards
   XX15 (with RAT if the angle exceeds RAT2) and acceleration according to CNT and CNT2 */
void ta152(u8 a)
{
    t_cnt = a;
    ta15();
}

void ta15(void)
{
    u8 a, x;
    a = sm_tas3(16);                             /* TA15: XX15 . roof */
    x = a;
    inwk[30] = (u8)((a ^ 0x80) & 0x80);
    a = (u8)(x << 1);
    if (a >= t_rat2) inwk[30] = (u8)(inwk[30] | t_rat);
    a = (u8)(inwk[29] << 1);                     /* TA11 */
    if (a < 32) {
        a = sm_tas3(22);                         /* XX15 . side */
        x = a;
        inwk[29] = (u8)(((a ^ inwk[30]) & 0x80) ^ 0x80);
        a = (u8)(x << 1);
        if (a >= t_rat2) inwk[29] = (u8)(inwk[29] | t_rat);
    }
    a = t_cnt;                                   /* TA6 */
    if (!(a & 0x80) && a >= t_cnt2) { inwk[28] = 3; return; }   /* PH10E */
    a = (u8)(a & 0x7F);                          /* TA9 */
    if (a < 18) return;
    a = 0xFF;
    if (ztype == SHIPT_MISSILE) a = 0xFE;        /* ASL A */
    inwk[28] = a;
}

/* TA151: if the nose already points (>= $98) there is no need to turn fast (RAT2 = 0) */
void ta151(void)
{
    u8 a = sm_tas3(10);
    if (a >= 0x98) t_rat2 = 0;
    ta152(a);
}

/* GOPL: towards the planet */
void gopl(void)
{
    sm_sps1();
    ta151();
}

/* TA2: XX15 = K3 reduced (high >> 1 with the sign byte) and normalised (falls through to NORM) */
static void ta2(void)
{
    zx15[0] = (u8)((zk3[1] >> 1) | zk3[2]);
    zx15[1] = (u8)((zk3[4] >> 1) | zk3[5]);
    zx15[2] = (u8)((zk3[7] >> 1) | zk3[8]);
    sm_norm();
}

/* TAS7: K3(x+2 x+1 x) += 2 * a (a signed in bit 7), in sign-magnitude */
static void tas7(u8 a, u8 x)
{
    u8 r, c;
    unsigned s;
    int d;
    c = (u8)(a >> 7);
    r = (u8)(a << 1);                            /* ASL A; STA R */
    if (!(((u8)((c << 7) ^ 0x80) ^ zk3[x + 2]) & 0x80)) {      /* LDA #0; ROR A; EOR #$80; EOR K3+2,X; BMI TS71 */
        s = (unsigned)r + zk3[x];                /* ADC K3,X (C = 0 after the ROR) */
        zk3[x] = (u8)s;
        if (s >> 8) zk3[x + 1]++;
        return;
    }
    d = (int)zk3[x] - r; c = (u8)(d >= 0); zk3[x] = (u8)d;     /* TS71 */
    d = (int)zk3[x + 1] - (1 - c); c = (u8)(d >= 0); zk3[x + 1] = (u8)d;
    if (c) return;
    s = (unsigned)(u8)(zk3[x] ^ 0xFF) + 1;       /* crosses zero: negate (16 bits) and change the sign */
    zk3[x] = (u8)s;
    zk3[x + 1] = (u8)((zk3[x + 1] ^ 0xFF) + (s >> 8));
    zk3[x + 2] = (u8)(zk3[x + 2] ^ 0x80);
}

static void dcs1_body(void)
{
    tas7(ks[NI + 10], 0);
    tas7(ks[NI + 12], 3);
    tas7(ks[NI + 14], 6);
}

/* DCS1: JSR P%+3: the body twice: K3 += 4 * station nose */
void dcs1(void) { dcs1_body(); dcs1_body(); }

static void ph22(void) { inwk[28] = 0; inwk[27] = 1; }

/* DOCKIT: docking computer (for a ship or, with negative TYPE, for ours) */
void dockit(void)
{
    u8 a, x, c;
    t_rat2 = 6;
    t_rat = 3;
    t_cnt2 = 29;
    if (sspr == 0) { gopl(); return; }
    sm_vcsu1();                                  /* K3 = position - station */
    if ((zk3[2] | zk3[5] | zk3[8]) & 0x7F) { gopl(); return; }
    ta2();
    zK[0] = zQ;                                  /* LDA Q; STA K: distance */
    sm_tas2();
    a = sm_tas4(10);                             /* XX15 . station nose */
    if ((a & 0x80) || a < 35) goto ph1;
    a = sm_tas3(10);                             /* XX15 . our nose */
    if (a >= 0xA2) goto ph3;
    if (zK[0] >= 157 && (ztype & 0x80)) goto ph3;
    sm_tas6();                                   /* PH2 */
    ta151();
    ph22();
    return;
ph1:
    sm_vcsu1();
    dcs1();
    dcs1();
    sm_tas2();
    sm_tas6();
    ta151();
    return;
ph3:
    x = 0;
    t_rat2 = 0;
    inwk[30] = 0;
    if (ztype & 0x80) {
        a = (u8)(ztype ^ zx15[0] ^ zx15[1]);
        c = (u8)(a >> 7);
        inwk[29] = (u8)((c << 7) | 1);           /* ASL A; LDA #2; ROR A */
        a = (u8)(zx15[0] << 1);
        if (a >= 12) { ph22(); return; }
        c = (u8)(zx15[1] >> 7);
        inwk[30] = (u8)((c << 7) | 1);
        a = (u8)(zx15[1] << 1);
        if (a >= 12) { ph22(); return; }
    }
    inwk[29] = x;                                /* PH32 */
    zx15[0] = inwk[22]; zx15[1] = inwk[24]; zx15[2] = inwk[26];
    a = sm_tas4(16);
    a = (u8)(a << 1);
    if (a >= 66) {                               /* TN11 */
        inwk[28]++;
        inwk[29] = 0x7F;
    } else {
        ph22();
    }
    if (zk3[10] == 0) inwk[I_NEWB] = (u8)(inwk[I_NEWB] | 0x80);   /* TN13: K3+10 (= XX2+10): see docs/HALLAZGOS.md */
}

/* DOKEY with the docking computer: our ship as INWK (ZINF, TYPE = $E0), DOCKIT, and its result in virtual keys
   (speed, roll, pitch) and in JSTX/JSTY; it also limits DELTA to 22. The keys are added to the player's */
void dokey_auto(struct controls *c)
{
    u8 a;
    if (!dock_auto) return;
    ship_zinf();
    inwk[14] = 96;
    inwk[22] = (u8)(96 | 0x80);
    ztype = inwk[22];                            /* STA TYPE: $E0, negative = our ship */
    inwk[27] = zDELTA;
    dockit();
    a = inwk[27];
    if (a >= 22) a = 22;
    zDELTA = a;
    if (inwk[28] != 0) {                         /* LDY INWK+28; BEQ DK11; BMI: KY1 (brake) / KY2 (accelerate) */
        if (inwk[28] & 0x80) c->slow_down = 1; else c->speed_up = 1;
    }
    a = (u8)(inwk[29] << 1);                     /* DK11: ASL INWK+29 */
    if (a == 0) jstx = 128;                      /* BEQ DK12 with A = 128 */
    else {
        if (a & 0x80) {                          /* BIT: bit 6 of the original roll: hard turn */
            jstx = 64;                           /* STA KLO,X with A = 0: that key is released even if pressed; JSTX = 64 */
            if (inwk[29] & 0x80) c->roll_right = 0; else c->roll_left = 0;
        } else {
            if (inwk[29] & 0x80) c->roll_right = 1; else c->roll_left = 1;   /* BCC: KY3; otherwise, KY4 */
        }
    }
    a = (u8)(inwk[30] << 1);                     /* DK12: ASL INWK+30 */
    if (a == 0) jsty = 128;                      /* BEQ DK13 with A = 128 */
    else {
        if (inwk[30] & 0x80) c->pitch_up = 1; else c->pitch_down = 1;       /* BCS: KY5; otherwise, KY6 */
    }
}
