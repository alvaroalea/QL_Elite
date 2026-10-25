#include "dockit.h"
#include "state.h"
#include "emath.h"
#include "ship_math.h"
#include "ship3d.h"
#include "shipdata.h"
#include "space.h"
#include "ships.h"

u8 t_rat, t_rat2, t_cnt, t_cnt2;

/* TA152 y la cola de TACTICS (TA15, TA11, TA6, PH10E, TA9): CNT = A (morro hacia el objetivo); cabeceo y alabeo hacia
   XX15 (con RAT si el angulo pasa de RAT2) y aceleracion segun CNT y CNT2 */
void ta152(u8 a)
{
    t_cnt = a;
    ta15();
}

void ta15(void)
{
    u8 a, x;
    a = sm_tas3(16);                             /* TA15: XX15 . techo */
    x = a;
    inwk[30] = (u8)((a ^ 0x80) & 0x80);
    a = (u8)(x << 1);
    if (a >= t_rat2) inwk[30] = (u8)(inwk[30] | t_rat);
    a = (u8)(inwk[29] << 1);                     /* TA11 */
    if (a < 32) {
        a = sm_tas3(22);                         /* XX15 . lateral */
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

/* TA151: si el morro ya apunta (>= $98) no hace falta girar deprisa (RAT2 = 0) */
void ta151(void)
{
    u8 a = sm_tas3(10);
    if (a >= 0x98) t_rat2 = 0;
    ta152(a);
}

/* GOPL: hacia el planeta */
void gopl(void)
{
    sm_sps1();
    ta151();
}

/* TA2: XX15 = K3 reducido (alto >> 1 con el byte de signo) y normalizado (cae en NORM) */
static void ta2(void)
{
    zx15[0] = (u8)((zk3[1] >> 1) | zk3[2]);
    zx15[1] = (u8)((zk3[4] >> 1) | zk3[5]);
    zx15[2] = (u8)((zk3[7] >> 1) | zk3[8]);
    sm_norm();
}

/* TAS7: K3(x+2 x+1 x) += 2 * a (a con signo en el bit 7), en signo-magnitud */
static void tas7(u8 a, u8 x)
{
    u8 r, c;
    unsigned s;
    int d;
    c = (u8)(a >> 7);
    r = (u8)(a << 1);                            /* ASL A; STA R */
    if (!(((u8)((c << 7) ^ 0x80) ^ zk3[x + 2]) & 0x80)) {      /* LDA #0; ROR A; EOR #$80; EOR K3+2,X; BMI TS71 */
        s = (unsigned)r + zk3[x];                /* ADC K3,X (C = 0 tras el ROR) */
        zk3[x] = (u8)s;
        if (s >> 8) zk3[x + 1]++;
        return;
    }
    d = (int)zk3[x] - r; c = (u8)(d >= 0); zk3[x] = (u8)d;     /* TS71 */
    d = (int)zk3[x + 1] - (1 - c); c = (u8)(d >= 0); zk3[x + 1] = (u8)d;
    if (c) return;
    s = (unsigned)(u8)(zk3[x] ^ 0xFF) + 1;       /* cruza el cero: negar (16 bits) y cambiar el signo */
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

/* DCS1: JSR P%+3: el cuerpo dos veces: K3 += 4 * morro de la estacion */
void dcs1(void) { dcs1_body(); dcs1_body(); }

static void ph22(void) { inwk[28] = 0; inwk[27] = 1; }

/* DOCKIT: ordenador de atraque (para una nave o, con TYPE negativo, para la nuestra) */
void dockit(void)
{
    u8 a, x, c;
    t_rat2 = 6;
    t_rat = 3;
    t_cnt2 = 29;
    if (sspr == 0) { gopl(); return; }
    sm_vcsu1();                                  /* K3 = posicion - estacion */
    if ((zk3[2] | zk3[5] | zk3[8]) & 0x7F) { gopl(); return; }
    ta2();
    zK[0] = zQ;                                  /* LDA Q; STA K: distancia */
    sm_tas2();
    a = sm_tas4(10);                             /* XX15 . morro de la estacion */
    if ((a & 0x80) || a < 35) goto ph1;
    a = sm_tas3(10);                             /* XX15 . nuestro morro */
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
    if (zk3[10] == 0) inwk[I_NEWB] = (u8)(inwk[I_NEWB] | 0x80);   /* TN13: K3+10 (= XX2+10): ver docs/HALLAZGOS.md */
}

/* DOKEY con el ordenador de atraque: nuestra nave como INWK (ZINF, TYPE = $E0), DOCKIT, y su resultado en teclas virtuales
   (velocidad, alabeo, cabeceo) y en JSTX/JSTY; tambien limita DELTA a 22. Las teclas se suman a las del jugador */
void dokey_auto(struct controls *c)
{
    u8 a;
    if (!dock_auto) return;
    ship_zinf();
    inwk[14] = 96;
    inwk[22] = (u8)(96 | 0x80);
    ztype = inwk[22];                            /* STA TYPE: $E0, negativo = nuestra nave */
    inwk[27] = zDELTA;
    dockit();
    a = inwk[27];
    if (a >= 22) a = 22;
    zDELTA = a;
    if (inwk[28] != 0) {                         /* LDY INWK+28; BEQ DK11; BMI: KY1 (frenar) / KY2 (acelerar) */
        if (inwk[28] & 0x80) c->slow_down = 1; else c->speed_up = 1;
    }
    a = (u8)(inwk[29] << 1);                     /* DK11: ASL INWK+29 */
    if (a == 0) jstx = 128;                      /* BEQ DK12 con A = 128 */
    else {
        if (a & 0x80) {                          /* BIT: bit 6 del alabeo original: giro fuerte */
            jstx = 64;                           /* STA KLO,X con A = 0: esa tecla queda suelta aunque se pulse; JSTX = 64 */
            if (inwk[29] & 0x80) c->roll_right = 0; else c->roll_left = 0;
        } else {
            if (inwk[29] & 0x80) c->roll_right = 1; else c->roll_left = 1;   /* BCC: KY3; si no, KY4 */
        }
    }
    a = (u8)(inwk[30] << 1);                     /* DK12: ASL INWK+30 */
    if (a == 0) jsty = 128;                      /* BEQ DK13 con A = 128 */
    else {
        if (inwk[30] & 0x80) c->pitch_up = 1; else c->pitch_down = 1;       /* BCS: KY5; si no, KY6 */
    }
}
