/* SUN, CIRCLE y apoyo: transcripcion de la fuente de referencia (Elite Apple II) con sus acarreos y saltos.
   Diferencias: HLOIN/LOIN son las primitivas XOR de gfx.c (el original enmascara a pixeles pares por el modo Apple II). */
#include "sun.h"
#include "state.h"
#include "emath.h"
#include "ship3d.h"

u8 sun_col = C_WHITE;
u8 sun_lso[256];
u8 sun_lsx = 255;
u8 circ_lsx2[256], circ_lsy2[256];
u8 circ_lsp = 1, circ_flag, circ_stp, circ_cnt;

#define X1 zx15[0]
#define Y1 zx15[1]
#define X2 zx15[2]
#define Y2 zx15[3]

static u8 sunx[2];                       /* SUNX (16 bits) */
static u8 yy[2];                         /* YY */
static u8 chk_p1, chk_p2;                /* P+1, P+2 (los pone CHKON) */
static u8 k5[4], k6[4];                  /* K5, K6 */

static void hloin(void) { gfx_hline_xor(X1, X2, Y1, sun_col); }
static void loin(void)  { gfx_line(X1, Y1, X2, Y2, sun_col, 1); }

/* FLFLLS */
void sun_flflls(void)
{
    u8 y;
    for (y = 199; y > 0; y--) sun_lso[y] = 0;      /* LDY #199 ... BNE */
    sun_lsx = 255;                                 /* DEY: Y pasa de 0 a $FF; STY LSX */
}

/* CHKON: acarreo 1 si el circulo no esta en la vista; ademas P+1 = K4+K (bajo) y P+2 (alto) para SUN */
u8 sun_chkon(void)
{
    unsigned s;
    int d;
    u8 hi, c, x;
    s = (unsigned)zk3[0] + zK[0]; hi = (u8)(zk3[1] + (s >> 8));
    if (hi & 0x80) return 1;                       /* BMI PL21 */
    d = (int)zk3[0] - zK[0]; c = (u8)(d >= 0); hi = (u8)(zk3[1] - 0 - (1 - c));
    if (!(hi & 0x80)) { if (hi != 0) return 1; }   /* BMI PL31; BNE PL21 */
    s = (unsigned)zk4[0] + zK[0]; chk_p1 = (u8)s; hi = (u8)(zk4[1] + (s >> 8));
    if (hi & 0x80) return 1;
    chk_p2 = hi;
    d = (int)zk4[0] - zK[0]; x = (u8)d; c = (u8)(d >= 0); hi = (u8)(zk4[1] - 0 - (1 - c));
    if (hi & 0x80) return 0;                       /* BMI PL44: CLC; RTS */
    if (hi != 0) return 1;                         /* BNE PL21 */
    return (u8)(x >= SUN_YX2M1);                   /* CPX Yx2M1; RTS: C = X >= 135 */
}

/* EDGES: X1 = YY - A, X2 = YY + A recortados a 0..255; devuelve 1 (y LSO[y] = 0) si queda fuera */
static u8 edges(u8 a, u8 y)
{
    u8 t = a, hi, c;
    unsigned s;
    int d;
    s = (unsigned)a + yy[0]; X2 = (u8)s;
    hi = (u8)(yy[1] + (s >> 8));
    if (hi & 0x80) goto ed1;
    if (hi != 0) X2 = 255;
    d = (int)yy[0] - t; c = (u8)(d >= 0); X1 = (u8)d;
    hi = (u8)(yy[1] - 0 - (1 - c));
    if (hi == 0) return 0;
    if (!(hi & 0x80)) goto ed1;                    /* BPL ED1 */
    X1 = 0;
    return 0;
ed1:
    sun_lso[y] = 0;
    return 1;
}

/* HLOIN2: EDGES, Y1 = y, LSO[y] = 0, HLOIN (YY ya tiene el centro) */
static void hloin2(u8 a, u8 y)
{
    edges(a, y);
    Y1 = y;
    sun_lso[y] = 0;
    hloin();
}

/* WPLS: borra el sol anterior (si LSX no es negativo) */
void sun_wpls(void)
{
    u8 y;
    if (sun_lsx & 0x80) return;                    /* LDA LSX; BMI WPLS-1 */
    yy[0] = sunx[0]; yy[1] = sunx[1];
    for (y = SUN_YX2M1; y != 0; y--)
        if (sun_lso[y] != 0) hloin2(sun_lso[y], y);
    sun_lsx = 255;                                 /* DEY: Y = 0 -> $FF; STY LSX */
}

/* SUN */
void sun_draw(void)
{
    u8 x, y, a, c, cnt, tgt, t, q, r, vlo, vhi, k2lo, k2hi, xxl, xxh;
    unsigned s;
    int d;

    sun_col = C_RED;
    sun_lsx = 1;
    if (sun_chkon()) { sun_wpls(); return; }       /* BCS PLF3M3: JMP WPLS */
    x = zK[0]; a = 0;                              /* LDA #0; LDX K; CPX #96; ROL A; CPX #40; ROL A; CPX #16; ROL A */
    a = (u8)((a << 1) | (x >= 96));
    a = (u8)((a << 1) | (x >= 40));
    a = (u8)((a << 1) | (x >= 16));
    cnt = a;                                       /* PLF18 */
    a = SUN_YX2M1; x = chk_p2;
    if (x != 0) goto plf2;
    if (a < chk_p1) goto plf2;                     /* CMP P+1; BCC PLF2 */
    a = chk_p1;
    if (a != 0) goto plf2;
    a = 1;
plf2:
    tgt = a;
    d = (int)SUN_YX2M1 - zk4[0]; x = (u8)d; c = (u8)(d >= 0);
    a = (u8)(0 - zk4[1] - (1 - c));
    if (a & 0x80) goto plf3;                       /* BMI PLF3 */
    if (a != 0) goto plf4;                         /* BNE PLF4 */
    if (x == 0) goto plf17;                        /* INX; DEX; BEQ PLF17 */
    if (x < zK[0]) goto plf5;                      /* CPX K; BCC PLF5 */
plf4:
    x = zK[0]; a = 0;
    goto plf5;
plf3:
    x = (u8)(0 - x);                               /* TXA; EOR #$FF; CLC; ADC #1; TAX */
plf17:
    a = 0xFF;
plf5:
    vlo = x; vhi = a;                              /* STX V; STA V+1 */
    a = em_squa2(zK[0]);                           /* LDA K; JSR SQUA2 */
    k2hi = a; k2lo = zP;
    y = SUN_YX2M1;
    yy[0] = sunx[0]; yy[1] = sunx[1];
plfl2:
    if (y == tgt) goto plfl;
    a = sun_lso[y];
    if (a == 0) goto plf13;
    hloin2(a, y);
plf13:
    y--;
    if (y != 0) goto plfl2;
plfl:
    a = em_squa2(vlo);                             /* LDA V; JSR SQUA2 */
    t = a; zT = t;
    q = (u8)(k2lo - zP); c = (u8)(k2lo >= zP);
    r = (u8)(k2hi - t - (1 - c));
    zQ = q; zR = r;
    Y1 = y;
    em_ll5();                                      /* JSR LL5: Q = SQRT(R Q) */
    y = Y1;
    a = (u8)(em_dornd() & cnt);
    s = (unsigned)a + zQ;                          /* CLC; ADC Q */
    a = (u8)s;
    if (s >> 8) a = 255;
    x = sun_lso[y]; sun_lso[y] = a;                /* PLF44 */
    if (x == 0) goto plf11;
    yy[0] = sunx[0]; yy[1] = sunx[1];
    a = x; edges(a, y);
    xxl = X1; xxh = X2;                            /* XX, XX+1 */
    yy[0] = zk3[0]; yy[1] = zk3[1];
    a = sun_lso[y];
    if (edges(a, y)) goto plf23;                   /* BCS PLF23 */
    a = X2; x = xxl; X2 = x; xxl = a;
    hloin();
plf23:
    X1 = xxl; X2 = xxh;
plf16:
    hloin();
plf6:
    y--;
    if (y == 0) goto plf8;
    if (vhi != 0) goto plf10;
    vlo--;
    if (vlo != 0) goto plfl;
    vhi--;                                         /* DEC V+1: 0 -> $FF */
    goto plfl;
plf11:
    yy[0] = zk3[0]; yy[1] = zk3[1];
    if (!edges(a, y)) goto plf16;                  /* A = ancho nuevo; BCC PLF16 */
    sun_lso[y] = 0;
    goto plf6;
plf10:
    vlo++;
    if (vlo <= zK[0]) goto plfl;                   /* CPX K; BCC; BEQ PLFLS */
    yy[0] = sunx[0]; yy[1] = sunx[1];
plfl3:
    a = sun_lso[y];
    if (a != 0) hloin2(a, y);
    y--;
    if (y != 0) goto plfl3;
plf8:
    sunx[0] = zk3[0]; sunx[1] = zk3[1];
}

/* BLINE: un segmento del circulo. X = parte baja y T = parte alta del desplazamiento en y (acarreo limpio); devuelve CNT */
static u8 bline(u8 x, u8 t)
{
    unsigned s;
    u8 y, tmp;
    s = (unsigned)x + zk4[0]; k6[2] = (u8)s;
    k6[3] = (u8)(zk4[1] + t + (s >> 8));
    if (circ_flag == 0) goto bl1;
    circ_flag++;                                   /* INC FLAG ($FF -> 0) */
bl5:
    y = circ_lsp;
    if (circ_lsy2[(u8)(y - 1)] == 255) goto bl7;
    circ_lsy2[y] = 255;
    circ_lsp++;
    if (circ_lsp != 0) goto bl7;
bl1:
    X1 = k5[0]; Y1 = k5[1]; X2 = k5[2]; Y2 = k5[3];
    zx15[4] = k6[0]; zx15[5] = k6[1]; zx12[0] = k6[2]; zx12[1] = k6[3];
    if (ship_clip()) goto bl5;                     /* JSR LL145; BCS BL5 */
    if (zswap != 0) {
        tmp = X1; X1 = X2; X2 = tmp;
        tmp = Y1; Y1 = Y2; Y2 = tmp;
    }
    y = circ_lsp;
    if (circ_lsy2[(u8)(y - 1)] != 255) goto bl8;
    circ_lsx2[y] = X1; circ_lsy2[y] = Y1; y++;
bl8:
    circ_lsx2[y] = X2; circ_lsy2[y] = Y2; y++;
    circ_lsp = y;
    loin();
    if (zxx13 != 0) goto bl5;
bl7:
    k5[0] = k6[0]; k5[1] = k6[1]; k5[2] = k6[2]; k5[3] = k6[3];
    circ_cnt = (u8)(circ_cnt + circ_stp);
    return circ_cnt;
}

/* CIRCLE2 */
u8 circle2_draw(void)
{
    u8 a, x, t, c;
    unsigned s;
    circ_flag = 255; circ_cnt = 0;
    for (;;) {
        a = em_fmltu2(circ_cnt);                   /* LDA CNT; JSR FMLTU2 */
        t = 0; x = circ_cnt;
        if (x < 33) { c = 0; }
        else {
            s = (unsigned)(u8)(a ^ 0xFF) + 0 + 1;  /* EOR #$FF; ADC #0 con C = 1 */
            x = (u8)s; c = (u8)(s >> 8);
            s = 0xFFu + 0 + c; t = (u8)s;          /* LDA #$FF; ADC #0; STA T */
            a = x; c = 0;                          /* TXA; CLC */
        }
        s = (unsigned)a + zk3[0] + c;              /* PL37: ADC K3 */
        k6[0] = (u8)s;
        k6[1] = (u8)(zk3[1] + t + (s >> 8));
        a = (u8)(circ_cnt + 16);                   /* LDA CNT; CLC; ADC #16 */
        a = em_fmltu2(a);
        c = em_carry;                              /* el acarreo con que FMLTU2 vuelve */
        x = a; t = 0;
        s = (unsigned)circ_cnt + 15 + c;           /* LDA CNT; ADC #15 */
        a = (u8)(s & 63);
        if (a >= 33) {                             /* CMP #33; BCC PL38 */
            s = (unsigned)(u8)(x ^ 0xFF) + 0 + 1;  /* TXA; EOR #$FF; ADC #0 con C = 1 */
            x = (u8)s; c = (u8)(s >> 8);
            s = 0xFFu + 0 + c; t = (u8)s;
        }
        a = bline(x, t);
        if (a >= 65) return 0;                     /* CMP #65; BCS: CLC; RTS */
    }
}

/* CIRCLE */
u8 circle_draw(void)
{
    u8 x, a;
    if (sun_chkon()) return 1;
    circ_lsx2[0] = 0;
    x = zK[0]; a = 8;
    if (x >= 8) {
        a >>= 1;
        if (x >= 60) a >>= 1;
    }
    circ_stp = a;
    return circle2_draw();
}

/* WPLS2: borra el circulo anterior volviendo a dibujar sus segmentos (XOR) */
void circle_wpls2(void)
{
    u8 y = circ_lsx2[0], a;
    if (y != 0) goto wp1;
wpl1:
    if (y >= circ_lsp) goto wp1;
    a = circ_lsy2[y];
    if (a == 255) goto wp2;
    Y2 = a; X2 = circ_lsx2[y];
    loin();
    y++;
    if (zswap != 0) goto wpl1;
    X1 = X2; Y1 = Y2;
    goto wpl1;
wp2:
    y++;
    X1 = circ_lsx2[y]; Y1 = circ_lsy2[y];
    y++;
    goto wpl1;
wp1:
    circ_lsp = 1;
    circ_lsx2[0] = 255;
}
