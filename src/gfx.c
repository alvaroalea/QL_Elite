#include "platform.h"

/* Modo 8: cada palabra = 4 pixeles. Para el pixel k (0..3, izq a der), sea s=2*(3-k):
   byte0: bit s+1 = Verde, bit s = Flash ; byte1: bit s+1 = Rojo, bit s = Azul. */
#define OFF(x,y) ((long)(y) * SCR_STRIDE + (((x) >> 2) << 1))

static int iabs(int v) { return v < 0 ? -v : v; }

void gfx_pixel(int x, int y, u8 c)
{
    u8 *p;
    int s;
    u8 m;
    if ((unsigned)x >= SCR_W || (unsigned)y >= SCR_H) return;
    p = screen + OFF(x, y);
    s = 2 * (3 - (x & 3));
    m = (u8)(3 << s);
    p[0] = (u8)((p[0] & ~m) | ((c & 4) ? (2 << s) : 0));
    p[1] = (u8)((p[1] & ~m) | ((c & 2) ? (2 << s) : 0) | ((c & 1) ? (1 << s) : 0));
}

#ifdef ASM_LINE
extern void ql_xpix(long x, long y, long c, long scr);                 /* ql_ell.asm */
#else
static void xor1(int x, int y, u8 c)
{
    u8 *p;
    int s;
    if ((unsigned)x >= SCR_W || (unsigned)y >= SCR_H) return;
    p = screen + OFF(x, y);
    s = 2 * (3 - (x & 3));
    p[0] ^= (u8)((c & 4) ? (2 << s) : 0);
    p[1] ^= (u8)(((c & 2) ? (2 << s) : 0) | ((c & 1) ? (1 << s) : 0));
}
#endif

/* Desplazamiento vertical de lo dibujado en XOR: el panel del original esta en las lineas 136..191; con la vista de vuelo de 160
   lineas baja a 160..215 y sus rutinas (DIALS, radar, bombillas...) siguen trabajando en las coordenadas del original. */
static int yoff = 0;
void gfx_yoff_set(int off) { yoff = off; }

void gfx_xor_pixel(int x, int y, u8 c)
{
#ifdef ASM_LINE
    y += yoff;
    if ((unsigned)x > 255 || y < 0 || y > 255) return;
    ql_xpix((long)x, (long)y, (long)c, (long)screen);
#else
    xor1(x, y + yoff, c);
#endif
}

u8 gfx_get(int x, int y)
{
    u8 *p;
    int s;
    if ((unsigned)x >= SCR_W || (unsigned)y >= SCR_H) return 0;
    p = screen + OFF(x, y);
    s = 2 * (3 - (x & 3));
    return (u8)((((p[0] >> (s + 1)) & 1) << 2) | (((p[1] >> (s + 1)) & 1) << 1) | ((p[1] >> s) & 1));
}

/* Relleno rapido de n lineas completas (128 bytes cada una) desde la linea y con el color c. En el QL se escribe de 4 en 4 bytes y
   desenrollado de 8 en 8 palabras largas (68008: unas 20 veces mas rapido que poner pixel a pixel); el PC usa bytes para no depender
   de reglas de alias. La pantalla y las lineas estan en direcciones pares. */
void gfx_fill_lines(int y, int n, u8 c)
{
    u8 g = (c & 4) ? 0xAA : 0;
    u8 rb = (u8)(((c & 2) ? 0xAA : 0) | ((c & 1) ? 0x55 : 0));
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
        long cnt = (long)n * 32L;                       /* palabras largas: 32 por linea */
        while (cnt >= 8) {
            p[0] = v; p[1] = v; p[2] = v; p[3] = v; p[4] = v; p[5] = v; p[6] = v; p[7] = v;
            p += 8; cnt -= 8;
        }
    }
#endif
}

/* Copia rapida de n bytes (multiplo de 4) a la pantalla o desde ella; por palabras largas si las dos direcciones son pares */
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
extern void ql_clearcells(long col, long row, long ncols, long nrows, long scr);
#endif

void gfx_hline_xor(int x0, int x1, int y, u8 c)
{
    int x, t;
    if (x0 > x1) { t = x0; x0 = x1; x1 = t; }
#ifdef ASM_LINE
    y += yoff;                                           /* como gfx_xor_pixel */
    if (y < 0 || y > 255 || x1 < 0 || x0 > 255) return;
    if (x0 < 0) x0 = 0;
    if (x1 > 255) x1 = 255;
    ql_hline((long)x0, (long)x1, (long)y, (long)c, (long)screen);
#else
    for (x = x0; x <= x1; x++) gfx_xor_pixel(x, y, c);
#endif
    (void)x;
}

/* Columna de puntos (x, y0..y1) en XOR (una columna de un punto de ancho) */
void gfx_vline_xor(int x, int y0, int y1, u8 c)
{
    int y, t;
    if (y0 > y1) { t = y0; y0 = y1; y1 = t; }
#ifdef ASM_LINE
    y0 += yoff; y1 += yoff;                              /* como gfx_xor_pixel */
    if (x < 0 || x > 255 || y1 < 0 || y0 > 255) return;
    if (y0 < 0) y0 = 0;
    if (y1 > 255) y1 = 255;
    ql_vline((long)x, (long)y0, (long)y1, (long)c, (long)screen);
#else
    for (y = y0; y <= y1; y++) gfx_xor_pixel(x, y, c);
#endif
    (void)y;
}

/* HLOIN del original (Apple II): X1 y X2 se enmascaran a pares (granularidad de 2 puntos del modo hires), se ordenan y la linea
   cubre de X1 a X2-1; si quedan iguales no dibuja nada. Siempre en XOR. */
void gfx_hloin(int x1, int x2, int y, u8 c)
{
    int t;
    x1 &= 0xFE; x2 &= 0xFE;
    if (x1 == x2) return;
    if (x1 > x2) { t = x1; x1 = x2; x2 = t; }
    gfx_hline_xor(x1, x2 - 1, y, c);
}

/* CPIX del original: un "pixel" de color de Elite = 2 puntos del Apple II de ancho (aqui x y x+1), en XOR */
void gfx_cpix(int x, int y, u8 c) { gfx_hline_xor(x, x + 1, y, c); }       /* dos puntos de ancho (x, x + 1) */

/* VLOIN del original: linea vertical de pixeles CPIX de y1 a y2 (en cualquier orden), ambos extremos incluidos */
void gfx_vloin(int x, int y1, int y2, u8 c)
{
    int t;
    if (y1 > y2) { t = y1; y1 = y2; y2 = t; }
    gfx_vline_xor(x, y1, y2, c);                         /* CPIX = 2 puntos de ancho: las columnas x y x + 1 */
    gfx_vline_xor(x + 1, y1, y2, c);
}

/* PIXEL del original: un "punto" de polvo/compas en XOR. El ancho y las lineas dependen de la distancia ZZ:
   ZZ >= 128: 2 puntos en una linea; 80 <= ZZ < 128: 3 puntos; ZZ < 80: 3 puntos en esta linea y en la de encima. */
/* PIXEL del original tal cual (3 puntos de ancho, dos lineas si zz < 80): lo usa la brujula y lo verifica test_lines */
void gfx_dust_6502(int x, int y, u8 zz)
{
    int w = (zz & 0x80) ? 2 : 3, i;
    for (i = 0; i < w; i++) gfx_xor_pixel(x + i, y, C_WHITE);
    if (zz < 80) for (i = 0; i < w; i++) gfx_xor_pixel(x + i, y - 1, C_WHITE);
}

/* Polvo estelar del juego (solo estetico): el tamano depende de la distancia ZZ: lejos (ZZ >= 128) 1 punto; entre 80 y 127, 1 de
   ancho por 2 de alto; cerca (ZZ < 80) 2 de ancho por 3 de alto (los puntos del QL son mas anchos que altos). La esquina inferior
   izquierda es (x, y) y crece hacia arriba y a la derecha. Columnas con gfx_vline_xor (ensamblador en el QL). */
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
    if (x <= 0 && x + w >= SCR_W) { gfx_fill_lines(y, h, c); return; }       /* ancho completo: relleno rapido */
    for (j = 0; j < h; j++) gfx_hline(x, x + w - 1, y + j, c);
}

/* Elipse de punto medio (Bresenham, variante entera de A. Zingl): centro (cx, cy), semiejes rx y ry, contorno de un punto en XOR.
   Solo se recorre un cuadrante (sumas y comparaciones; las multiplicaciones son de la preparacion) y se refleja en los cuatro. Los
   puntos de los ejes se dibujan UNA vez (en XOR dos veces se borrarian). Es determinista: volver a llamarla con los mismos datos
   borra lo dibujado, asi que no hace falta guardar los puntos. Todo en long (en c68 int es de 16 bits): con rx <= 300 y ry <= 360 los
   terminos de error caben en 32 bits. Recorta al rectangulo (clx0, cly0)-(clx1, cly1); si cabe entero va sin comprobar cada punto. */
static u8 el_col;
static int el_x0, el_y0, el_x1, el_y1, el_clip;

static void el_plot(int x, int y)
{
    if (el_clip && (x < el_x0 || x > el_x1 || y < el_y0 || y > el_y1)) return;
    gfx_xor_pixel(x, y, el_col);
}

void gfx_ellipse_xor(int cx, int cy, int rx, int ry, u8 c, int clx0, int cly0, int clx1, int cly1)
{
    long a, b, dx, dy, err, e2, a8, b8;
    int x0, x1, y0, y1;
    if (rx < 0) rx = -rx;
    if (ry < 0) ry = -ry;
    if (cx + rx < clx0 || cx - rx > clx1 || cy + ry < cly0 || cy - ry > cly1) return;       /* fuera del todo */
    el_col = c; el_x0 = clx0; el_y0 = cly0; el_x1 = clx1; el_y1 = cly1;
    el_clip = !(cx - rx >= clx0 && cx + rx <= clx1 && cy - ry >= cly0 && cy + ry <= cly1);
    if (rx == 0 || ry == 0) {                          /* degenerada: un punto o un segmento */
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
    {                                                    /* bucle completo en ensamblador (ql_ell.asm): bloque de parametros */
        static long blk[18];
        blk[0] = a8; blk[1] = b8;
        blk[2] = (c & 4) ? 0xAAL : 0L;
        blk[3] = (long)(((c & 2) ? 0xAA : 0) | ((c & 1) ? 0x55 : 0));
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
    if (y0 - y1 < 2 * ry) while (y0 - y1 <= 2 * ry) {   /* elipses estrechas: el bucle acabo antes de llegar arriba; termina la parte vertical */
        el_plot(x0 - 1, y0);
        if (x1 + 1 != x0 - 1) el_plot(x1 + 1, y0);
        if (y1 != y0) { el_plot(x0 - 1, y1); if (x1 + 1 != x0 - 1) el_plot(x1 + 1, y1); }
        y0++; y1--;
    }
}

/* Semianchos de la elipse fila a fila (para rellenarla): el mismo recorrido de punto medio que gfx_ellipse_xor, sin dibujar; en cada
   fila dy (0 = la del centro) el primer punto del recorrido es el mas a la derecha. half[dy] = x - cx para dy = 0..ry (ry < 512). */
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
        int x0 = -rx, xe = rx;                          /* x0, x1 relativos al centro */
        x1 = xe;
        do {
            if (y0 != last && y0 <= ry) { half[y0] = (u8)(x1 > 255 ? 255 : x1); last = y0; }
            e2 = 2L * err;
            if (e2 <= dy) { y0++; dy += a8; err += dy; }
            if (e2 >= dx || 2L * err > dy) { x0++; x1--; dx += b8; err += dx; }
        } while (x0 <= x1);
        for (d = y0; d <= ry; d++) {                    /* cola de las estrechas (como en gfx_ellipse_xor) */
            int v = x1 + 1 > 255 ? 255 : (x1 + 1 < 0 ? 0 : x1 + 1);
            if (d > last || half[d] < v) half[d] = (u8)v;
        }
    }
}

/* Tabla de tramos del contorno de una elipse de semiejes rx, ry (ry <= 303): para cada distancia dy = 0..ry del centro, tab[2 * dy] = lo y
   tab[2 * dy + 1] = hi: el contorno de esa fila es cx + lo .. cx + hi (y su reflejo a la izquierda; con lo = 0, un unico tramo cx - hi .. cx + hi).
   Sale de la tabla de semianchos half[] (gfx_ellipse_rows): fila dy < ry: de half[dy + 1] + 1 a half[dy] si half[dy + 1] < half[dy], si no el
   punto half[dy]; fila dy = ry: de 0 a half[ry]. Es el mismo contorno que dibuja gfx_ellipse_xor (punto medio): comprobado punto a punto en
   make test_ellrows. Mide 2 * (ry + 1) bytes. */
void gfx_ell_spans(int rx, int ry, u8 *tab)
{
    static u8 half[304];
    int dy;
    gfx_ellipse_rows(rx, ry, half);
    for (dy = 0; dy <= ry; dy++) {
        tab[2 * dy + 1] = half[dy];
        tab[2 * dy] = (u8)((dy == ry) ? 0 : ((half[dy + 1] < half[dy]) ? half[dy + 1] + 1 : half[dy]));
    }
}

/* Tramo de la fila y (tabla de gfx_ell_spans; cy = centro, ry = semialto, -1 = no hay elipse): 0 si no toca la fila, si no 1 con *lo y *hi */
static int ell_span(int y, int cy, int ry, const u8 *tab, int *lo, int *hi)
{
    int dy = y - cy;
    if (dy < 0) dy = -dy;
    if (ry < 0 || dy > ry) return 0;
    *lo = tab[2 * dy]; *hi = tab[2 * dy + 1];
    return 1;
}

static void ell_xor_spans(int cx, int lo, int hi, int y, u8 c)
{
    if (lo == 0) { gfx_hline_xor(cx - hi, cx + hi, y, c); return; }
    gfx_hline_xor(cx - hi, cx - lo, y, c);
    gfx_hline_xor(cx + lo, cx + hi, y, c);
}

/* Cambia una elipse (centro ocx, ocy, semialto ory, tabla de tramos otab; ory = -1 si no habia ninguna) por otra (ncx, ncy, nry, ntab; nry = -1 para
   borrarla), recortadas a x 0..255 e y 0..cly1, FILA A FILA: en cada linea se borra lo viejo y se dibuja lo nuevo seguidos (y se salta si
   no cambia), asi la elipse nunca desaparece entera. El resultado en pantalla es el mismo que gfx_ellipse_xor(vieja) + gfx_ellipse_xor(nueva)
   (XOR). yoff debe ser 0. En el QL va en ensamblador (ql_ellrows.asm). */
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
        blk[10] = (c & 4) ? 0xAAL : 0L;
        blk[11] = (long)(((c & 2) ? 0xAA : 0) | ((c & 1) ? 0x55 : 0));
        blk[12] = (long)screen;
        ql_ellrows(blk);
        return;
    }
#else
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

/* Contorno de rectangulo (centro, semiancho hw, semialto hh) en XOR: cada punto una sola vez, recortado */
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
    {                                                    /* entero en ensamblador (ql_frame, ql_ell.asm): bloque de parametros */
        static long blk[11];
        blk[0] = cx - hw; blk[1] = cy - hh; blk[2] = cx + hw; blk[3] = cy + hh;
        blk[4] = clx0; blk[5] = cly0; blk[6] = clx1; blk[7] = cly1;
        blk[8] = (c & 4) ? 0xAAL : 0L;
        blk[9] = (long)(((c & 2) ? 0xAA : 0) | ((c & 1) ? 0x55 : 0));
        blk[10] = (long)screen;
        ql_frame(blk);
    }
#else
    {
        int y;
        fr_row(cx - hw, cx + hw, cy - hh, c, clx0, cly0, clx1, cly1);                 /* arriba */
        if (hh > 0) fr_row(cx - hw, cx + hw, cy + hh, c, clx0, cly0, clx1, cly1);     /* abajo */
        for (y = cy - hh + 1; y <= cy + hh - 1; y++) {                                 /* lados, sin las esquinas */
            if (y < cly0 || y > cly1) continue;
            if (cx - hw >= clx0 && cx - hw <= clx1) gfx_xor_pixel(cx - hw, y, c);
            if (hw > 0 && cx + hw >= clx0 && cx + hw <= clx1) gfx_xor_pixel(cx + hw, y, c);
        }
    }
#endif
}

/* Vacia (negro) un rectangulo de celdas de texto: ncols columnas desde col, nrows filas desde row (0..31; se recorta). Los textos que
   cambian de longitud no se borran escribiendo espacios: se calcula el rectangulo sobrante y se vacia de una vez. */
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
