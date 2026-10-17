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

void gfx_xor_pixel(int x, int y, u8 c)
{
    u8 *p;
    int s;
    if ((unsigned)x >= SCR_W || (unsigned)y >= SCR_H) return;
    p = screen + OFF(x, y);
    s = 2 * (3 - (x & 3));
    p[0] ^= (u8)((c & 4) ? (2 << s) : 0);
    p[1] ^= (u8)(((c & 2) ? (2 << s) : 0) | ((c & 1) ? (1 << s) : 0));
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

void gfx_clear(u8 c)
{
    long i;
    u8 g = (c & 4) ? 0xAA : 0;
    u8 r = (c & 2) ? 0xAA : 0;
    u8 b = (c & 1) ? 0x55 : 0;
    for (i = 0; i < SCR_BYTES; i += 2) {
        screen[i] = g;
        screen[i + 1] = (u8)(r | b);
    }
}

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

void gfx_hline_xor(int x0, int x1, int y, u8 c)
{
    int x, t;
    if (x0 > x1) { t = x0; x0 = x1; x1 = t; }
    for (x = x0; x <= x1; x++) gfx_xor_pixel(x, y, c);
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
void gfx_cpix(int x, int y, u8 c)
{
    gfx_xor_pixel(x, y, c);
    gfx_xor_pixel(x + 1, y, c);
}

/* VLOIN del original: linea vertical de pixeles CPIX de y1 a y2 (en cualquier orden), ambos extremos incluidos */
void gfx_vloin(int x, int y1, int y2, u8 c)
{
    int t;
    if (y1 > y2) { t = y1; y1 = y2; y2 = t; }
    for (t = y1; t <= y2; t++) gfx_cpix(x, t, c);
}

/* PIXEL del original: un "punto" de polvo/compas en XOR. El ancho y las lineas dependen de la distancia ZZ:
   ZZ >= 128: 2 puntos en una linea; 80 <= ZZ < 128: 3 puntos; ZZ < 80: 3 puntos en esta linea y en la de encima. */
void gfx_dust(int x, int y, u8 zz)
{
    int w = (zz & 0x80) ? 2 : 3, i;
    for (i = 0; i < w; i++) gfx_xor_pixel(x + i, y, C_WHITE);
    if (zz < 80) for (i = 0; i < w; i++) gfx_xor_pixel(x + i, y - 1, C_WHITE);
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
    for (j = 0; j < h; j++) gfx_hline(x, x + w - 1, y + j, c);
}
