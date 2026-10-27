/* SUN, CIRCLE and support: transcription of the reference source (Elite Apple II) with its carries and jumps.
   Differences: HLOIN/LOIN are the XOR primitives of gfx.c (the original masks to even pixels because of the Apple II mode). */
#include "sun.h"
#include "state.h"
#include "emath.h"
#include "ship3d.h"
#include "lines.h"
#include "platform.h"
#include "sne_table.h"

u8 sun_col = C_WHITE;
u8 sun_lso[256];
u8 sun_lsx = 255;
u8 circ_lsx2[256];
u8 circ_lsy2[256] = { 255 };               /* [0] = $FF: sentinel; without it, WPLS2 would draw a garbage line at the start */
u8 circ_lsp = 1, circ_flag, circ_stp, circ_cnt;

#define X1 zx15[0]
#define Y1 zx15[1]
#define X2 zx15[2]
#define Y2 zx15[3]

static u8 sunx[2];                       /* SUNX (16 bits) */
static u8 yy[2];                         /* YY */
static u8 chk_p1, chk_p2;                /* P+1, P+2 (set by CHKON) */
static u8 k5[4], k6[4];                  /* K5, K6 */
static void sn_erase(void);              /* sun and ellipses of the native view (further down) */
static void ell_params(int *cx, int *cy, int *rx, int *ry);
static void ell_erase(void);

/* Drawing transformation of the charts (screen only; the routines keep working in the original's coordinates): the local
   chart is squeezed to 9/10 in x and stretched 27/25 in y (height / width = 1.2, like the flight view) and centred (QL dots are wider than they are tall: so the circles
   come out round on a 4:3 monitor); the galactic one moves down 8 dots. */
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
    if (xf_mode == 1) { y = 90 + ((y - 75) * 27) / 25; if (y < 0) y = 0; if (y > 255) y = 255; }   /* local: 1.08 in y with 0.9 in x = 1.2 (like the planet) */
    else if (xf_mode == 3) { y = 80 + ((y - 68) * 6) / 5; if (y < 0) y = 0; if (y > 255) y = 255; }   /* native view: centre 80, x 6/5 */
    else if (xf_mode == 2) { y = 27 + ((y - 19) * 6) / 5; if (y < 0) y = 0; if (y > 255) y = 255; }   /* galactic: moves down 8 and is stretched 6/5 (dot aspect) */
    return y;
}

/* HLOIN with the transformation: row y covers the image rows between y and y + 1 (no gaps when stretching) */
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
    t = sun_xf_x(x);                                /* 1-dot-wide stick (the original's CPIX is 2) */
    y1 = sun_xf_y(y1);
    y2 = (sun_xf_y(y2 + 1) > sun_xf_y(y2) ? sun_xf_y(y2 + 1) - 1 : sun_xf_y(y2));
    gfx_vline_xor(t, y1, y2, col);
}

/* Circles of the charts with the transformation active: the same midpoint ellipse as the flight view planet, with the
   aspect of each chart (local: x 9/10 and y 27/25; galactic: y 6/5; in both, height / width = 1.2), instead of the CIRCLE2 segments deformed by the transformation.
   The centre (cx, cy) and the radius k are in the original's coordinates. */
static void xf_radii(int k, int *rx, int *ry)
{
    *rx = (xf_mode == 1) ? (k * 9 + 5) / 10 : k;
    *ry = (xf_mode == 1) ? (k * 27 + 12) / 25 : k + k / 5;      /* height / width = 1.2, the planet's aspect in flight */
    if (*rx < 1) *rx = 1;
    if (*ry < 1) *ry = 1;
}

static int xf_bottom(void) { return xf_mode == 1 ? 167 : 151; }   /* without overwriting the text below (rows 21 and 19) */

void sun_xf_circle(int cx, int cy, int k, u8 col)               /* outline (range circle) */
{
    int rx, ry;
    xf_radii(k, &rx, &ry);
    gfx_ellipse_xor(sun_xf_x(cx), sun_xf_y(cy), rx, ry, col, 0, 20, 255, xf_bottom());
}

/* Short chart planet: ellipse filled with the per-row half-widths of gfx_ellipse_rows (the same as the flight view sun) */
void sun_xf_disc(int cx, int cy, int k, u8 col)
{
    static u8 half[64];
    int rx, ry, dy, y;
    xf_radii(k, &rx, &ry);
    if (ry > 63) ry = 63;
    gfx_ellipse_rows(rx, ry, half);
    cx = sun_xf_x(cx); cy = sun_xf_y(cy);
    for (dy = -ry; dy <= ry; dy++) {
        y = cy + dy;
        if (y < 20 || y > xf_bottom()) continue;
        gfx_hline_xor(cx - half[dy < 0 ? -dy : dy], cx + half[dy < 0 ? -dy : dy], y, col);
    }
}

void sun_xf_dust(int x, int y, u8 zz)
{
    if (xf_mode == 2) { gfx_xor_pixel(sun_xf_x(x), sun_xf_y(y), C_WHITE); return; }   /* galactic: single-dot systems */
    gfx_dust_6502(sun_xf_x(x), sun_xf_y(y), zz);
}

/* Native flight view: the planet, the sun and the tunnel circles are calculated as in the original (centre 68, 136 lines, clipping
   at 133 so that the 6/5 does not go past line 159) and drawn with transformation 3 (replicated rows / line ends). */
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

void (*sun_loin_hook)(void) = 0;                /* tests: each LOIN, with X1..Y2 already sorted and SWAP (zswap) */
void (*sun_hloin_hook)(u8 l, u8 r, u8 y) = 0;   /* tests: each HLOIN that draws (left, right, row) */

/* HLOIN of the original with its effects in memory: X1 and X2 are rounded to even; if X2 < X1, X2 = X1 (STX X2); if they coincide it
   does not draw (HL6). Draws the dot pairs from l to r - 1 (gfx_hloin) */
static void hloin(void)
{
    u8 l = (u8)(X1 & 0xFE), r = (u8)(X2 & 0xFE), t;
    X1 = l; X2 = r;
    if (l == r) return;
    if (r < l) { X2 = l; t = l; l = r; r = t; }
    if (sun_hloin_hook) sun_hloin_hook(l, r, Y1);
    sun_xf_hloin(l, r, Y1, sun_col);
}

/* LOIN of the original with its effects in memory (parts 1 and the start of STPX/STPY): S = 128, P = |dx|, Q = |dy|, SWAP = 0;
   if the line is more horizontal (Q < P) it is sorted left to right and otherwise bottom to top (Y1 >= Y2); when it
   swaps the endpoints in X1..Y2 it sets SWAP = $FF. WPLS2 depends on this: with SWAP = 0 it copies (X2, Y2) into (X1, Y1) and with
   SWAP != 0 it assumes LOIN already left them there. Without this, a SWAP left over from LL145 (when clipping at the edge)
   made WPLS2 draw all the lines from the same point (bug seen on the QL, 2026-10-17) */
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
    sun_lsx = 255;                                 /* DEY: Y goes from 0 to $FF; STY LSX */
}

/* CHKON: carry 1 if the circle is not in the view; also P+1 = K4+K (low) and P+2 (high) for SUN */
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

/* EDGES: X1 = YY - A, X2 = YY + A clipped to 0..255; returns 1 (and LSO[y] = 0) if it falls outside */
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

/* HLOIN2: EDGES, Y1 = y, LSO[y] = 0, HLOIN (YY already has the centre) */
static void hloin2(u8 a, u8 y)
{
    edges(a, y);
    Y1 = y;
    sun_lso[y] = 0;
    hloin();
}

/* WPLS: erases the previous sun (if LSX is not negative) */
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

/* ---- Native view sun: row-by-row fill with the original's ragged edge ----
   Each screen row has its lit span [lo, hi] (clipped to 0..255; lo > hi = empty). On each frame the new span comes
   from the ellipse half-widths (gfx_ellipse_rows, no roots) plus a random number (DORND AND CNT, with CNT = 7, 3, 1 or 0 depending on
   the radius, like SUN) and only the differences between the old and the new span of each row (the edges that change) are painted in XOR:
   like the original, the whole sun is not redrawn each time. Erasing = painting the spans that exist. */
static u8 sn_lo[160], sn_hi[160];
static u8 sn_half[300];
static int sn_any = 0, sn_ready = 0;

static void sn_span(int y, int lo, int hi)
{
    if (lo > hi) return;
    gfx_hline_xor(lo, hi, y, sun_col);
}

static void sn_row(int y, int nl, int nh)           /* new span (nl > nh: empty): paints the difference from the old one and stores it */
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

int sun_rows_get(int y, int *lo, int *hi)              /* tests: the span the screen should have on row y */
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
    if (sun_chkon()) { sn_erase(); return 1; }          /* CHKON: outside the view */
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
    em_rand_c = em_carry;                          /* DORND receives the carry LL5 returns with */
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
    if (!edges(a, y)) goto plf16;                  /* A = new width; BCC PLF16 */
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

/* BLINE: a circle segment. X = low part and T = high part of the y offset (carry clear); returns CNT */
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

/* Native flight view: the planet and the tunnel rings are midpoint ellipses (gfx_ellipse_xor): horizontal semi-axis K and
   vertical 1.2 K, centre (K3, K4) of the original's space brought to the screen with view_ty. No segment heaps: erasing =
   drawing again with the same data (the planet remembers those of the last drawing). */
#define NAT_BOT 159                              /* last line of the native view */
#define ELL_TAB 608                              /* spans per row (2 bytes): ry <= (6 * 248 + 2) / 5 = 298 */
static int ell_valid = 0, ell_cx, ell_cy, ell_rx, ell_ry;
static u8 ell_col;
static unsigned short ell_tabw[2][ELL_TAB];      /* span table of the old and the new outline (alternating; only recalculated if the radius changes): bytes in mode 8, words in mode 4 (at the 512 dots) */
#define ELL_T(i) ((u8 *)ell_tabw[i])
static int ell_cur = 0;                          /* table of the drawn outline (ELL_T(ell_cur)) */

static void ell_params(int *cx, int *cy, int *rx, int *ry)
{
    long v = ((long)zk3[1] << 8) | zk3[0];
    long w = ((long)zk4[1] << 8) | zk4[0];
    if (zk3[1] & 0x80) v -= 65536L;
    if (zk4[1] & 0x80) w -= 65536L;
    *cx = (int)v;
    *cy = view_ty((int)w);
    *rx = zK[0];
    *ry = (6 * zK[0] + 2) / 5;                   /* height / width = 1.2 (the monitor aspect), rounded */
}


/* ---- Meridians and crater (PL9 parts 2 and 3, PLS1..PLS5 and PLS22 of Elite-A; the Apple II only has the circle) ----
   With the whole planet on screen (K < 256) and radius >= 6, type 128 carries two semi-ellipses (meridians at 90 degrees to each other, the
   half that faces us) and type 130 a crater (full displaced ellipse, only if roofv_z >= 0). Each figure is a centre and two
   screen vectors u, w: P(t) = c + u cos t + w sin t (t in 1/64 of a turn, and y inverted). They are polygonal lines of 64/STP segments
   (STP = 2, 4 or 8 depending on the radius) drawn in XOR with clipping; erasing repeats exactly the same segments. When the frame
   changes they are replaced segment by segment (identical ones are left alone), so the whole figure does not disappear. Each segment does not
   include its end point (the next one sets it), because in XOR the shared vertex would be erased. */
#define ARC_PTS 33
static int arc_n[2];                                /* points of each polygonal line drawn (0 = none) */
static int arc_x[2][ARC_PTS], arc_y[2][ARC_PTS];
static u8 arc_col;

static int arc_clip(int *x1, int *y1, int *x2, int *y2)   /* Cohen-Sutherland to 0..255 x 0..NAT_BOT; 0 if nothing is visible */
{
    int c1, c2, c, x = 0, y = 0;
#define ARC_CODE(px, py) (((px) < 0) | (((px) > 255) << 1) | (((py) < 0) << 2) | (((py) > NAT_BOT) << 3))
    c1 = ARC_CODE(*x1, *y1); c2 = ARC_CODE(*x2, *y2);
    for (;;) {
        if (!(c1 | c2)) return 1;
        if (c1 & c2) return 0;
        c = c1 ? c1 : c2;
        if (c & 1)      { y = *y1 + (int)((long)(*y2 - *y1) * (0 - *x1) / (*x2 - *x1)); x = 0; }
        else if (c & 2) { y = *y1 + (int)((long)(*y2 - *y1) * (255 - *x1) / (*x2 - *x1)); x = 255; }
        else if (c & 4) { x = *x1 + (int)((long)(*x2 - *x1) * (0 - *y1) / (*y2 - *y1)); y = 0; }
        else            { x = *x1 + (int)((long)(*x2 - *x1) * (NAT_BOT - *y1) / (*y2 - *y1)); y = NAT_BOT; }
        if (c == c1) { *x1 = x; *y1 = y; c1 = ARC_CODE(x, y); }
        else         { *x2 = x; *y2 = y; c2 = ARC_CODE(x, y); }
    }
}

static void arc_seg(int x1, int y1, int x2, int y2)       /* a segment in XOR without its end point */
{
    int ex = x2, ey = y2;
    if (arc_clip(&x1, &y1, &x2, &y2)) {
        loin_draw((u8)x1, (u8)y1, (u8)x2, (u8)y2, arc_col);
        if (x2 == ex && y2 == ey) gfx_xor_pixel(x2, y2, arc_col);   /* the end only if it was not clipped (otherwise it is the edge) */
    }
}

static void arc_poly_erase(int k)
{
    int i;
    for (i = 0; i + 1 < arc_n[k]; i++) arc_seg(arc_x[k][i], arc_y[k][i], arc_x[k][i + 1], arc_y[k][i + 1]);
    arc_n[k] = 0;
}

static void arcs_erase(void)
{
    arc_poly_erase(0);
    arc_poly_erase(1);
}

static long arc_sin(int i)                          /* sin(i / 64 turns) * 256 (maximum 255) */
{
    i &= 63;
    return i < 32 ? (long)em_sne[i] : -(long)em_sne[i - 32];
}

static long arc_fdiv(long a, long b)                /* round-down division (b > 0) */
{
    return a >= 0 ? a / b : -((-a + b - 1) / b);
}

static long arc_comp(int off)                       /* INWK component (sign and magnitude) from -24576 to 24576 */
{
    long v = ((long)(inwk[off + 1] & 0x7F) << 8) | inwk[off];
    return (inwk[off + 1] & 0x80) ? -v : v;
}

static int arc_atan(long a, long b)                 /* angle (1/64 turn) of the vector (a, b) = ARCTAN + the quadrants of PLS4 */
{
    static const u8 tab[33] = { 0,0,1,1,1,2,2,2,2,3,3,3,4,4,4,4,5,5,5,5,6,6,6,6,7,7,7,7,7,7,8,8,8 };
    long ax = a < 0 ? -a : a, ay = b < 0 ? -b : b;
    int t;
    if (ax == 0 && ay == 0) return 0;
    if (ay <= ax) t = tab[(ay * 32 + ax / 2) / ax];
    else t = 16 - tab[(ax * 32 + ay / 2) / ay];
    if (a >= 0) return b >= 0 ? t : (64 - t) & 63;
    return b >= 0 ? 32 - t : 32 + t;
}

/* Polygonal line k of the figure (centre cx, cy; vectors in 1/16 dot: ux, uy, wx, wy), from t0 with n segments of step stp. It only CALCULATES the new
   points (arc_nx/arc_ny); arc_commit draws them. This way the maths goes before changing the ellipse and between the planet change and that of
   its meridians only the line drawing remains */
static int arc_nx[2][ARC_PTS], arc_ny[2][ARC_PTS], arc_pn[2];
static void arc_poly(int k, int cx, int cy, long ux, long uy, long wx, long wy, int t0, int n, int stp)
{
    int i;
    for (i = 0; i <= n; i++) {
        long s = arc_sin(t0 + i * stp), c = arc_sin(t0 + i * stp + 16);
        arc_nx[k][i] = cx + (int)arc_fdiv(ux * c + wx * s + 2048, 4096);
        arc_ny[k][i] = cy - (int)arc_fdiv(uy * c + wy * s + 2048, 4096);
    }
    arc_pn[k] = n + 1;
}

static void arc_commit_poly(int k)
{
    int n = arc_pn[k] - 1, i, same;
    if (n < 0) { arc_poly_erase(k); return; }
    same = (arc_n[k] == n + 1);
    if (!same) arc_poly_erase(k);
    for (i = 0; i < n; i++) {
        if (same && arc_nx[k][i] == arc_x[k][i] && arc_ny[k][i] == arc_y[k][i] && arc_nx[k][i + 1] == arc_x[k][i + 1] && arc_ny[k][i + 1] == arc_y[k][i + 1]) continue;
        if (same) arc_seg(arc_x[k][i], arc_y[k][i], arc_x[k][i + 1], arc_y[k][i + 1]);     /* erases the old one */
        arc_seg(arc_nx[k][i], arc_ny[k][i], arc_nx[k][i + 1], arc_ny[k][i + 1]);            /* and draws the new one */
    }
    for (i = 0; i <= n; i++) { arc_x[k][i] = arc_nx[k][i]; arc_y[k][i] = arc_ny[k][i]; }
    arc_n[k] = n + 1;
}

static void arc_commit(void)
{
    if ((arc_n[0] | arc_n[1]) && arc_col != sun_col) arcs_erase();
    arc_col = sun_col;
    arc_commit_poly(0);
    arc_commit_poly(1);
}

/* Calculates the figures for the planet circle (centre cx, cy on screen; radius rx, ry); type = 128 meridians, 130 crater */
static void planet_arcs_prepare(u8 type, int cx, int cy, int rx, int ry)
{
    int stp = rx >= 64 ? 2 : (rx >= 32 ? 4 : 8);
    long k = rx, kv = ry;
    long nx, ny, rxv, ryv, sx, sy;
    int t0;
    arc_pn[0] = arc_pn[1] = 0;
    if (zK[1] != 0) return;
    if (type == 128) {
        if (rx < 6) return;
        nx = arc_comp(9) * k / 1536; ny = arc_comp(11) * kv / 1536;                 /* nosev */
        rxv = arc_comp(15) * k / 1536; ryv = arc_comp(17) * kv / 1536;              /* roofv */
        sx = arc_comp(21) * k / 1536; sy = arc_comp(23) * kv / 1536;                /* sidev */
        t0 = (arc_atan(arc_comp(13), arc_comp(19)) - 16) & 63;                       /* z of nosev cos t + roofv sin t >= 0 */
        arc_poly(0, cx, cy, nx, ny, rxv, ryv, t0, 32 / stp, stp);
        t0 = (arc_atan(arc_comp(13), arc_comp(25)) - 16) & 63;
        arc_poly(1, cx, cy, nx, ny, sx, sy, t0, 32 / stp, stp);
    } else {
        if (inwk[20] & 0x80) return;                                                /* crater on the hidden face */
        cx += (int)(arc_comp(15) * k * 222 / 6291456L);
        cy -= (int)(arc_comp(17) * kv * 222 / 6291456L);
        nx = arc_comp(9) * k / 3072; ny = arc_comp(11) * kv / 3072;                 /* nosev / 2 */
        sx = arc_comp(21) * k / 3072; sy = arc_comp(23) * kv / 3072;                /* sidev / 2 */
        arc_poly(0, cx, cy, nx, ny, sx, sy, 0, 64 / stp, stp);
    }
}

static void ell_erase(void)
{
    arcs_erase();
    if (!ell_valid) return;
    gfx_ell_update(ell_cx, ell_cy, ell_ry, ELL_T(ell_cur), 0, 0, -1, ELL_T(ell_cur), ell_col, NAT_BOT);
    ell_valid = 0;
}

/* PL9 of the native view: replaces the old ellipse with the new one ROW BY ROW (gfx_ell_update; ql_ellrows.asm on the QL): on each line the
   old is erased and the new drawn consecutively, so the planet does not disappear while it is redrawn (before, the whole ellipse was erased and
   then the whole drawn: flicker). The span table is only recalculated if the radius changes. Except when neither the
   centre nor the radius nor the colour has changed (flying straight with the planet far: nothing to do). If it is outside the view (CHKON) it only erases. */
void circle_update(void)
{
    int cx, cy, rx, ry, nxt;
    view_virtual_begin();
    if (sun_chkon()) { ell_erase(); view_virtual_end(); return; }       /* CHKON: outside the view */
    ell_params(&cx, &cy, &rx, &ry);
    if (ell_valid && cx == ell_cx && cy == ell_cy && rx == ell_rx && ry == ell_ry && sun_col == ell_col) { planet_arcs_prepare(ztype, cx, cy, rx, ry); arc_commit(); view_virtual_end(); return; }
    if (ell_valid && sun_col != ell_col) ell_erase();                   /* another colour: cannot be changed row by row with a single mask */
    if (ell_valid && rx == ell_rx && ry == ell_ry) nxt = ell_cur;       /* same radius: the same table */
    else {
        nxt = 1 - ell_cur;
        gfx_ell_spans(rx, ry, ELL_T(nxt));
    }
    planet_arcs_prepare(ztype, cx, cy, rx, ry);                          /* the meridian maths before touching the screen */
    gfx_ell_update(ell_cx, ell_cy, ell_valid ? ell_ry : -1, ELL_T(ell_cur), cx, cy, ry, ELL_T(nxt), sun_col, NAT_BOT);
    ell_cx = cx; ell_cy = cy; ell_rx = rx; ell_ry = ry; ell_col = sun_col; ell_cur = nxt;
    ell_valid = 1;
    arc_commit();                                                        /* and their drawing right after the planet */
    view_virtual_end();
}

static u8 ell_planet(void)
{
    circle_update();
    return ell_valid ? 0 : 1;
}

/* Tunnel ring: on launch (STP = 8) a rectangle with the proportion of the drawing window (256 x 160: half-height = 5/8 of the
   half-width); in hyperspace (STP = 4) an ellipse. Both are erased by drawing them again */
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
            s = (unsigned)(u8)(a ^ 0xFF) + 0 + 1;  /* EOR #$FF; ADC #0 with C = 1 */
            x = (u8)s; c = (u8)(s >> 8);
            s = 0xFFu + 0 + c; t = (u8)s;          /* LDA #$FF; ADC #0; STA T */
            a = x; c = 0;                          /* TXA; CLC */
        }
        s = (unsigned)a + zk3[0] + c;              /* PL37: ADC K3 */
        k6[0] = (u8)s;
        k6[1] = (u8)(zk3[1] + t + (s >> 8));
        a = (u8)(circ_cnt + 16);                   /* LDA CNT; CLC; ADC #16 */
        a = em_fmltu2(a);
        c = em_carry;                              /* the carry FMLTU2 returns with */
        x = a; t = 0;
        s = (unsigned)circ_cnt + 15 + c;           /* LDA CNT; ADC #15 */
        a = (u8)(s & 63);
        if (a >= 33) {                             /* CMP #33; BCC PL38 */
            s = (unsigned)(u8)(x ^ 0xFF) + 0 + 1;  /* TXA; EOR #$FF; ADC #0 with C = 1 */
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
    if (view_native) return ell_ring();                 /* tunnel rings */
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

/* WPLS2: erases the previous circle by drawing its segments again (XOR) */
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

/* ---------------------------------------------------------------- hyperspace / launch tunnel (HFS2, HFS1, HFL1) */
void (*sun_hiss_hook)(int y) = 0;                /* SOHISS(y): sound of each ring (set by the game) */
void (*sun_blip_hook)(int x, int y) = 0;         /* SOBLIP(x, y) of LL164 */
void (*sun_tt66_hook)(void) = 0;                 /* TT66 of HFS2: erase the view */
void (*sun_circle_hook)(u8 k3, u8 k4, u8 k, u8 stp) = 0;   /* tests: each CIRCLE2 of the tunnel */
void (*sun_pace_hook)(void) = 0;                 /* own: after each ring, so they do not all appear at once on fast machines */

/* HFL1: a series of concentric circles whose radius starts at (XX4 AND 7) + 8 and doubles while < 160 */
static void hfl1(u8 xx4)
{
    zK[0] = (u8)((xx4 & 7) + 8);
    for (;;) {                                   /* HFL2 */
        em_rand_c = 0;                           /* carry of the sound DORNDs: 0 (CLC; ADC #8 in HFL1 or CMP #160; BCC HFL2) */
        if (sun_hiss_hook) sun_hiss_hook(10);    /* LDY #10; JSR SOHISS */
        circ_lsp = 1;
        if (sun_circle_hook) sun_circle_hook(zk3[0], zk4[0], zK[0], circ_stp);
        circle2_draw();
        if (sun_pace_hook) sun_pace_hook();
        if (zK[0] & 0x80) return;                /* ASL K; BCS HF8 */
        zK[0] = (u8)(zK[0] << 1);
        if (zK[0] >= 160) return;                /* CMP #160; BCC HFL2 */
    }
}

/* HFS2 without the screen clear: STP = circle step (4 hyperspace, 8 launch); centre (X, Y) = (128, 68) */
void tunnel_draw(u8 stp)
{
    u8 xx4;
    circ_stp = stp;
    zk3[0] = 128; zk3[1] = 0;                    /* HFS1: K3 = X, K4 = Y */
    zk4[0] = 68;  zk4[1] = 0;
    for (xx4 = 0; xx4 != 8; xx4++) hfl1(xx4);
}

/* HFS2: TT66 (erase the view) and the rings */
static void hfs2(u8 stp)
{
    if (sun_tt66_hook) sun_tt66_hook();
    tunnel_draw(stp);
}

/* LAUN: two SOHISS of 256 clicks and the launch tunnel (STP = 8) */
void tunnel_laun(void)
{
    if (sun_hiss_hook) { sun_hiss_hook(0); sun_hiss_hook(0); }
    hfs2(8);
}

/* LL164: nine SOBLIPs with the period falling from 255 to 175 and the hyperspace tunnel (STP = 4). The first SBC #10 carries the
   incoming carry (with C = 0 the series is 255, 244, ..., 174); here always in steps of 10 (only the tone changes) */
void tunnel_ll164(void)
{
    u8 x = 255;
    do { if (sun_blip_hook) sun_blip_hook(x, 90); x = (u8)(x - 10); } while (x >= 170);
    hfs2(4);
}

/* Tests: SUNX */
void sun_get_sunx(u8 *v) { v[0] = sunx[0]; v[1] = sunx[1]; }
void sun_set_sunx(u8 lo, u8 hi) { sunx[0] = lo; sunx[1] = hi; }

/* After erasing the view area: neither the sun nor the planet circle count as drawn */
void sun_forget(void)
{
    arc_n[0] = arc_n[1] = 0;                     /* nor their meridians or crater */
    ell_valid = 0;                               /* the planet ellipse (native view) does not count as drawn either */
    sn_clear();                                  /* nor the sun spans */
    sun_flflls();
    circ_lsp = 1;
    circ_lsx2[0] = 255;
    circ_lsy2[0] = 255;
}
