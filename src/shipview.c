/* Utilidades para dibujar una nave con LL9 a partir de angulos y una posicion (pantalla de titulo, demos).
   Sin coma flotante: tabla de senos entera. */
#include "shipview.h"

#define ANG 64                                 /* pasos por vuelta */
static const long sin64[ANG / 4 + 1] = { 0, 3212, 6393, 9512, 12540, 15447, 18205, 20788, 23170, 25330, 27246, 28899, 30274, 31357, 32138, 32610, 32768 };   /* sin(i*90/16 grados) * 32768 */ /* x 32768/... se usa en 1/32768 */

/* seno y coseno en punto fijo (1.0 = 32768) para el angulo a (0..63). long: 32768 no cabe en el int de 16 bits de c68 */
static long isin(int a)
{
    int q, r;
    a &= ANG - 1; q = a / (ANG / 4); r = a % (ANG / 4);
    /* tabla de un cuarto de vuelta con 16 pasos: sin64[i] = sin(i*90/16 grados) * 32768 */
    if (q == 0) return sin64[r] * 1;
    if (q == 1) return sin64[ANG / 4 - r];
    if (q == 2) return -sin64[r];
    return -sin64[ANG / 4 - r];
}
static long icos(int a) { return isin(a + ANG / 4); }

/* vector en formato Elite (lo, hi con signo-magnitud, 96 = 1.0) a partir de un valor en 1/32768 */
static void put_vec(u8 *p, long v)
{
    long mag = v < 0 ? -v : v;
    mag = mag * 96L * 256L / 32768L;
    if (mag > 0x7FFF) mag = 0x7FFF;
    p[0] = (u8)(mag & 255);
    p[1] = (u8)((mag >> 8) | (v < 0 ? 0x80 : 0));
}

/* matriz de orientacion: guinada ay y cabeceo ax sobre los ejes de la nave */
void shipview_orient(int ay, int ax)
{
    long cy = icos(ay), sy = isin(ay), cx = icos(ax), sx = isin(ax);
    long side[3], roof[3], nose[3], t[3];
    int i;
    /* base inicial: side = x, roof = y, nose = z; se gira por Ry y luego por Rx (1.0 = 32768) */
    long b[3][3] = { { 32768L, 0, 0 }, { 0, 32768L, 0 }, { 0, 0, 32768L } };
    for (i = 0; i < 3; i++) {
        t[0] = (b[i][0] * cy + b[i][2] * sy) / 32768L;
        t[1] = b[i][1];
        t[2] = (-b[i][0] * sy + b[i][2] * cy) / 32768L;
        b[i][0] = t[0];
        b[i][1] = (t[1] * cx - t[2] * sx) / 32768L;
        b[i][2] = (t[1] * sx + t[2] * cx) / 32768L;
    }
    for (i = 0; i < 3; i++) { side[i] = b[0][i]; roof[i] = b[1][i]; nose[i] = b[2][i]; }
    for (i = 0; i < 3; i++) {
        put_vec(inwk + I_NOSEV + 2 * i, nose[i]);
        put_vec(inwk + I_ROOFV + 2 * i, roof[i]);
        put_vec(inwk + I_SIDEV + 2 * i, side[i]);
    }
}

void shipview_pos(int x, int y, int z)               /* coordenadas en unidades de 256 (hi) con signo */
{
    inwk[I_XLO] = 0; inwk[I_XHI] = (u8)(x < 0 ? -x : x); inwk[I_XSG] = (u8)(x < 0 ? 0x80 : 0);
    inwk[I_YLO] = 0; inwk[I_YHI] = (u8)(y < 0 ? -y : y); inwk[I_YSG] = (u8)(y < 0 ? 0x80 : 0);
    inwk[I_ZLO] = 0; inwk[I_ZHI] = (u8)z; inwk[I_ZSG] = 0;
}


void shipview_draw(u8 type, int x, int y, int z, int ay, int ax, u8 *heap, u8 *slot, int fresh)
{
    shipview_draw16(type, x, y, (long)z * 256L, ay, ax, heap, slot, fresh);
}

int shipview_radius(u8 type)                         /* mayor coordenada de los vertices del plano (el Cobra Mk III da ~65) */
{
    const u8 *bp = SHIP_BP(type), *v = bp + 20;
    int i, n = bp[8], r = 0, k;
    for (i = 0; i + 5 < n; i += 6)
        for (k = 0; k < 3; k++)
            if (v[i + k] > r) r = v[i + k];
    return r;
}

void shipview_draw16(u8 type, int x, int y, long z, int ay, int ax, u8 *heap, u8 *slot, int fresh)
{
    shipview_pos(x, y, (int)(z >> 8));
    inwk[I_ZLO] = (u8)(z & 255);
    shipview_orient(ay, ax);
    inwk[I_NEWB] = 0;
    /* LL9 solo borra las lineas viejas del monton si el bit 3 de INWK+31 esta puesto (lo pone al terminar de dibujar):
       ese estado va con la nave (slot[I_STATE]); ponerlo a 0 en cada fotograma dejaba todas las lineas viejas en pantalla */
    inwk[I_STATE] = fresh ? 0 : slot[I_STATE];
    ztype = type;
    zxx0 = SHIP_BP(type);
    zxx19 = heap;
    if (fresh) zxx19[0] = 0;                         /* sin esto el monton conserva las lineas viejas y LL9 las borra (XOR) */
    zinf = slot;
    ship_ll9();
    slot[I_STATE] = inwk[I_STATE];
}
