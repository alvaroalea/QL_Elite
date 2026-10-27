#include "platform.h"

/* Mode 8: each word = 4 pixels. For pixel k (0..3, left to right), let s=2*(3-k):
   byte0: bit s+1 = Green, bit s = Flash ; byte1: bit s+1 = Red, bit s = Blue.
   Mode 4: each word = 8 physical dots (byte0 = green, byte1 = red, bit 7 = leftmost). A game dot (x from 0 to 255) is
   2 consecutive physical dots, which fall exactly on the bit pairs s+1 and s of mode 8: that is why the dot, span and
   fill routines are the same in both modes and only the colour masks change (the "flash" bit acts as the second green dot and the
   "blue" bit as the red one; see gfx_ccode). The lines and ellipses of the flight view use the 512 dots (suffix _hr). */
#define OFF(x,y) ((long)(y) * SCR_STRIDE + (((x) >> 2) << 1))

u8 gfx_m4 = 0;
u8 gfx_hires_opt = 0;
void (*gfx_mode_hook)(void) = 0;

static const u8 ccm4[8] = { 0, 15, 3, 15, 12, 15, 15, 15 };   /* mode 4: black, red (2) and green (4) as they are; everything else, white */
#define CC(c) (gfx_m4 ? ccm4[(c) & 7] : (u8)((c) & 7))

u8 gfx_ccode(u8 c) { return CC(c); }

void gfx_set_mode(int m4)
{
    gfx_m4 = (u8)(m4 ? 1 : 0);
    if (gfx_mode_hook) gfx_mode_hook();
}

static int iabs(int v) { return v < 0 ? -v : v; }

void gfx_pixel(int x, int y, u8 c)
{
    u8 *p;
    int s;
    u8 m, cc;
    if ((unsigned)x >= SCR_W || (unsigned)y >= SCR_H) return;
    p = screen + OFF(x, y);
    s = 2 * (3 - (x & 3));
    m = (u8)(3 << s);
    cc = CC(c);
    p[0] = (u8)((p[0] & ~m) | (GFX_GM(cc) & m));
    p[1] = (u8)((p[1] & ~m) | (GFX_RM(cc) & m));
}

#ifdef ASM_LINE
extern void ql_xpix(long x, long y, long c, long scr);                 /* ql_ell.asm */
#else
static void xor1(int x, int y, u8 c)
{
    u8 *p;
    int s;
    u8 m, cc;
    if ((unsigned)x >= SCR_W || (unsigned)y >= SCR_H) return;
    p = screen + OFF(x, y);
    s = 2 * (3 - (x & 3));
    m = (u8)(3 << s);
    cc = CC(c);
    p[0] ^= (u8)(GFX_GM(cc) & m);
    p[1] ^= (u8)(GFX_RM(cc) & m);
}
#endif

/* Vertical offset of what is drawn in XOR: the original's panel is on lines 136..191; with the 160-line flight view
   it moves down to 160..215 and its routines (DIALS, radar, bulbs...) keep working in the original's coordinates. */
static int yoff = 0;
void gfx_yoff_set(int off) { yoff = off; }

void gfx_xor_pixel(int x, int y, u8 c)
{
#ifdef ASM_LINE
    y += yoff;
    if ((unsigned)x > 255 || y < 0 || y > 255) return;
    ql_xpix((long)x, (long)y, (long)CC(c), (long)screen);
#else
    xor1(x, y + yoff, c);
#endif
}

/* Mode 4: one physical dot (x from 0 to 511) in XOR, with the same vertical offset as gfx_xor_pixel. In mode 8 there is no double
   resolution: it is the game dot x / 2 */
void gfx_xor_pixel_hr(int x, int y, u8 c)
{
    u8 *p, m, cc;
    if (!gfx_m4) { gfx_xor_pixel(x >> 1, y, c); return; }
    y += yoff;
    if ((unsigned)x > 511U || (unsigned)y > 255U) return;
    cc = CC(c);
    p = screen + (long)y * SCR_STRIDE + (((long)x >> 3) << 1);
    m = (u8)(0x80 >> (x & 7));
    if (cc & 4) p[0] ^= m;
    if (cc & 2) p[1] ^= m;
}

u8 gfx_get(int x, int y)
{
    u8 *p;
    int s;
    int g, r;
    if ((unsigned)x >= SCR_W || (unsigned)y >= SCR_H) return 0;
    p = screen + OFF(x, y);
    s = 2 * (3 - (x & 3));
    g = (p[0] >> (s + 1)) & 1; r = (p[1] >> (s + 1)) & 1;
    if (gfx_m4) return (u8)((g << 2) | (r << 1) | (g & r));       /* white = green and red */
    return (u8)((g << 2) | (r << 1) | ((p[1] >> s) & 1));
}

/* Fast fill of n whole lines (128 bytes each) from line y with colour c. On the QL it is written 4 bytes at a time and
   unrolled 8 long words at a time (68008: about 20 times faster than setting pixel by pixel); the PC uses bytes so as not to depend
   on aliasing rules. The screen and the lines are at even addresses. */
void gfx_fill_lines(int y, int n, u8 c)
{
    u8 cc = CC(c);
    u8 g = GFX_GM(cc);
    u8 rb = GFX_RM(cc);
    if (y < 0) { n += y; y = 0; }
    if (y + n > SCR_H) n = SCR_H - y;
    if (n <= 0) return;
#ifdef __GNUC__
    {
        u8 *p = screen + (long)y * 128L;
        long i, cnt = (long)n * 128L;
        for (i = 0; i < cnt; i += 2) { p[i] = g; p[i + 1] = rb; }
    }
#else
    {
        unsigned long v = ((unsigned long)g << 24) | ((unsigned long)rb << 16) | ((unsigned long)g << 8) | rb;
        unsigned long *p = (unsigned long *)(screen + (long)y * 128L);
        long cnt = (long)n * 32L;                       /* long words: 32 per line */
        while (cnt >= 8) {
            p[0] = v; p[1] = v; p[2] = v; p[3] = v; p[4] = v; p[5] = v; p[6] = v; p[7] = v;
            p += 8; cnt -= 8;
        }
    }
#endif
}

/* Fast copy of n bytes (multiple of 4) to or from the screen; by long words if both addresses are even */
void gfx_copy(u8 *dst, const u8 *src, long n)
{
#ifndef __GNUC__
    if ((((unsigned long)dst | (unsigned long)src) & 1UL) == 0UL && (n & 31L) == 0L) {
        unsigned long *d = (unsigned long *)dst;
        const unsigned long *s = (const unsigned long *)src;
        long cnt = n >> 2;
        while (cnt > 0) {
            d[0] = s[0]; d[1] = s[1]; d[2] = s[2]; d[3] = s[3]; d[4] = s[4]; d[5] = s[5]; d[6] = s[6]; d[7] = s[7];
            d += 8; s += 8; cnt -= 8;
        }
        return;
    }
#endif
    {
        long i;
        for (i = 0; i < n; i++) dst[i] = src[i];
    }
}

void gfx_clear(u8 c) { gfx_fill_lines(0, SCR_H, c); }

void gfx_line(int x0, int y0, int x1, int y1, u8 c, int xor_mode)
{
    int dx = iabs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -iabs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    int e2;
    for (;;) {
        if (xor_mode) gfx_xor_pixel(x0, y0, c); else gfx_pixel(x0, y0, c);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

#ifdef ASM_LINE
extern void ql_ellipse(long *blk);                                          /* ql_ell.asm */
extern void ql_hline(long x0, long x1, long y, long c, long scr);
extern void ql_vline(long x, long y0, long y1, long c, long scr);
extern void ql_frame(long *blk);
extern void ql_ellrows(long *blk);                                          /* ql_ellrows.asm */
extern void ql_ellrows4(long *blk);                                         /* mode 4, at the 512 dots */
extern void ql_clearcells(long col, long row, long ncols, long nrows, long scr);
#endif

void gfx_hline_xor(int x0, int x1, int y, u8 c)
{
    int x, t;
    if (x0 > x1) { t = x0; x0 = x1; x1 = t; }
#ifdef ASM_LINE
    y += yoff;                                           /* like gfx_xor_pixel */
    if (y < 0 || y > 255 || x1 < 0 || x0 > 255) return;
    if (x0 < 0) x0 = 0;
    if (x1 > 255) x1 = 255;
    ql_hline((long)x0, (long)x1, (long)y, (long)CC(c), (long)screen);
#else
    for (x = x0; x <= x1; x++) gfx_xor_pixel(x, y, c);
#endif
    (void)x;
}

/* Column of dots (x, y0..y1) in XOR (a column one dot wide) */
void gfx_vline_xor(int x, int y0, int y1, u8 c)
{
    int y, t;
    if (y0 > y1) { t = y0; y0 = y1; y1 = t; }
#ifdef ASM_LINE
    y0 += yoff; y1 += yoff;                              /* like gfx_xor_pixel */
    if (x < 0 || x > 255 || y1 < 0 || y0 > 255) return;
    if (y0 < 0) y0 = 0;
    if (y1 > 255) y1 = 255;
    ql_vline((long)x, (long)y0, (long)y1, (long)CC(c), (long)screen);
#else
    for (y = y0; y <= y1; y++) gfx_xor_pixel(x, y, c);
#endif
    (void)y;
}

/* HLOIN of the original (Apple II): X1 and X2 are masked to even values (2-dot granularity of the hires mode), sorted and the line
   covers X1 to X2-1; if they end up equal it draws nothing. Always in XOR. */
void gfx_hloin(int x1, int x2, int y, u8 c)
{
    int t;
    x1 &= 0xFE; x2 &= 0xFE;
    if (x1 == x2) return;
    if (x1 > x2) { t = x1; x1 = x2; x2 = t; }
    gfx_hline_xor(x1, x2 - 1, y, c);
}

/* CPIX of the original: an Elite colour "pixel" = 2 Apple II dots wide (here x and x+1), in XOR */
void gfx_cpix(int x, int y, u8 c) { gfx_hline_xor(x, x + 1, y, c); }       /* two dots wide (x, x + 1) */

/* VLOIN of the original: vertical line of CPIX pixels from y1 to y2 (in either order), both ends included */
void gfx_vloin(int x, int y1, int y2, u8 c)
{
    int t;
    if (y1 > y2) { t = y1; y1 = y2; y2 = t; }
    gfx_vline_xor(x, y1, y2, c);                         /* CPIX = 2 dots wide: columns x and x + 1 */
    gfx_vline_xor(x + 1, y1, y2, c);
}

/* PIXEL of the original: a dust/compass "dot" in XOR. The width and lines depend on the distance ZZ:
   ZZ >= 128: 2 dots on one line; 80 <= ZZ < 128: 3 dots; ZZ < 80: 3 dots on this line and on the one above. */
/* PIXEL of the original as is (3 dots wide, two lines if zz < 80): used by the compass and verified by test_lines */
void gfx_dust_6502(int x, int y, u8 zz)
{
    int w = (zz & 0x80) ? 2 : 3, i;
    for (i = 0; i < w; i++) gfx_xor_pixel(x + i, y, C_WHITE);
    if (zz < 80) for (i = 0; i < w; i++) gfx_xor_pixel(x + i, y - 1, C_WHITE);
}

/* Game stardust (cosmetic only): the size depends on the distance ZZ: far (ZZ >= 128) 1 dot; between 80 and 127, 1 wide
   by 2 high; near (ZZ < 80) 2 wide by 3 high (QL dots are wider than they are tall). The lower left
   corner is (x, y) and it grows up and to the right. Columns with gfx_vline_xor (assembly on the QL). */
void gfx_dust(int x, int y, u8 zz)
{
    int w = 1, h = 1, i;
    if (zz < 80) { w = 2; h = 3; }
    else if (zz < 128) h = 2;
    if (w == 1 && h == 1) { gfx_xor_pixel(x, y, C_WHITE); return; }
    for (i = 0; i < w; i++) gfx_vline_xor(x + i, y - h + 1, y, C_WHITE);
}

void gfx_hline(int x0, int x1, int y, u8 c)
{
    int x, t;
    if (x0 > x1) { t = x0; x0 = x1; x1 = t; }
    for (x = x0; x <= x1; x++) gfx_pixel(x, y, c);
}

void gfx_rect(int x, int y, int w, int h, u8 c)
{
    int j;
    if (x <= 0 && x + w >= SCR_W) { gfx_fill_lines(y, h, c); return; }       /* full width: fast fill */
    for (j = 0; j < h; j++) gfx_hline(x, x + w - 1, y + j, c);
}

/* Midpoint ellipse (Bresenham, integer variant by A. Zingl): centre (cx, cy), semi-axes rx and ry, one-dot outline in XOR.
   Only one quadrant is traversed (additions and comparisons; the multiplications are in the set-up) and mirrored into the four. The
   points on the axes are drawn ONCE (in XOR twice would erase them). It is deterministic: calling it again with the same data
   erases what was drawn, so there is no need to store the points. All in long (in c68 int is 16 bits): with rx <= 300 and ry <= 360 the
   error terms fit in 32 bits. Clips to the rectangle (clx0, cly0)-(clx1, cly1); if it fits entirely it goes without checking each point. */
static u8 el_col;
static int el_x0, el_y0, el_x1, el_y1, el_clip, el_hr;

static void el_plot(int x, int y)
{
    if (el_clip && (x < el_x0 || x > el_x1 || y < el_y0 || y > el_y1)) return;
    if (el_hr) gfx_xor_pixel_hr(x, y, el_col); else gfx_xor_pixel(x, y, el_col);
}

static void ellipse_core(int cx, int cy, int rx, int ry, u8 c, int clx0, int cly0, int clx1, int cly1, int hr);

/* In mode 4 the ellipse is drawn at the 512 dots: the horizontal semi-axis is 2 rx physical dots (rx and ry are the game's, with the
   monitor aspect already applied to ry: the physical dots of mode 4 are half as wide as those of mode 8, the same 2 x as the lines) */
void gfx_ellipse_xor(int cx, int cy, int rx, int ry, u8 c, int clx0, int cly0, int clx1, int cly1)
{
    if (gfx_m4) {
        if (rx < 0) rx = -rx;
        if (cx + rx < clx0 || cx - rx > clx1) return;      /* before doubling: cx can be large and in c68 int is 16 bits */
        ellipse_core(2 * cx, cy, 2 * rx, ry, c, 2 * clx0, cly0, 2 * clx1 + 1, cly1, 1);
    } else ellipse_core(cx, cy, rx, ry, c, clx0, cly0, clx1, cly1, 0);
}

static void ellipse_core(int cx, int cy, int rx, int ry, u8 c, int clx0, int cly0, int clx1, int cly1, int hr)
{
    long a, b, dx, dy, err, e2, a8, b8;
    int x0, x1, y0, y1;
    if (rx < 0) rx = -rx;
    if (ry < 0) ry = -ry;
    if (cx + rx < clx0 || cx - rx > clx1 || cy + ry < cly0 || cy - ry > cly1) return;       /* entirely outside */
    el_col = c; el_x0 = clx0; el_y0 = cly0; el_x1 = clx1; el_y1 = cly1; el_hr = hr;
    el_clip = !(cx - rx >= clx0 && cx + rx <= clx1 && cy - ry >= cly0 && cy + ry <= cly1);
    if (rx == 0 || ry == 0) {                          /* degenerate: a point or a segment */
        for (y0 = cy - ry; y0 <= cy + ry; y0++) for (x0 = cx - rx; x0 <= cx + rx; x0++) el_plot(x0, y0);
        return;
    }
    x0 = cx - rx; x1 = cx + rx;
    y0 = cy; y1 = cy;
    a = 2L * rx; b = 2L * ry;
    dx = 4L * (1L - a) * b * b;
    dy = 4L * a * a;
    err = dx + dy;
    a8 = 8L * a * a; b8 = 8L * b * b;
#ifdef ASM_LINE
    {                                                    /* whole loop in assembly (ql_ell.asm): parameter block */
        static long blk[19];
        blk[0] = a8; blk[1] = b8;
        blk[2] = (long)GFX_GM(CC(c));
        blk[3] = (long)GFX_RM(CC(c));
        blk[18] = (long)hr;
        blk[4] = clx0; blk[5] = cly0; blk[6] = clx1; blk[7] = cly1;
        blk[8] = dx; blk[9] = dy; blk[10] = 0L; blk[11] = err;
        blk[12] = (long)screen; blk[13] = 2L * ry;
        blk[14] = x0; blk[15] = x1; blk[16] = y0; blk[17] = y1;
        ql_ellipse(blk);
        return;
    }
#endif
    do {
        el_plot(x1, y0);
        if (x0 != x1) el_plot(x0, y0);
        if (y1 != y0) {
            el_plot(x0, y1);
            if (x0 != x1) el_plot(x1, y1);
        }
        e2 = 2L * err;
        if (e2 <= dy) { y0++; y1--; dy += a8; err += dy; }
        if (e2 >= dx || 2L * err > dy) { x0++; x1--; dx += b8; err += dx; }
    } while (x0 <= x1);
    if (y0 - y1 < 2 * ry) while (y0 - y1 <= 2 * ry) {   /* narrow ellipses: the loop ended before reaching the top; finishes the vertical part */
        el_plot(x0 - 1, y0);
        if (x1 + 1 != x0 - 1) el_plot(x1 + 1, y0);
        if (y1 != y0) { el_plot(x0 - 1, y1); if (x1 + 1 != x0 - 1) el_plot(x1 + 1, y1); }
        y0++; y1--;
    }
}

/* Half-widths of the ellipse row by row (to fill it): the same midpoint traversal as gfx_ellipse_xor, without drawing; in each
   row dy (0 = the centre one) the first point of the traversal is the rightmost. half[dy] = x - cx for dy = 0..ry (ry < 512). */
void gfx_ellipse_rows(int rx, int ry, u8 *half)
{
    long a, b, dx, dy, err, e2, a8, b8;
    int x1, y0, last = -1, d;
    if (rx < 0) rx = -rx;
    if (ry < 0) ry = -ry;
    if (rx == 0 || ry == 0) { for (d = 0; d <= ry; d++) half[d] = (u8)rx; return; }
    x1 = rx; y0 = 0;
    a = 2L * rx; b = 2L * ry;
    dx = 4L * (1L - a) * b * b;
    dy = 4L * a * a;
    err = dx + dy;
    a8 = 8L * a * a; b8 = 8L * b * b;
    {
        int x0 = -rx, xe = rx;                          /* x0, x1 relative to the centre */
        x1 = xe;
        do {
            if (y0 != last && y0 <= ry) { half[y0] = (u8)(x1 > 255 ? 255 : x1); last = y0; }
            e2 = 2L * err;
            if (e2 <= dy) { y0++; dy += a8; err += dy; }
            if (e2 >= dx || 2L * err > dy) { x0++; x1--; dx += b8; err += dx; }
        } while (x0 <= x1);
        for (d = y0; d <= ry; d++) {                    /* tail of the narrow ones (as in gfx_ellipse_xor) */
            int v = x1 + 1 > 255 ? 255 : (x1 + 1 < 0 ? 0 : x1 + 1);
            if (d > last || half[d] < v) half[d] = (u8)v;
        }
    }
}

/* Mode 4, at the 512 dots: like gfx_ellipse_rows but with the semi-axis rx already in physical dots (up to 510) and 16-bit half-widths */
static void ell_rows_hr(int rx, int ry, unsigned short *half)
{
    long a, b, dx, dy, err, e2, a8, b8;
    int x0, x1, y0, last = -1, d;
    if (rx == 0 || ry == 0) { for (d = 0; d <= ry; d++) half[d] = (unsigned short)rx; return; }
    y0 = 0;
    a = 2L * rx; b = 2L * ry;
    dx = 4L * (1L - a) * b * b;
    dy = 4L * a * a;
    err = dx + dy;
    a8 = 8L * a * a; b8 = 8L * b * b;
    x0 = -rx; x1 = rx;
    do {
        if (y0 != last && y0 <= ry) { half[y0] = (unsigned short)x1; last = y0; }
        e2 = 2L * err;
        if (e2 <= dy) { y0++; dy += a8; err += dy; }
        if (e2 >= dx || 2L * err > dy) { x0++; x1--; dx += b8; err += dx; }
    } while (x0 <= x1);
    for (d = y0; d <= ry; d++) {
        int v = x1 + 1 < 0 ? 0 : x1 + 1;
        if (d > last || half[d] < v) half[d] = (unsigned short)v;
    }
}

/* Table of outline spans of an ellipse of semi-axes rx, ry (ry <= 303): for each distance dy = 0..ry from the centre, tab[2 * dy] = lo and
   tab[2 * dy + 1] = hi: the outline of that row is cx + lo .. cx + hi (and its reflection on the left; with lo = 0, a single span cx - hi .. cx + hi).
   It comes from the half-width table half[] (gfx_ellipse_rows): row dy < ry: from half[dy + 1] + 1 to half[dy] if half[dy + 1] < half[dy], otherwise the
   point half[dy]; row dy = ry: from 0 to half[ry]. It is the same outline gfx_ellipse_xor draws (midpoint): checked point by point in
   make test_ellrows. It takes 2 * (ry + 1) bytes. In mode 4 the table is of 16-bit words (unsigned short, even address; 4 * (ry + 1)
   bytes) and in physical dots: rx is the game's semi-axis and the outline comes out with 2 rx. */
void gfx_ell_spans(int rx, int ry, u8 *tab)
{
    static u8 half[304];
    int dy;
    if (gfx_m4) {
        static unsigned short half16[304];
        unsigned short *t = (unsigned short *)tab;
        ell_rows_hr(2 * (rx < 0 ? -rx : rx), ry < 0 ? -ry : ry, half16);
        if (ry < 0) ry = -ry;
        for (dy = 0; dy <= ry; dy++) {
            t[2 * dy + 1] = half16[dy];
            t[2 * dy] = (unsigned short)((dy == ry) ? 0 : ((half16[dy + 1] < half16[dy]) ? half16[dy + 1] + 1 : half16[dy]));
        }
        return;
    }
    gfx_ellipse_rows(rx, ry, half);
    for (dy = 0; dy <= ry; dy++) {
        tab[2 * dy + 1] = half[dy];
        tab[2 * dy] = (u8)((dy == ry) ? 0 : ((half[dy + 1] < half[dy]) ? half[dy + 1] + 1 : half[dy]));
    }
}

/* Span of row y (gfx_ell_spans table; cy = centre, ry = half-height, -1 = no ellipse): 0 if it does not touch the row, otherwise 1 with *lo and *hi */
static int ell_span(int y, int cy, int ry, const u8 *tab, int *lo, int *hi)
{
    int dy = y - cy;
    if (dy < 0) dy = -dy;
    if (ry < 0 || dy > ry) return 0;
    *lo = tab[2 * dy]; *hi = tab[2 * dy + 1];
    return 1;
}

#ifndef ASM_LINE
/* Mode 4 (physical dots): the same table in 16-bit words */
static int ell_span_hr(int y, int cy, int ry, const unsigned short *tab, int *lo, int *hi)
{
    int dy = y - cy;
    if (dy < 0) dy = -dy;
    if (ry < 0 || dy > ry) return 0;
    *lo = tab[2 * dy]; *hi = tab[2 * dy + 1];
    return 1;
}

static void hline_hr(int x0, int x1, int y, u8 c)       /* x0 <= x1, 0 to 511 clipped */
{
    if (x0 < 0) x0 = 0;
    if (x1 > 511) x1 = 511;
    for (; x0 <= x1; x0++) gfx_xor_pixel_hr(x0, y, c);
}

static void ell_xor_spans_hr(int cx, int lo, int hi, int y, u8 c)
{
    if (lo == 0) { hline_hr(cx - hi, cx + hi, y, c); return; }
    hline_hr(cx - hi, cx - lo, y, c);
    hline_hr(cx + lo, cx + hi, y, c);
}
#endif

static void ell_xor_spans(int cx, int lo, int hi, int y, u8 c)
{
    if (lo == 0) { gfx_hline_xor(cx - hi, cx + hi, y, c); return; }
    gfx_hline_xor(cx - hi, cx - lo, y, c);
    gfx_hline_xor(cx + lo, cx + hi, y, c);
}

/* Replaces one ellipse (centre ocx, ocy, half-height ory, span table otab; ory = -1 if there was none) with another (ncx, ncy, nry, ntab; nry = -1 to
   erase it), clipped to x 0..255 and y 0..cly1, ROW BY ROW: on each line the old is erased and the new drawn consecutively (and skipped if
   unchanged), so the ellipse never disappears entirely. The result on screen is the same as gfx_ellipse_xor(old) + gfx_ellipse_xor(new)
   (XOR). yoff must be 0. On the QL it is in assembly (ql_ellrows.asm). In mode 4 the tables are 16-bit and in physical dots
   (gfx_ell_spans), the centres are still the game's (physical x = 2 x) and it clips to 0..511. */
void gfx_ell_update(int ocx, int ocy, int ory, const u8 *otab, int ncx, int ncy, int nry, const u8 *ntab, u8 c, int cly1)
{
    int top = 32767, bot = -32768, y;
    if (ory >= 0) { top = ocy - ory; bot = ocy + ory; }
    if (nry >= 0) { if (ncy - nry < top) top = ncy - nry; if (ncy + nry > bot) bot = ncy + nry; }
    if (top > bot) return;
    if (top < 0) top = 0;
    if (bot > cly1) bot = cly1;
    if (top > bot) return;
#ifdef ASM_LINE
    {
        static long blk[13];
        blk[0] = top; blk[1] = bot;
        blk[2] = ocx; blk[3] = ocy; blk[4] = ory; blk[5] = (long)otab;
        blk[6] = ncx; blk[7] = ncy; blk[8] = nry; blk[9] = (long)ntab;
        blk[10] = (long)GFX_GM(CC(c));
        blk[11] = (long)GFX_RM(CC(c));
        blk[12] = (long)screen;
        if (gfx_m4) { blk[2] = 2L * ocx; blk[6] = 2L * ncx; ql_ellrows4(blk); }
        else ql_ellrows(blk);
        return;
    }
#else
    if (gfx_m4) {
        for (y = top; y <= bot; y++) {
            int olo, ohi, nlo, nhi, o = ell_span_hr(y, ocy, ory, (const unsigned short *)otab, &olo, &ohi), n = ell_span_hr(y, ncy, nry, (const unsigned short *)ntab, &nlo, &nhi);
            if (!o && !n) continue;
            if (o && n && olo == nlo && ohi == nhi && ocx == ncx) continue;
            if (o) ell_xor_spans_hr(2 * ocx, olo, ohi, y, c);
            if (n) ell_xor_spans_hr(2 * ncx, nlo, nhi, y, c);
        }
        return;
    }
    for (y = top; y <= bot; y++) {
        int olo, ohi, nlo, nhi, o = ell_span(y, ocy, ory, otab, &olo, &ohi), n = ell_span(y, ncy, nry, ntab, &nlo, &nhi);
        if (!o && !n) continue;
        if (o && n && olo == nlo && ohi == nhi && ocx == ncx) continue;
        if (o) ell_xor_spans(ocx, olo, ohi, y, c);
        if (n) ell_xor_spans(ncx, nlo, nhi, y, c);
    }
#endif
    (void)y;
}

/* Rectangle outline (centre, half-width hw, half-height hh) in XOR: each point only once, clipped */
#ifndef ASM_LINE
static void fr_row(int x0, int x1, int y, u8 c, int clx0, int cly0, int clx1, int cly1)
{
    if (y < cly0 || y > cly1) return;
    if (x0 < clx0) x0 = clx0;
    if (x1 > clx1) x1 = clx1;
    for (; x0 <= x1; x0++) gfx_xor_pixel(x0, y, c);
}
#endif

void gfx_frame_xor(int cx, int cy, int hw, int hh, u8 c, int clx0, int cly0, int clx1, int cly1)
{
    if (hw < 0) hw = -hw;
    if (hh < 0) hh = -hh;
#ifdef ASM_LINE
    {                                                    /* all in assembly (ql_frame, ql_ell.asm): parameter block */
        static long blk[11];
        blk[0] = cx - hw; blk[1] = cy - hh; blk[2] = cx + hw; blk[3] = cy + hh;
        blk[4] = clx0; blk[5] = cly0; blk[6] = clx1; blk[7] = cly1;
        blk[8] = (long)GFX_GM(CC(c));
        blk[9] = (long)GFX_RM(CC(c));
        blk[10] = (long)screen;
        ql_frame(blk);
    }
#else
    {
        int y;
        fr_row(cx - hw, cx + hw, cy - hh, c, clx0, cly0, clx1, cly1);                 /* top */
        if (hh > 0) fr_row(cx - hw, cx + hw, cy + hh, c, clx0, cly0, clx1, cly1);     /* bottom */
        for (y = cy - hh + 1; y <= cy + hh - 1; y++) {                                 /* sides, without the corners */
            if (y < cly0 || y > cly1) continue;
            if (cx - hw >= clx0 && cx - hw <= clx1) gfx_xor_pixel(cx - hw, y, c);
            if (hw > 0 && cx + hw >= clx0 && cx + hw <= clx1) gfx_xor_pixel(cx + hw, y, c);
        }
    }
#endif
}

/* Empties (black) a rectangle of text cells: ncols columns from col, nrows rows from row (0..31; clipped). Texts that
   change length are not erased by writing spaces: the leftover rectangle is calculated and emptied at once. */
void gfx_clear_cells(int col, int row, int ncols, int nrows)
{
    if (col < 0) { ncols += col; col = 0; }
    if (row < 0) { nrows += row; row = 0; }
    if (col + ncols > 32) ncols = 32 - col;
    if (row + nrows > 32) nrows = 32 - row;
    if (ncols <= 0 || nrows <= 0) return;
#ifdef ASM_LINE
    ql_clearcells((long)col, (long)row, (long)ncols, (long)nrows, (long)screen);
#else
    {
        int y, i;
        for (y = row * 8; y < (row + nrows) * 8; y++) {
            u8 *p = screen + (long)y * SCR_STRIDE + (long)col * 4;
            for (i = 0; i < ncols * 4; i++) p[i] = 0;
        }
    }
#endif
}
