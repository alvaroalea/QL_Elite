#include "text.h"
#include "font.h"

/* Celda de 8x8 = 4 bytes por linea: [0..1] pixeles 0-3, [2..3] pixeles 4-7 (ver gfx.c).
   Un nibble de la fuente (bit3 = pixel izquierdo) se expande a los bits "altos" de cada par:
   pixel k -> bit 2*(3-k)+1. Verde va en el byte par, rojo (bit alto) y azul (bit bajo) en el impar.

   Cada linea de un glifo se escribe de una vez, como palabra larga (4 bytes):
     fm[b]  mascara de pixeles de la linea de fuente b: los dos bits de cada pixel puestos (tabla fija, se crea en txt_init)
     papel: pp = color del papel en los 4 bytes;  dd = tinta XOR papel;   opaco:  linea = pp ^ (fm & dd)
     transparente: ii = color de la tinta;  linea = pantalla ^ ((pantalla ^ ii) & fm)
   Son sumas de bits sobre palabras largas, asi que da igual el orden de los bytes dentro de la palabra. */

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
static unsigned short fi[96][8];   /* fuente x 4 = desplazamiento en bytes dentro de fm (la usa ql_puts) */
static u32 pp, dd, ii;
static u8 ink = C_WHITE;
static int paper = C_BLACK;
static int cx = 0, cy = 0;
static int nowrap = 0;          /* 1: el texto que se sale por la derecha se pierde en vez de seguir en la fila de abajo */
static int clipped = 0;         /* ya se llego a la ultima columna con nowrap: se ignora lo que siga (hasta el siguiente txt_at) */

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
    gfx_mode_hook = set_masks;                       /* al cambiar de modo los mismos colores dan otras mascaras */
}

/* Colores en formato de pantalla: byte par = verde (bit 2), impar = rojo (bit 1) y azul (bit 0); en modo 4 los codigos de gfx_ccode
   (verde = $FF en el byte par, rojo = $FF en el impar: cada punto del texto son 2 puntos fisicos) */
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

/* Papel opaco (lo habitual): tira de celdas seguidas sin leer la pantalla. Dibuja hasta maxn caracteres de s mientras sean imprimibles
   (32..127) y devuelve cuantos; con ASM_TEXT es ql_puts (ql_text.asm), si no su version en C (la misma formula) */
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

/* Papel transparente (nombres de las cartas, panel sin fichero): respeta lo que haya dibujado debajo */
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

static void advance(void)                         /* tras dibujar una celda: siguiente columna, con ajuste de linea o recorte */
{
    if (++cx >= TXT_COLS) {
        if (nowrap) { cx = TXT_COLS - 1; clipped = 1; }
        else { cx = 0; if (cy < TXT_ROWS - 1) cy++; }
    }
}

void txt_putc(int ch)
{
    if (ch >= 32 && ch <= 127 && !clipped) {          /* lo habitual: un caracter imprimible */
        draw_glyph(ch);
        advance();
    }
    else if (ch == '\n') { cx = 0; if (cy < TXT_ROWS - 1) cy++; }
    else if (ch == '\r') { cx = 0; }
    else if (ch == '\b') { if (cx > 0) cx--; }
    else if (clipped) { }
    else {
        draw_glyph(ch);                               /* fuera de rango: '?' */
        advance();
    }
}

void txt_nowrap(int on) { nowrap = on ? 1 : 0; clipped = 0; }

/* Con papel opaco, las tiras de caracteres imprimibles que caben en la fila se dibujan de una vez (ql_puts); el resto (control, fuera de
   rango, texto recortado, papel transparente) va caracter a caracter por txt_putc */
void txt_puts(const char *s)
{
    long n;
    while (*s) {
        if (paper >= 0 && !clipped && (u8)*s >= 32 && (u8)*s <= 127) {
            n = ql_puts(screen + ((long)cy << 10) + ((long)cx << 2), (const u8 *)s, &fi[0][0], fm, (long)pp, (long)dd, (long)(TXT_COLS - cx));
            s += n; cx += (int)n;
            if (cx >= TXT_COLS) { cx--; advance(); }      /* ultima columna dibujada: el mismo ajuste/recorte que un caracter suelto */
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
    gfx_fill_lines(row0 * 8, nrows * 8, color);          /* relleno rapido de lineas completas */
}
