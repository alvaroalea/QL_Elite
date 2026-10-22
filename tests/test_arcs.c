/* Meridianos y crater del planeta (vista nativa): con orientaciones al azar, dibujar y quitar deja la pantalla vacia, mover el planeta
   no deja restos, las figuras se dibujan solo en la cara visible y el volcado de pantalla (argumento) sirve para verlas. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "platform.h"
#include "state.h"
#include "ships.h"
#include "ship3d.h"
#include "sun.h"
#include "emath.h"

static int fails, total;
static void check(int ok, const char *m) { total++; if (!ok) { fails++; printf("FALLO: %s\n", m); } }
static int blank(void) { int i; for (i = 0; i < 32768; i++) if (screen[i]) return 0; return 1; }
static int count_on(void) { int x, y, n = 0; for (y = 0; y < 160; y++) for (x = 0; x < 256; x++) if (gfx_get(x, y)) n++; return n; }

static void put_vec(int off, double x, double y, double z)
{
    double v[3]; int i; long c;
    v[0] = x; v[1] = y; v[2] = z;
    for (i = 0; i < 3; i++) {
        c = (long)(v[i] * 24576.0); if (c < 0) c = -c;
        inwk[off + 2 * i] = (u8)(c & 255); inwk[off + 2 * i + 1] = (u8)((c >> 8) & 0x7F) | (v[i] < 0 ? 0x80 : 0);
    }
}
/* planeta en (x, y) del espacio del original con orientacion de angulos a, b, c */
static void put_planet(int type, int xhi, int xs, int yhi, int ys, int zhi, double a, double b, double c)
{
    double ca = cos(a), sa = sin(a), cb = cos(b), sb = sin(b), cc = cos(c), sc = sin(c);
    double n[3], r[3], s[3];
    int i;
    ztype = (u8)type;
    for (i = 0; i < 37; i++) inwk[i] = 0;
    inwk[1] = (u8)xhi; inwk[2] = (u8)xs; inwk[4] = (u8)yhi; inwk[5] = (u8)ys; inwk[8] = (u8)zhi;
    n[0] = ca * cb; n[1] = sa * cb; n[2] = -sb;
    s[0] = ca * sb * sc - sa * cc; s[1] = sa * sb * sc + ca * cc; s[2] = cb * sc;
    r[0] = ca * sb * cc + sa * sc; r[1] = sa * sb * cc - ca * sc; r[2] = cb * cc;
    put_vec(9, n[0], n[1], n[2]); put_vec(15, r[0], r[1], r[2]); put_vec(21, s[0], s[1], s[2]);
}

int main(int argc, char **argv)
{
    int f, t, base, n;
    plat_init();
    memset(screen, 0, 32768);
    state_clear();
    ships_hooks();
    view_set_native(1);
    sun_forget();
    srand(11);
    for (t = 128; t <= 130; t += 2) {
        /* quitar tras dibujar: nada de restos */
        for (f = 0; f < 200; f++) {
            put_planet(t, rand() % 0x50, (rand() & 1) << 7, rand() % 0x30, (rand() & 1) << 7, 1 + rand() % 5,
                       rand() * 6.28 / RAND_MAX, rand() * 6.28 / RAND_MAX, rand() * 6.28 / RAND_MAX);
            planet();
        }
        put_planet(t, 0x41, 0x80, 0x20, 0, 60, 0, 0, 0);               /* z fuera de rango: se quita */
        planet();
        check(blank(), t == 128 ? "meridianos: al quitar el planeta no queda nada" : "crater: al quitar el planeta no queda nada");
        /* orientaciones: la figura aparece (mas puntos que el circulo solo) */
        put_planet(t, 0, 0, 0, 0, 3, 0.3, 0.4, 0.5);
        planet(); n = count_on();
        put_planet(t, 0, 0, 0, 0, 3, 0.3, 0.4, 0.5);
        planet();
        check(count_on() == n, "figura estable: el mismo fotograma no cambia nada");
        if (t == 128 && argc > 1) { FILE *fp = fopen(argv[1], "wb"); fwrite(screen, 1, 32768, fp); fclose(fp); }
        put_planet(t, 0x41, 0x80, 0x20, 0, 60, 0, 0, 0); planet();
        check(blank(), "vacia otra vez");
        /* crater: cara oculta (roofv_z < 0) no dibuja la figura */
        if (t == 130) {
            put_planet(t, 0, 0, 0, 0, 3, 0, 0, 0);
            planet(); base = count_on();
            put_planet(t, 0, 0, 0, 0, 3, 0, 0, 0); inwk[20] |= 0x80; inwk[19] = 5;
            planet();
            check(count_on() < base, "crater: roofv_z < 0 no se ve");
            put_planet(t, 0x41, 0x80, 0x20, 0, 60, 0, 0, 0); planet();
            check(blank(), "crater oculto: vacia");
        }
    }
    printf("test_arcs: %d comprobaciones, %d fallos\n", total, fails);
    return fails != 0;
}
