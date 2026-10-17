#include "stars.h"
#include "emath.h"
#include "state.h"
#include "space.h"

u8 stars_nostm = NOST;
u8 sx[NOST + 1], sy[NOST + 1], sz[NOST + 1];
u8 sxl[NOST + 1], syl[NOST + 1], szl[NOST + 1];
u8 delt4[2];

#define X1 zx15[0]
#define Y1 zx15[1]
#define YHALF 68                          /* Y: mitad de la altura de la vista (136 / 2) */

static u8 c;                              /* acarreo del 6502 */
static u8 xx[2], yy[2];                   /* XX, YY (16 bits, byte bajo primero) */
static u8 zz;                             /* ZZ */
static u8 rat, rat2, newzp;               /* RAT, RAT2 y newzp de STARS2 */

static u8 adcc(u8 a, u8 m) { unsigned s = (unsigned)a + m + c; c = (u8)(s >> 8); return (u8)s; }
static u8 sbcc(u8 a, u8 m) { int d = (int)a - m - (1 - c); c = (u8)(d >= 0); return (u8)d; }
static u8 lsrc(u8 v) { c = (u8)(v & 1); return (u8)(v >> 1); }
static u8 rorc(u8 v) { u8 n = (u8)((v >> 1) | (c << 7)); c = (u8)(v & 1); return n; }
static u8 rolc(u8 v) { u8 n = (u8)((v << 1) | c); c = (u8)(v >> 7); return n; }

/* DORND con el acarreo del 6502 como entrada y salida (la semilla usa el acarreo) */
static u8 dornd(void)
{
    u8 a;
    em_rand_c = c;
    a = em_dornd();
    c = em_rand_c;
    return a;
}

void stars_set_speed(u8 delta)
{
    u8 a = delta;
    delt4[0] = 0;
    a = lsrc(a); delt4[0] = rorc(delt4[0]);
    a = lsrc(a); delt4[0] = rorc(delt4[0]);
    delt4[1] = a;
}

/* PIXEL2: dibuja (XOR) la particula de X1, Y1 (signo-magnitud respecto al centro) con distancia ZZ */
static void pixel2(void)
{
    u8 a, x, t;
    a = X1;
    if (a & 0x80) { a = (u8)(a ^ 0x7F); a = (u8)(a + 1); }
    x = (u8)(a ^ 0x80);
    a = (u8)(Y1 & 0x7F);
    if (a >= YHALF) return;                /* CMP #Y; BCS PXR1 */
    a = Y1;
    c = 0;                                 /* el CMP deja el acarreo limpio */
    if (a & 0x80) { a = (u8)(a ^ 0x7F); a = adcc(a, 1); }
    t = a;
    a = sbcc(YHALF + 1, t);
    gfx_dust(x, a, zz);
}

/* MLS1 / MLS2 / MUT1 / MUT2 / MULTS-2: envoltorios de la multiplicacion con signo */
static u8 mls1(u8 a) { zP = alp1; return em_mults(a); }
static u8 mls2(u8 a) { zR = xx[0]; zS = xx[1]; zP = alp1; return em_mults(a); }

/* ---------------------------------------------------------------- vista frontal */
static void stars1(void)
{
    u8 y = stars_nostm, a, x;
    for (;;) {                                         /* STL1 */
        em_dv42(sz[y]);                                /* (P R) = DELTA / z */
        a = zR;
        zP = lsrc(zP); a = rorc(a);
        zP = lsrc(zP); a = rorc(a);
        a |= 1; zQ = a;
        a = sbcc(szl[y], delt4[0]); szl[y] = a;
        a = sz[y]; zz = a;
        a = sbcc(a, delt4[1]); sz[y] = a;
        a = em_mlu1(sy[y], &Y1);                       /* (A P) = |y| * Q; Y1 = y */
        c = 0;                                         /* MLU1 sale con C = 0 */
        yy[1] = a;
        a = zP; a = adcc(a, syl[y]); yy[0] = a; zR = a;
        a = Y1; a = adcc(a, yy[1]); yy[1] = a; zS = a;
        a = sx[y]; X1 = a;
        a = em_mlu2(a);                                /* (A P) = |x| * Q */
        c = 0;
        xx[1] = a;
        a = zP; a = adcc(a, sxl[y]); xx[0] = a;
        a = X1; a = adcc(a, xx[1]); xx[1] = a;
        a = (u8)(a ^ alp2[1]);
        a = mls1(a);
        a = em_add(a); x = em_x; yy[1] = a; yy[0] = x;
        a = (u8)(a ^ alp2[0]);
        a = mls2(a);
        a = em_add(a); x = em_x; xx[1] = a; xx[0] = x;
        zP = bet1;                                     /* LDX BET1; LDA YY+1; EOR BET2+1; JSR MULTS-2 */
        a = (u8)(yy[1] ^ bet2[1]);
        a = em_mults(a);
        zQ = a;
        zS = xx[1]; zR = xx[0];                        /* MUT2: S = XX+1, R = XX */
        a = em_mult1(a);                               /* (A P) = Q * A */
        c = (u8)(zP >> 7); zP = (u8)(zP << 1);         /* ASL P */
        a = rolc(a);                                   /* ROL A */
        zT = a;
        a = rorc(0);                                   /* LDA #0; ROR A */
        a = (u8)(a | zT);
        a = em_add(a); x = em_x; xx[1] = a; sxl[y] = x;
        zR = yy[0]; zS = yy[1]; zP = 0;
        a = (u8)(beta ^ 0x80);
        a = em_add(a); x = em_x; yy[1] = a; syl[y] = x;                    /* PIX1 */
        pixel2();                                      /* borra la particula vieja (X1, Y1, ZZ siguen siendo los viejos) */
        a = xx[1]; X1 = a; sx[y] = a;
        if ((u8)(a & 0x7F) >= 120) goto kill1;
        a = yy[1]; sy[y] = a; Y1 = a;
        if ((u8)(a & 0x7F) >= 120) goto kill1;
        a = sz[y];
        if (a < 16) goto kill1;
        zz = a;
stc1:
        pixel2();
        y--;
        if (y == 0) return;
        continue;
kill1:
        a = dornd(); a |= 4; Y1 = a; sy[y] = a;
        a = dornd(); a |= 8; X1 = a; sx[y] = a;
        a = dornd(); a |= 144; sz[y] = a; zz = a;
        goto stc1;
    }
}

/* ---------------------------------------------------------------- vista trasera */
static void stars6(void)
{
    u8 y = stars_nostm, a, x;
    for (;;) {                                         /* STL6 */
        em_dv42(sz[y]);
        a = zR;
        zP = lsrc(zP); a = rorc(a);
        zP = lsrc(zP); a = rorc(a);
        a |= 1; zQ = a;
        a = sx[y]; X1 = a;
        a = em_mlu2(a); c = 0;
        xx[1] = a;
        a = sbcc(sxl[y], zP); xx[0] = a;
        a = sbcc(X1, xx[1]); xx[1] = a;
        a = em_mlu1(sy[y], &Y1); c = 0;
        yy[1] = a;
        a = sbcc(syl[y], zP); yy[0] = a; zR = a;
        a = sbcc(Y1, yy[1]); yy[1] = a; zS = a;
        a = adcc(szl[y], delt4[0]); szl[y] = a;
        a = sz[y]; zz = a;
        a = adcc(a, delt4[1]); sz[y] = a;
        a = (u8)(xx[1] ^ alp2[0]);
        a = mls1(a);
        a = em_add(a); x = em_x; yy[1] = a; yy[0] = x;
        a = (u8)(a ^ alp2[1]);
        a = mls2(a);
        a = em_add(a); x = em_x; xx[1] = a; xx[0] = x;
        a = (u8)(yy[1] ^ bet2[1]);
        zP = bet1;
        a = em_mults(a);
        zQ = a;
        zS = xx[1]; zR = xx[0];                        /* MUT1: R = XX; S = XX+1 */
        a = (u8)(xx[1] ^ 0x80);
        a = em_mult1(a);
        c = (u8)(zP >> 7); zP = (u8)(zP << 1);
        a = rolc(a);
        zT = a;
        a = rorc(0);
        a = (u8)(a | zT);
        a = em_add(a); x = em_x; xx[1] = a; sxl[y] = x;
        zR = yy[0]; zS = yy[1]; zP = 0;
        a = em_add(beta); x = em_x; yy[1] = a; syl[y] = x;                 /* PIX1 con BETA sin invertir */
        pixel2();
        a = xx[1]; X1 = a; sx[y] = a;
        a = yy[1]; sy[y] = a; Y1 = a;
        if ((u8)(a & 0x7F) >= 110) goto kill6;
        a = sz[y];
        if (a >= 160) goto kill6;
        zz = a;
stc6:
        pixel2();
        y--;
        if (y == 0) return;
        continue;
kill6:
        a = dornd(); a &= 0x7F; a = adcc(a, 10); sz[y] = a; zz = a;
        a = lsrc(a);
        if (c) goto st4;
        a = lsrc(a);
        a = 252; a = rorc(a);
        X1 = a; sx[y] = a;
        a = dornd(); Y1 = a; sy[y] = a;
        goto stc6;
st4:
        a = dornd(); X1 = a; sx[y] = a;
        a = lsrc(a);
        a = 230; a = rorc(a);
        Y1 = a; sy[y] = a;
        goto stc6;
    }
}

/* ST2: invierte los signos de ALPHA, ALP2 y BET2 segun RAT (se llama al entrar y al salir de STARS2) */
static void st2(void)
{
    alpha = (u8)(alpha ^ rat);
    alp2[0] = (u8)(alp2[0] ^ rat);
    alp2[1] = (u8)(alp2[0] ^ 0x80);
    bet2[0] = (u8)(bet2[0] ^ rat);
    bet2[1] = (u8)(bet2[0] ^ 0x80);
}

/* ---------------------------------------------------------------- vistas izquierda y derecha */
static void stars2(u8 xreg)
{
    u8 y, a, x;
    a = 0; c = (u8)(xreg >= 2);                        /* LDA #0; CPX #2; ROR A */
    a = rorc(a);
    rat = a; rat2 = (u8)(a ^ 0x80);
    st2();
    y = stars_nostm;
    for (;;) {                                         /* STL2 */
        a = sz[y]; zz = a;
        a = lsrc(a); a = lsrc(a); a = lsrc(a);
        em_dv41(a);                                    /* (P R) = DELTA / (z / 8) */
        a = zP; newzp = a;
        zS = (u8)(a ^ rat2);
        zP = sxl[y];
        a = sx[y]; X1 = a;
        a = em_add(a); x = em_x;
        zS = a; zR = x;
        a = sy[y]; Y1 = a;
        a = (u8)(a ^ bet2[0]);
        zP = bet1;
        a = em_mults(a);
        a = em_add(a); x = em_x;
        xx[0] = x; xx[1] = a;
        zR = syl[y]; zS = Y1;
        zP = bet1;
        a = (u8)(a ^ bet2[1]);
        a = em_mults(a);
        a = em_add(a); x = em_x;
        yy[0] = x; yy[1] = a;
        zP = alp1;
        a = (u8)(a ^ alp2[0]);
        a = em_mults(a);
        zQ = a;
        zR = xx[0]; zS = xx[1];
        a = (u8)(xx[1] ^ 0x80);
        a = em_mad(a); x = em_x;
        xx[1] = a; sxl[y] = x;
        zR = yy[0]; zS = yy[1];
        a = em_mad(yy[1]); x = em_x;
        zS = a; zR = x;
        zP = 0;
        a = em_add(alpha); x = em_x;                   /* PIX1 */
        yy[1] = a; syl[y] = x;
        pixel2();
        a = xx[1]; X1 = a; sx[y] = a;
        a = (u8)((a & 0x7F) ^ 0x7F);
        if (a <= newzp) goto kill2;
        a = yy[1]; sy[y] = a; Y1 = a;
        a = (u8)(a & 0x7F);
        if (a >= 116) goto st5;
stc2:
        pixel2();
        y--;
        if (y == 0) { st2(); return; }
        continue;
kill2:
        a = dornd(); Y1 = a; sy[y] = a;
        a = (u8)(115 | rat); X1 = a; sx[y] = a;
        goto stf1;
st5:
        a = dornd(); X1 = a; sx[y] = a;
        a = (u8)(110 | alp2[1]); Y1 = a; sy[y] = a;
stf1:
        a = dornd(); a |= 8; zz = a; sz[y] = a;
        goto stc2;
    }
}

void stars_update(u8 view)
{
    if (view == 0) stars1();
    else if (view == 1) stars6();
    else stars2((u8)(view - 1));                       /* DEX; BNE ST11 -> STARS2 con X = VIEW - 1 */
}

void stars_new(void)
{
    u8 y = stars_nostm, a;
    do {                                               /* SAL4 */
        a = dornd(); a |= 8; sz[y] = a; zz = a;
        a = dornd(); sx[y] = a; X1 = a;
        a = dornd(); sy[y] = a; Y1 = a;
        pixel2();
        y--;
    } while (y != 0);
}

void stars_flip(void)
{
    u8 y = stars_nostm, a, x;
    do {                                               /* FLL1 */
        x = sy[y]; a = sx[y];
        Y1 = a; sy[y] = a;
        X1 = x; sx[y] = x;
        zz = sz[y];
        pixel2();
        y--;
    } while (y != 0);
}

/* Ayuda de pruebas: dibuja (XOR) todas las particulas en sus posiciones actuales, sin moverlas */
void stars_draw_all(void)
{
    u8 y = stars_nostm;
    do {
        X1 = sx[y]; Y1 = sy[y]; zz = sz[y];
        pixel2();
        y--;
    } while (y != 0);
}
