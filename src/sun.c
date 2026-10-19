/* SUN, CIRCLE y apoyo: transcripcion de la fuente de referencia (Elite Apple II) con sus acarreos y saltos.
   Diferencias: HLOIN/LOIN son las primitivas XOR de gfx.c (el original enmascara a pixeles pares por el modo Apple II). */
#include "sun.h"
#include "state.h"
#include "emath.h"
#include "ship3d.h"
#include "lines.h"

u8 sun_col = C_WHITE;
u8 sun_lso[256];
u8 sun_lsx = 255;
u8 circ_lsx2[256];
u8 circ_lsy2[256] = { 255 };               /* [0] = $FF: centinela; sin el, WPLS2 dibujaria una linea basura al empezar */
u8 circ_lsp = 1, circ_flag, circ_stp, circ_cnt;

#define X1 zx15[0]
#define Y1 zx15[1]
#define X2 zx15[2]
#define Y2 zx15[3]

static u8 sunx[2];                       /* SUNX (16 bits) */
static u8 yy[2];                         /* YY */
static u8 chk_p1, chk_p2;                /* P+1, P+2 (los pone CHKON) */
static u8 k5[4], k6[4];                  /* K5, K6 */
static void sn_erase(void);              /* sol y elipses de la vista nativa (mas abajo) */
static void ell_params(int *cx, int *cy, int *rx, int *ry);
static void ell_erase(void);

/* Transformacion de dibujo de las cartas (solo en pantalla; las rutinas siguen trabajando en las coordenadas del original): la carta
   local se aprieta a 9/10 en x y se estira 6/5 en y y se centra (los puntos del QL son mas anchos que altos: asi los circulos
   salen redondos en un monitor 4:3); la galactica baja 8 puntos. */
static int xf_mode = 0;

void chart_xf_set(int mode) { xf_mode = mode; }
int chart_xf_mode(void) { return xf_mode; }

int sun_xf_x(int x)
{
    if (xf_mode == 1) { x = 128 + ((x - 105) * 9) / 10; if (x < 0) x = 0; if (x > 255) x = 255; }
    return x;
}

int sun_xf_y(int y)
{
    if (xf_mode == 1) { y = 90 + ((y - 75) * 6) / 5; if (y < 0) y = 0; if (y > 255) y = 255; }
    else if (xf_mode == 3) { y = 80 + ((y - 68) * 6) / 5; if (y < 0) y = 0; if (y > 255) y = 255; }   /* vista nativa: centro 80, x 6/5 */
    else if (xf_mode == 2) { y = 27 + ((y - 19) * 6) / 5; if (y < 0) y = 0; if (y > 255) y = 255; }   /* galactica: baja 8 y se estira 6/5 (aspecto de los puntos) */
    return y;
}

/* HLOIN con la transformacion: la fila y cubre las filas de la imagen entre y y y + 1 (sin huecos al estirar) */
void sun_xf_hloin(int x1, int x2, int y, u8 col)
{
    int a, b, r;
    if (!xf_mode) { gfx_hloin(x1, x2, y, col); return; }
    x1 = sun_xf_x(x1); x2 = sun_xf_x(x2);
    a = sun_xf_y(y); b = sun_xf_y(y + 1);
    if (b <= a) b = a + 1;
    for (r = a; r < b; r++) gfx_hloin(x1, x2, r, col);
}

void sun_xf_vloin(int x, int y1, int y2, u8 col)
{
    int t;
    if (!xf_mode) { gfx_vloin(x, y1, y2, col); return; }
    if (y1 > y2) { t = y1; y1 = y2; y2 = t; }
    t = sun_xf_x(x);                                /* palo de 1 punto de ancho (el CPIX del original es de 2) */
    y1 = sun_xf_y(y1);
    y2 = (sun_xf_y(y2 + 1) > sun_xf_y(y2) ? sun_xf_y(y2 + 1) - 1 : sun_xf_y(y2));
    gfx_vline_xor(t, y1, y2, col);
}

/* Planeta de la carta corta: elipse rellena en coordenadas de pantalla, a 1 punto de resolucion (el SUN del original rellena
   con HLOIN de pares de puntos y la transformacion lo deforma). Radio original k; los puntos del QL son 4/3 mas anchos que altos */
void sun_xf_disc(int cx, int cy, int k, u8 col)
{
    int rx = k, ry, dy, w, r;
    long t, big;
    if (rx < 1) rx = 1;
    ry = (rx * 4 + 1) / 3;
    big = 2L * ry + 1;                                     /* alto del disco entre bordes (en medios puntos) */
    cx = sun_xf_x(cx); cy = sun_xf_y(cy);
    for (dy = -ry; dy <= ry; dy++) {
        t = (long)rx * rx * (big * big - 4L * dy * dy) * 4 / (big * big);   /* (2 * semiancho)^2 */
        r = 0;
        while ((long)(r + 1) * (r + 1) <= t) r++;          /* raiz entera */
        w = (r + 1) / 2;                                   /* semiancho redondeado */
        gfx_hline_xor(cx - w, cx + w, cy + dy, col);
    }
}

void sun_xf_dust(int x, int y, u8 zz)
{
    if (xf_mode == 2) { gfx_xor_pixel(sun_xf_x(x), sun_xf_y(y), C_WHITE); return; }   /* galactica: sistemas de un solo punto */
    gfx_dust_6502(sun_xf_x(x), sun_xf_y(y), zz);
}

/* Vista nativa de vuelo: el planeta, el sol y los circulos del tunel se calculan como en el original (centro 68, 136 lineas, recorte
   en 133 para que el 6/5 no pase de la linea 159) y se dibujan con la transformacion 3 (filas replicadas / extremos de linea). */
int sun_yx2m1 = 135;
static int vdepth = 0, v_cy, v_h, v_ys, v_yx, v_xf;

void view_virtual_begin(void)
{
    if (!view_native) return;
    if (vdepth++ == 0) {
        v_cy = view_cy; v_h = clip_h; v_ys = view_yscale; v_yx = sun_yx2m1; v_xf = xf_mode;
        view_cy = 68; clip_h = 134; view_yscale = 0; sun_yx2m1 = 133; xf_mode = 3;
    }
}

void view_virtual_end(void)
{
    if (!view_native) return;
    if (--vdepth == 0) {
        view_cy = v_cy; clip_h = v_h; view_yscale = v_ys; sun_yx2m1 = v_yx; xf_mode = v_xf;
    }
}

void (*sun_loin_hook)(void) = 0;                /* pruebas: cada LOIN, con X1..Y2 ya ordenados y SWAP (zswap) */
void (*sun_hloin_hook)(u8 l, u8 r, u8 y) = 0;   /* pruebas: cada HLOIN que dibuja (izquierda, derecha, fila) */

/* HLOIN del original con sus efectos en memoria: X1 y X2 se redondean a pares; si X2 < X1, X2 = X1 (STX X2); si coinciden no
   dibuja (HL6). Dibuja los pares de puntos de l a r - 1 (gfx_hloin) */
static void hloin(void)
{
    u8 l = (u8)(X1 & 0xFE), r = (u8)(X2 & 0xFE), t;
    X1 = l; X2 = r;
    if (l == r) return;
    if (r < l) { X2 = l; t = l; l = r; r = t; }
    if (sun_hloin_hook) sun_hloin_hook(l, r, Y1);
    sun_xf_hloin(l, r, Y1, sun_col);
}

/* LOIN del original con sus efectos en memoria (partes 1 y comienzo de STPX/STPY): S = 128, P = |dx|, Q = |dy|, SWAP = 0;
   si la linea es mas horizontal (Q < P) se ordena de izquierda a derecha y si no, de abajo arriba (Y1 >= Y2); cuando
   intercambia los extremos en X1..Y2 pone SWAP = $FF. WPLS2 depende de ello: con SWAP = 0 copia (X2, Y2) en (X1, Y1) y con
   SWAP != 0 da por hecho que LOIN ya los dejo ahi. Sin esto, un SWAP que quedara de LL145 (al recortar en el borde)
   hacia que WPLS2 trazara todas las lineas desde el mismo punto (fallo visto en el QL, 2026-10-17) */
static void loin(void)
{
    u8 dx, dy, t, sw, ox1 = X1, oy1 = Y1, ox2 = X2, oy2 = Y2;
    zS = 128;
    zswap = 0;
    dx = (u8)(X2 >= X1 ? X2 - X1 : X1 - X2);
    dy = (u8)(Y2 >= Y1 ? Y2 - Y1 : Y1 - Y2);
    zP = dx; zQ = dy;
    sw = (u8)((dy < dx) ? (X1 >= X2) : (Y1 < Y2));      /* STPX: CPX X2; BCC LI3  /  STPY: CPY Y2; BCS LI15 */
    if (sw) {
        zswap = 0xFF;
        t = X1; X1 = X2; X2 = t;
        t = Y1; Y1 = Y2; Y2 = t;
    }
    if (sun_loin_hook) sun_loin_hook();
    if (xf_mode) loin_draw((u8)sun_xf_x(ox1), (u8)sun_xf_y(oy1), (u8)sun_xf_x(ox2), (u8)sun_xf_y(oy2), sun_col);
    else loin_draw(ox1, oy1, ox2, oy2, sun_col);
}

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
static void sun_wpls_v(void)
{
    u8 y;
    if (sun_lsx & 0x80) return;                    /* LDA LSX; BMI WPLS-1 */
    yy[0] = sunx[0]; yy[1] = sunx[1];
    for (y = SUN_YX2M1; y != 0; y--)
        if (sun_lso[y] != 0) hloin2(sun_lso[y], y);
    sun_lsx = 255;                                 /* DEY: Y = 0 -> $FF; STY LSX */
}

void sun_wpls(void)
{
    if (view_native) { sn_erase(); return; }
    view_virtual_begin(); sun_wpls_v(); view_virtual_end();
}

/* ---- Sol de la vista nativa: relleno por filas con el borde irregular del original ----
   Cada fila de pantalla tiene su tramo encendido [lo, hi] (recortado a 0..255; lo > hi = vacia). En cada fotograma el tramo nuevo sale
   de los semianchos de la elipse (gfx_ellipse_rows, sin raices) mas un numero aleatorio (DORND AND CNT, con CNT = 7, 3, 1 o 0 segun
   el radio, como SUN) y solo se pintan en XOR las diferencias entre el tramo viejo y el nuevo de cada fila (los bordes que cambian):
   igual que el original, no se redibuja el sol entero cada vez. Borrar = pintar los tramos que hubiera. */
static u8 sn_lo[160], sn_hi[160];
static u8 sn_half[300];
static int sn_any = 0, sn_ready = 0;

static void sn_span(int y, int lo, int hi)
{
    if (lo > hi) return;
    gfx_hline_xor(lo, hi, y, sun_col);
}

static void sn_row(int y, int nl, int nh)           /* tramo nuevo (nl > nh: vacio): pinta la diferencia con el viejo y lo guarda */
{
    int ol = sn_lo[y], oh = sn_hi[y];
    int a, b;
    if (ol > oh) sn_span(y, nl, nh);
    else if (nl > nh) sn_span(y, ol, oh);
    else if (nh < ol || oh < nl) { sn_span(y, ol, oh); sn_span(y, nl, nh); }
    else {
        a = ol < nl ? ol : nl; b = ol < nl ? nl : ol;
        if (a < b) sn_span(y, a, b - 1);
        a = oh < nh ? oh : nh; b = oh < nh ? nh : oh;
        if (a < b) sn_span(y, a + 1, b);
    }
    if (nl > nh) { sn_lo[y] = 255; sn_hi[y] = 0; } else { sn_lo[y] = (u8)nl; sn_hi[y] = (u8)nh; }
}

static void sn_clear(void)
{
    int y;
    sn_ready = 1;
    for (y = 0; y < 160; y++) { sn_lo[y] = 255; sn_hi[y] = 0; }
    sn_any = 0;
}

static void sn_erase(void)
{
    int y;
    if (!sn_any) return;
    for (y = 0; y < 160; y++) if (sn_lo[y] <= sn_hi[y]) { sn_span(y, sn_lo[y], sn_hi[y]); sn_lo[y] = 255; sn_hi[y] = 0; }
    sn_any = 0;
}

int sun_rows_get(int y, int *lo, int *hi)              /* pruebas: el tramo que la pantalla deberia tener en la fila y */
{
    if (y < 0 || y > 159) return 0;
    *lo = sn_lo[y]; *hi = sn_hi[y];
    return sn_lo[y] <= sn_hi[y];
}

static u8 sn_draw(void)
{
    int cx, cy, rx, ry, y, top, bot, dy, w, lo, hi, cnt;
    sun_col = C_RED;
    if (!sn_ready) { sn_clear(); sn_ready = 1; }
    if (sun_chkon()) { sn_erase(); return 1; }          /* CHKON: fuera de la vista */
    ell_params(&cx, &cy, &rx, &ry);
    cnt = rx >= 96 ? 7 : (rx >= 40 ? 3 : (rx >= 16 ? 1 : 0));
    gfx_ellipse_rows(rx, ry, sn_half);
    top = cy - ry; bot = cy + ry;
    for (y = 0; y < 160; y++) {
        if (y < top || y > bot) { if (sn_lo[y] <= sn_hi[y]) sn_row(y, 1, 0); continue; }
        dy = y - cy; if (dy < 0) dy = -dy;
        em_rand_c = 0;
        w = sn_half[dy] + (int)(em_dornd() & cnt);
        lo = cx - w; hi = cx + w;
        if (hi < 0 || lo > 255) { sn_row(y, 1, 0); continue; }
        if (lo < 0) lo = 0;
        if (hi > 255) hi = 255;
        sn_row(y, lo, hi);
    }
    sn_any = 1;
    return 0;
}

/* SUN */
static void sun_draw_v(void)
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
    em_rand_c = em_carry;                          /* DORND recibe el acarreo con que vuelve LL5 */
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

void sun_draw(void)
{
    view_virtual_begin();
    if (view_native) sn_draw(); else sun_draw_v();
    view_virtual_end();
}

/* Vista nativa de vuelo: el planeta y los anillos del tunel son elipses de punto medio (gfx_ellipse_xor): semieje horizontal K y
   vertical 1,2 K, centro (K3, K4) del espacio del original llevado a la pantalla con view_ty. Sin montones de segmentos: borrar =
   volver a dibujar con los mismos datos (el planeta recuerda los del ultimo dibujo). */
#define NAT_BOT 159                              /* ultima linea de la vista nativa */
static int ell_valid = 0, ell_cx, ell_cy, ell_rx, ell_ry;
static u8 ell_col;

static void ell_params(int *cx, int *cy, int *rx, int *ry)
{
    long v = ((long)zk3[1] << 8) | zk3[0];
    long w = ((long)zk4[1] << 8) | zk4[0];
    if (zk3[1] & 0x80) v -= 65536L;
    if (zk4[1] & 0x80) w -= 65536L;
    *cx = (int)v;
    *cy = view_ty((int)w);
    *rx = zK[0];
    *ry = zK[0] + zK[0] / 5;
}

static void ell_erase(void)
{
    if (!ell_valid) return;
    gfx_ellipse_xor(ell_cx, ell_cy, ell_rx, ell_ry, ell_col, 0, 0, 255, NAT_BOT);
    ell_valid = 0;
}

/* PL9 de la vista nativa: borra el circulo anterior y dibuja el nuevo, salvo que no hayan cambiado ni el centro ni el radio ni el color
   (volar recto con el planeta lejos: nada que hacer). Si el circulo queda fuera de la vista solo borra. */
void circle_update(void)
{
    int cx, cy, rx, ry;
    view_virtual_begin();
    if (sun_chkon()) { ell_erase(); view_virtual_end(); return; }       /* CHKON: fuera de la vista */
    ell_params(&cx, &cy, &rx, &ry);
    if (ell_valid && cx == ell_cx && cy == ell_cy && rx == ell_rx && ry == ell_ry && sun_col == ell_col) { view_virtual_end(); return; }
    ell_erase();
    ell_cx = cx; ell_cy = cy; ell_rx = rx; ell_ry = ry; ell_col = sun_col;
    gfx_ellipse_xor(cx, cy, rx, ry, ell_col, 0, 0, 255, NAT_BOT);
    ell_valid = 1;
    view_virtual_end();
}

static u8 ell_planet(void)
{
    circle_update();
    return ell_valid ? 0 : 1;
}

/* Anillo del tunel: en el despegue (STP = 8) un rectangulo con la proporcion de la ventana de dibujo (256 x 160: semialto = 5/8 del
   semiancho); en el hiperespacio (STP = 4) una elipse. Los dos se borran volviendo a dibujarlos */
static u8 ell_ring(void)
{
    int cx, cy, rx, ry;
    ell_params(&cx, &cy, &rx, &ry);
    if (circ_stp >= 8) {
        int hh = (rx * 5) / 8;
        gfx_frame_xor(cx, cy, rx, hh < 1 ? 1 : hh, sun_col, 0, 0, 255, NAT_BOT);
    } else gfx_ellipse_xor(cx, cy, rx, ry, sun_col, 0, 0, 255, NAT_BOT);
    return 0;
}

/* CIRCLE2 */
static u8 circle2_draw_v(void)
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

u8 circle2_draw(void)
{
    u8 r;
    if (view_native) return ell_ring();                 /* anillos del tunel */
    view_virtual_begin(); r = circle2_draw_v(); view_virtual_end(); return r;
}

/* CIRCLE */
static u8 circle_draw_v(void)
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

u8 circle_draw(void)
{
    u8 r;
    view_virtual_begin();
    r = view_native ? ell_planet() : circle_draw_v();
    view_virtual_end();
    return r;
}

/* WPLS2: borra el circulo anterior volviendo a dibujar sus segmentos (XOR) */
static void circle_wpls2_v(void)
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
    circ_lsy2[0] = 255;
}

void circle_wpls2(void)
{
    if (view_native) { ell_erase(); return; }
    view_virtual_begin(); circle_wpls2_v(); view_virtual_end();
}

/* ---------------------------------------------------------------- tunel de hiperespacio / lanzamiento (HFS2, HFS1, HFL1) */
void (*sun_hiss_hook)(int y) = 0;                /* SOHISS(y): sonido de cada anillo (la pone el juego) */
void (*sun_blip_hook)(int x, int y) = 0;         /* SOBLIP(x, y) de LL164 */
void (*sun_tt66_hook)(void) = 0;                 /* TT66 de HFS2: borrar la vista */
void (*sun_circle_hook)(u8 k3, u8 k4, u8 k, u8 stp) = 0;   /* pruebas: cada CIRCLE2 del tunel */

/* HFL1: una serie de circulos concentricos cuyo radio empieza en (XX4 AND 7) + 8 y se duplica mientras sea < 160 */
static void hfl1(u8 xx4)
{
    zK[0] = (u8)((xx4 & 7) + 8);
    for (;;) {                                   /* HFL2 */
        em_rand_c = 0;                           /* acarreo de los DORND del sonido: 0 (CLC; ADC #8 en HFL1 o CMP #160; BCC HFL2) */
        if (sun_hiss_hook) sun_hiss_hook(10);    /* LDY #10; JSR SOHISS */
        circ_lsp = 1;
        if (sun_circle_hook) sun_circle_hook(zk3[0], zk4[0], zK[0], circ_stp);
        circle2_draw();
        if (zK[0] & 0x80) return;                /* ASL K; BCS HF8 */
        zK[0] = (u8)(zK[0] << 1);
        if (zK[0] >= 160) return;                /* CMP #160; BCC HFL2 */
    }
}

/* HFS2 sin la limpieza de pantalla: STP = paso del circulo (4 hiperespacio, 8 lanzamiento); centro (X, Y) = (128, 68) */
void tunnel_draw(u8 stp)
{
    u8 xx4;
    circ_stp = stp;
    zk3[0] = 128; zk3[1] = 0;                    /* HFS1: K3 = X, K4 = Y */
    zk4[0] = 68;  zk4[1] = 0;
    for (xx4 = 0; xx4 != 8; xx4++) hfl1(xx4);
}

/* HFS2: TT66 (borrar la vista) y los anillos */
static void hfs2(u8 stp)
{
    if (sun_tt66_hook) sun_tt66_hook();
    tunnel_draw(stp);
}

/* LAUN: dos SOHISS de 256 clics y el tunel de lanzamiento (STP = 8) */
void tunnel_laun(void)
{
    if (sun_hiss_hook) { sun_hiss_hook(0); sun_hiss_hook(0); }
    hfs2(8);
}

/* LL164: nueve SOBLIP con el periodo bajando de 255 a 175 y el tunel de hiperespacio (STP = 4). El primer SBC #10 lleva el
   acarreo de entrada (con C = 0 la serie es 255, 244, ..., 174); aqui siempre de 10 en 10 (solo cambia el tono) */
void tunnel_ll164(void)
{
    u8 x = 255;
    do { if (sun_blip_hook) sun_blip_hook(x, 90); x = (u8)(x - 10); } while (x >= 170);
    hfs2(4);
}

/* Pruebas: SUNX */
void sun_get_sunx(u8 *v) { v[0] = sunx[0]; v[1] = sunx[1]; }
void sun_set_sunx(u8 lo, u8 hi) { sunx[0] = lo; sunx[1] = hi; }

/* Tras borrar el area de vista: ni el sol ni el circulo del planeta figuran como dibujados */
void sun_forget(void)
{
    ell_valid = 0;                               /* la elipse del planeta (vista nativa) tampoco figura como dibujada */
    sn_clear();                                  /* ni los tramos del sol */
    sun_flflls();
    circ_lsp = 1;
    circ_lsx2[0] = 255;
    circ_lsy2[0] = 255;
}
