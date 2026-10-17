/* Demo: dibuja varias naves con LL9 (proyeccion, caras ocultas y recorte) usando XOR sobre la pantalla.
   En el PC dibuja un cuadro y termina; en el QL espera una tecla. Sin coma flotante: tabla de senos entera. */
#include "ship3d.h"
#include "text.h"

#define ANG 64                                 /* pasos por vuelta */
static const int sin64[ANG / 4 + 1] = { 0, 3212, 6393, 9512, 12540, 15447, 18205, 20788, 23170, 25330, 27246, 28899, 30274, 31357, 32138, 32610, 32768 };   /* sin(i*90/16 grados) * 32768 */ /* x 32768/... se usa en 1/32768 */

/* seno y coseno en punto fijo (1.0 = 32768) para el angulo a (0..63) */
static int isin(int a)
{
    int q, r;
    a &= ANG - 1; q = a / (ANG / 4); r = a % (ANG / 4);
    /* tabla de un cuarto de vuelta con 16 pasos: sin64[i] = sin(i*90/16 grados) * 32768 */
    if (q == 0) return sin64[r] * 1;
    if (q == 1) return sin64[ANG / 4 - r];
    if (q == 2) return -sin64[r];
    return -sin64[ANG / 4 - r];
}
static int icos(int a) { return isin(a + ANG / 4); }

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
static void set_orient(int ay, int ax)
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

static void set_pos(int x, int y, int z)               /* coordenadas en unidades de 256 (hi) con signo */
{
    inwk[I_XLO] = 0; inwk[I_XHI] = (u8)(x < 0 ? -x : x); inwk[I_XSG] = (u8)(x < 0 ? 0x80 : 0);
    inwk[I_YLO] = 0; inwk[I_YHI] = (u8)(y < 0 ? -y : y); inwk[I_YSG] = (u8)(y < 0 ? 0x80 : 0);
    inwk[I_ZLO] = 0; inwk[I_ZHI] = (u8)z; inwk[I_ZSG] = 0;
}

static u8 heaps[6][256];

int main(void)
{
    static const u8 types[6] = { SHIPT_COBRA_MK_3, SHIPT_PYTHON, SHIPT_VIPER, SHIPT_CORIOLIS, SHIPT_ASP_MK_2, SHIPT_THARGOID };
    static const int px[6] = { -3, 0, 3, -4, 0, 4 };
    static const int py[6] = { -1, -1, -1, 1, 1, 1 };
    static const int pz[6] = { 7, 12, 6, 24, 7, 14 };
    int i;
    plat_init();
    gfx_clear(C_BLACK);
    state_clear();
    ztype = 0;
    for (i = 0; i < 6; i++) {
        inwk[I_XLO] = 0;
        set_pos(px[i], py[i], pz[i]);
        set_orient(6 + 3 * i, 5 + i);
        inwk[I_STATE] = 0; inwk[I_NEWB] = 0;
        ztype = types[i];
        zxx0 = SHIP_BP(types[i]);
        zxx19 = heaps[i]; zxx19[0] = 0;
        zinf = state_slot(i);
        ship_ll9();                                   /* dibuja */
    }
    gfx_rect(0, 136, 256, 1, C_RED);
    txt_init();
    txt_at(0, 18); txt_puts("LL9: 6 NAVES, XOR");
    plat_present();
    plat_shutdown();
    return 0;
}
