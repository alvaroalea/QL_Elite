/* Vista nativa de vuelo: sol por filas (con su borde aleatorio), planeta (no se redibuja si no cambia), anillos del tunel (rectangulos
   en el despegue, elipses en el hiperespacio) y el borrado exacto de todo. Invariante del sol: la pantalla tiene encendido en cada fila
   exactamente el tramo que el sol dice (sun_rows_get). */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "platform.h"
#include "state.h"
#include "ships.h"
#include "ship3d.h"
#include "sun.h"
#include "emath.h"

static u8 snap[32768];
static int fails, total;
static void check(int ok, const char *m) { total++; if (!ok) { fails++; printf("FALLO: %s\n", m); } }
static int blank(void) { int i; for (i = 0; i < 32768; i++) if (screen[i]) return 0; return 1; }
static int rows_ok(void)
{
    int y, x, lo, hi, has;
    for (y = 0; y < 256; y++) {
        has = (y < 160) && sun_rows_get(y, &lo, &hi);
        for (x = 0; x < 256; x++) {
            int on = gfx_get(x, y) != 0;
            int want = has && x >= lo && x <= hi;
            if (on != want) return 0;
        }
    }
    return 1;
}
static void put_body(int type, int xhi, int xs, int yhi, int ys, int zhi)
{
    int i;
    ztype = (u8)type;
    for (i = 0; i < 37; i++) inwk[i] = 0;
    inwk[1] = (u8)xhi; inwk[2] = (u8)xs; inwk[4] = (u8)yhi; inwk[5] = (u8)ys; inwk[8] = (u8)zhi;
}

int main(void)
{
    int i, f;
    plat_init();
    memset(screen, 0, 32768);
    state_clear();
    ships_hooks();
    view_set_native(1);
    sun_forget();
    /* sol: varios fotogramas moviendose y creciendo; el invariante vale en todos */
    srand(5);
    for (f = 0; f < 60; f++) {
        put_body(129, 20 + rand() % 90, (rand() & 1) << 7, 10 + rand() % 70, (rand() & 1) << 7, 2 + rand() % 9);
        planet();
        check(rows_ok(), "sol: la pantalla coincide con los tramos de cada fila");
    }
    put_body(129, 0x70, 0, 0x30, 0x80, 0xE0);                        /* lejisimos / fuera: se borra */
    planet();
    sun_wpls();
    check(blank(), "sol: borrar deja la pantalla vacia");
    /* sol sin cambios de posicion: el borde aleatorio cambia pero el invariante sigue */
    for (f = 0; f < 30; f++) { put_body(129, 40, 0, 30, 0, 3); planet(); check(rows_ok(), "sol quieto: invariante"); }
    sun_wpls();
    check(blank(), "sol quieto: borrar deja vacio");
    /* planeta: dos fotogramas iguales no cambian nada; moverlo borra el anterior; quitarlo vacia */
    put_body(128, 0x40, 0x80, 0x20, 0, 3);
    planet();
    memcpy(snap, screen, sizeof snap);
    check(!blank(), "planeta: se dibuja");
    planet();
    check(memcmp(snap, screen, sizeof snap) == 0, "planeta: igual -> no cambia (se salta el redibujado)");
    put_body(128, 0x58, 0x80, 0x20, 0, 3);
    planet();
    check(memcmp(snap, screen, sizeof snap) != 0 && !blank(), "planeta: al moverse cambia");
    put_body(128, 0x41, 0x80, 0x20, 0, 60);                          /* z fuera de rango (>= 48): se quita */
    planet();
    check(blank(), "planeta: al quitarse no queda nada");
    /* tunel: dos pasadas iguales borran (despegue = rectangulos, hiperespacio = elipses) */
    zk3[0] = 128; zk3[1] = 0; zk4[0] = 68; zk4[1] = 0;
    memset(screen, 0, 32768);
    tunnel_draw(8);
    check(!blank(), "tunel de despegue: dibuja");
    for (i = 0; i < 256; i++) if (gfx_get(i, 80 - 8 * 5 / 8)) break;
    tunnel_draw(8);
    check(blank(), "tunel de despegue: la segunda pasada borra");
    tunnel_draw(4);
    check(!blank(), "tunel de hiperespacio: dibuja");
    tunnel_draw(4);
    check(blank(), "tunel de hiperespacio: la segunda pasada borra");
    printf("%d comprobaciones, %d fallos\n", total, fails);
    return fails != 0;
}
