#include "text.h"
#include "font.h"

/* 8x8 cell = 4 bytes per line: [0..1] pixels 0-3, [2..3] pixels 4-7 (see gfx.c).
   A font nibble (bit3 = left pixel) is expanded to the "high" bits of each pair:
   pixel k -> bit 2*(3-k)+1. Green goes in the even byte, red (high bit) and blue (low bit) in the odd one.

   Each glyph line is written at once, as a long word (4 bytes):
     fm[b]  pixel mask of font line b: both bits of each pixel set (fixed table, created in txt_init)
     paper: pp = paper colour in the 4 bytes;  dd = ink XOR paper;   opaque:  line = pp ^ (fm & dd)
     transparent: ii = ink colour;  line = screen ^ ((screen ^ ii) & fm)
   They are bit operations on long words, so the byte order inside the word does not matter. */

#ifdef __GNUC__
#include <string.h>
typedef unsigned int u32;
#define LD(p) (memcpy(&t32, (p), 4), t32)
#define ST(p, v) (t32 = (v), memcpy((p), &t32, 4))
static u32 t32;
#else
typedef unsigned long u32;
#define LD(p) (*(u32 *)(p))
#define ST(p, v) (*(u32 *)(p) = (v))
#endif

static u8 ex[16];
static u32 fm[256];
static unsigned short fi[96][8];   /* font x 4 = byte offset inside fm (used by ql_puts) */
static u32 pp, dd, ii;
static u8 ink = C_WHITE;
static int paper = C_BLACK;
static int cx = 0, cy = 0;
static int nowrap = 0;          /* 1: text that runs off the right edge is lost instead of continuing on the next row */
static int clipped = 0;         /* the last column was already reached with nowrap: whatever follows is ignored (until the next txt_at) */

static void set_masks(void);

void txt_init(void)
{
    int n, k;
    for (n = 0; n < 16; n++) {
        ex[n] = 0;
        for (k = 0; k < 4; k++)
            if (n & (8 >> k)) ex[n] = (u8)(ex[n] | (2 << (2 * (3 - k))));
    }
    for (n = 0; n < 256; n++) {
        u8 *q = (u8 *)&fm[n];
        u8 fh = (u8)(ex[n >> 4] | (ex[n >> 4] >> 1)), fl = (u8)(ex[n & 15] | (ex[n & 15] >> 1));
        q[0] = q[1] = fh; q[2] = q[3] = fl;
    }
    for (n = 0; n < 96; n++)
        for (k = 0; k < 8; k++) fi[n][k] = (unsigned short)(font8x8[n][k] * 4);
    ink = C_WHITE; paper = C_BLACK; cx = cy = 0;
    set_masks();
    gfx_mode_hook = set_masks;                       /* when the mode changes the same colours give different masks */
}

/* Colours in screen format: even byte = green (bit 2), odd = red (bit 1) and blue (bit 0); in mode 4 the gfx_ccode codes
   (green = $FF in the even byte, red = $FF in the odd: each text dot is 2 physical dots) */
static void col_bytes(int col, u8 *q)
{
    u8 cc = gfx_ccode((u8)col);
    q[0] = q[2] = GFX_GM(cc);
    q[1] = q[3] = GFX_RM(cc);
}

static void set_masks(void)
{
    u32 pk, ik;
    u8 *q;
    q = (u8 *)&ik; col_bytes(ink, q);
    ii = ik;
    if (paper >= 0) {
        q = (u8 *)&pk; col_bytes(paper, q);
        pp = pk; dd = ik ^ pk;
    }
}

/* Opaque paper (the usual): strip of consecutive cells without reading the screen. Draws up to maxn characters of s while they are printable
   (32..127) and returns how many; with ASM_TEXT it is ql_puts (ql_text.asm), otherwise its C version (the same formula) */
#ifdef ASM_TEXT
extern long ql_puts(u8 *dst, const u8 *s, const unsigned short *font, const u32 *fmt, long ppl, long ddl, long maxn);
#else
static long ql_puts(u8 *dst, const u8 *s, const unsigned short *font, const u32 *fmt, long ppl, long ddl, long maxn)
{
    long n = 0;
    unsigned c;
    int r;
    const unsigned short *g;
    while (n < maxn) {
        c = (unsigned)*s - 32U;
        if (c > 95U) break;
        g = font + c * 8;
        for (r = 0; r < 8; r++) ST(dst + r * SCR_STRIDE, (u32)ppl ^ (fmt[g[r] >> 2] & (u32)ddl));
        s++; dst += 4; n++;
    }
    return n;
}
#endif

/* Transparent paper (chart names, panel without file): respects whatever is drawn underneath */
static void draw_glyph_transp(const u8 *g, u8 *p)
{
    u32 v;
    int r;
    for (r = 0; r < 8; r++) {
        v = LD(p);
        ST(p, v ^ ((v ^ ii) & fm[g[r]]));
        p += SCR_STRIDE;
    }
}

static void draw_glyph(int ch)
{
    u8 *p;
    u8 cc;
    if (ch < 32 || ch > 127) ch = '?';
    p = screen + ((long)cy << 10) + ((long)cx << 2);
    if (paper >= 0) { cc = (u8)ch; ql_puts(p, &cc, &fi[0][0], fm, (long)pp, (long)dd, 1L); }
    else draw_glyph_transp(font8x8[ch - 32], p);
}

void txt_color(u8 i, int p) { ink = (u8)(i & 7); paper = (p < 0) ? TXT_TRANSPARENT : (p & 7); set_masks(); }

void txt_at(int col, int row)
{
    clipped = 0;
    cx = col < 0 ? 0 : (col >= TXT_COLS ? TXT_COLS - 1 : col);
    cy = row < 0 ? 0 : (row >= TXT_ROWS ? TXT_ROWS - 1 : row);
}

int txt_col(void) { return cx; }
int txt_row(void) { return cy; }

static void advance(void)                         /* after drawing a cell: next column, with line wrap or clipping */
{
    if (++cx >= TXT_COLS) {
        if (nowrap) { cx = TXT_COLS - 1; clipped = 1; }
        else { cx = 0; if (cy < TXT_ROWS - 1) cy++; }
    }
}

void txt_putc(int ch)
{
    if (ch >= 32 && ch <= 127 && !clipped) {          /* the usual: a printable character */
        draw_glyph(ch);
        advance();
    }
    else if (ch == '\n') { cx = 0; if (cy < TXT_ROWS - 1) cy++; }
    else if (ch == '\r') { cx = 0; }
    else if (ch == '\b') { if (cx > 0) cx--; }
    else if (clipped) { }
    else {
        draw_glyph(ch);                               /* out of range: '?' */
        advance();
    }
}

void txt_nowrap(int on) { nowrap = on ? 1 : 0; clipped = 0; }

/* With opaque paper, strips of printable characters that fit in the row are drawn at once (ql_puts); the rest (control, out of
   range, clipped text, transparent paper) goes character by character through txt_putc */
void txt_puts(const char *s)
{
    long n;
    while (*s) {
        if (paper >= 0 && !clipped && (u8)*s >= 32 && (u8)*s <= 127) {
            n = ql_puts(screen + ((long)cy << 10) + ((long)cx << 2), (const u8 *)s, &fi[0][0], fm, (long)pp, (long)dd, (long)(TXT_COLS - cx));
            s += n; cx += (int)n;
            if (cx >= TXT_COLS) { cx--; advance(); }      /* last column drawn: the same wrap/clip as a single character */
        } else
            txt_putc((u8)*s++);
    }
}

void txt_print_at(int col, int row, const char *s) { txt_at(col, row); txt_puts(s); }

void txt_clear_rows(int row0, int nrows, u8 color)
{
    if (row0 < 0) { nrows += row0; row0 = 0; }
    if (row0 + nrows > TXT_ROWS) nrows = TXT_ROWS - row0;
    if (nrows <= 0) return;
    gfx_fill_lines(row0 * 8, nrows * 8, color);          /* fast fill of whole lines */
}
