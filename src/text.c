#include "text.h"
#include "font.h"

/* Celda de 8x8 = 4 bytes por linea: [0..1] pixeles 0-3, [2..3] pixeles 4-7 (ver gfx.c).
   Un nibble de la fuente (bit3 = pixel izquierdo) se expande a los bits "altos" de cada par:
   pixel k -> bit 2*(3-k)+1. Verde va en el byte par, rojo (bit alto) y azul (bit bajo) en el impar. */

static u8 ex[16];
static u8 ink = C_WHITE;
static int paper = C_BLACK;
static int cx = 0, cy = 0;
static int cur_on = 0;          /* cursor activado */
static int cur_vis = 0;         /* bloque dibujado ahora mismo */
static u8 cur_col = C_WHITE;
static int nowrap = 0;          /* 1: el texto que se sale por la derecha se pierde en vez de seguir en la fila de abajo */
static int clipped = 0;         /* ya se llego a la ultima columna con nowrap: se ignora lo que siga (hasta el siguiente txt_at) */

void txt_init(void)
{
    int n, k;
    for (n = 0; n < 16; n++) {
        ex[n] = 0;
        for (k = 0; k < 4; k++)
            if (n & (8 >> k)) ex[n] = (u8)(ex[n] | (2 << (2 * (3 - k))));
    }
    ink = C_WHITE; paper = C_BLACK; cx = cy = 0; cur_on = cur_vis = 0; cur_col = C_WHITE;
}

static u8 *cell(int col, int row)
{
    return screen + (long)row * 8 * SCR_STRIDE + (long)col * 4;
}

static void put4(u8 *p, int n)
{
    u8 e = ex[n];
    u8 pm = (u8)(e | (e >> 1));
    u8 ne;
    if (paper >= 0) {
        ne = (u8)(~e & 0xAA);
        p[0] = (u8)(((ink & 4) ? e : 0) | ((paper & 4) ? ne : 0));
        p[1] = (u8)(((ink & 2) ? e : 0) | ((ink & 1) ? (e >> 1) : 0) |
                    ((paper & 2) ? ne : 0) | ((paper & 1) ? (ne >> 1) : 0));
    } else {
        p[0] = (u8)((p[0] & ~pm) | ((ink & 4) ? e : 0));
        p[1] = (u8)((p[1] & ~pm) | ((ink & 2) ? e : 0) | ((ink & 1) ? (e >> 1) : 0));
    }
}

static void draw_glyph(int col, int row, int ch)
{
    const u8 *g;
    u8 *p;
    int r;
    if (ch < 32 || ch > 127) ch = '?';
    g = font8x8[ch - 32];
    p = cell(col, row);
    for (r = 0; r < 8; r++) {
        put4(p, g[r] >> 4);
        put4(p + 2, g[r] & 15);
        p += SCR_STRIDE;
    }
}

static void xor_block(void)
{
    u8 gm = (cur_col & 4) ? 0xAA : 0;
    u8 rb = (u8)(((cur_col & 2) ? 0xAA : 0) | ((cur_col & 1) ? 0x55 : 0));
    u8 *p = cell(cx, cy);
    int r;
    for (r = 0; r < 8; r++) {
        p[0] ^= gm; p[1] ^= rb; p[2] ^= gm; p[3] ^= rb;
        p += SCR_STRIDE;
    }
}

static void cur_hide(void) { if (cur_vis) { xor_block(); cur_vis = 0; } }
static void cur_show(void) { if (cur_on && !cur_vis) { xor_block(); cur_vis = 1; } }

void txt_color(u8 i, int p) { ink = (u8)(i & 7); paper = (p < 0) ? TXT_TRANSPARENT : (p & 7); }

void txt_at(int col, int row)
{
    cur_hide();
    clipped = 0;
    cx = col < 0 ? 0 : (col >= TXT_COLS ? TXT_COLS - 1 : col);
    cy = row < 0 ? 0 : (row >= TXT_ROWS ? TXT_ROWS - 1 : row);
    cur_show();
}

int txt_col(void) { return cx; }
int txt_row(void) { return cy; }

void txt_putc(int ch)
{
    cur_hide();
    if (ch == '\n') { cx = 0; if (cy < TXT_ROWS - 1) cy++; }
    else if (ch == '\r') { cx = 0; }
    else if (ch == '\b') { if (cx > 0) cx--; }
    else if (clipped) { }
    else {
        draw_glyph(cx, cy, ch);
        if (++cx >= TXT_COLS) {
            if (nowrap) { cx = TXT_COLS - 1; clipped = 1; }
            else { cx = 0; if (cy < TXT_ROWS - 1) cy++; }
        }
    }
    cur_show();
}

void txt_nowrap(int on) { nowrap = on ? 1 : 0; clipped = 0; }

void txt_puts(const char *s) { while (*s) txt_putc((u8)*s++); }

void txt_print_at(int col, int row, const char *s) { txt_at(col, row); txt_puts(s); }

void txt_clear_rows(int row0, int nrows, u8 color)
{
    cur_hide();
    if (row0 < 0) { nrows += row0; row0 = 0; }
    if (row0 + nrows > TXT_ROWS) nrows = TXT_ROWS - row0;
    if (nrows <= 0) { cur_show(); return; }
    gfx_fill_lines(row0 * 8, nrows * 8, color);          /* relleno rapido de lineas completas */
    cur_show();
}

void txt_cursor(int on)
{
    cur_on = on;
    if (on) cur_show(); else cur_hide();
}

void txt_cursor_blink(void)
{
    if (!cur_on) return;
    if (cur_vis) cur_hide(); else cur_show();
}

void txt_cursor_color(u8 c) { cur_hide(); cur_col = (u8)(c & 7); cur_show(); }
