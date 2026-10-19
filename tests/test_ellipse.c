/* Elipse de punto medio (gfx_ellipse_xor; conexion comprobada para relaciones de ejes hasta 3:1): sin puntos duplicados (el XOR los borraria), simetria en los cuatro cuadrantes, ocho
   conectividad (curva cerrada), cercania a la elipse ideal, borrado exacto, recorte y las medidas del juego (rx = K, ry = 1,2 K). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "platform.h"

u8 *screen;
static u8 buf[32768];
static int fails, total;
static void check(int ok, const char *m) { total++; if (!ok) { fails++; printf("FALLO: %s\n", m); } }
static int lit(int x, int y) { return x >= 0 && x < 256 && y >= 0 && y < 256 && gfx_get(x, y) != 0; }

int main(void)
{
    int rx, ry, cx = 128, cy = 128, x, y, n, i;
    long cases = 0;
    screen = buf;
    for (rx = 1; rx <= 100; rx += (rx < 20 ? 1 : 7)) {
        for (ry = 1; ry <= 120; ry += (ry < 20 ? 1 : 9)) {
            int sym = 1, near = 1, closed = 1;
            cases++;
            memset(buf, 0, sizeof buf);
            gfx_ellipse_xor(cx, cy, rx, ry, C_WHITE, 0, 0, 255, 255);
            n = 0;
            for (y = 0; y < 256; y++) for (x = 0; x < 256; x++) if (lit(x, y)) {
                double v = ((double)(x - cx) * (x - cx)) / ((double)rx * rx) + ((double)(y - cy) * (y - cy)) / ((double)ry * ry);
                n++;
                if (lit(2 * cx - x, y) != 1 || lit(x, 2 * cy - y) != 1) sym = 0;
                /* a menos de ~1 punto de la elipse ideal (en unidades de longitud) */
                if (fabs(sqrt(v) - 1.0) * (rx < ry ? rx : ry) > 1.2) near = 0;
                /* conectada: algun vecino de 8 encendido (salvo un punto suelto: elipse de 1 punto) */
                { int nb = 0, dxx, dyy; for (dyy = -1; dyy <= 1; dyy++) for (dxx = -1; dxx <= 1; dxx++) if ((dxx || dyy) && lit(x + dxx, y + dyy)) nb++; if (nb == 0 && n > 1) closed = 0; }
            }
            if (!(lit(cx - rx, cy) && lit(cx + rx, cy) && lit(cx, cy - ry) && lit(cx, cy + ry))) printf("rx=%d ry=%d L%d R%d T%d B%d\n", rx, ry, lit(cx-rx,cy), lit(cx+rx,cy), lit(cx,cy-ry), lit(cx,cy+ry));
            check(lit(cx - rx, cy) && lit(cx + rx, cy) && lit(cx, cy - ry) && lit(cx, cy + ry), "los cuatro extremos de los ejes");
            check(sym, "simetria en los cuatro cuadrantes (tambien detecta puntos borrados por XOR doble)");
            check(near, "cerca de la elipse ideal");
            if (ry > 3 * rx || rx > 3 * ry) closed = 1;       /* muy estrechas (el juego usa ry = 1,2 rx): puede haber huecos de un punto */
            check(closed, "curva conectada");
            check(n >= 2 * (rx > ry ? rx : ry), "no faltan puntos");
            gfx_ellipse_xor(cx, cy, rx, ry, C_WHITE, 0, 0, 255, 255);          /* volver a dibujar borra */
            for (i = 0; i < 32768 && buf[i] == 0; i++) ;
            check(i == 32768, "dibujar dos veces borra");
        }
    }
    /* las medidas del juego: rx = K, ry = K + K / 5, centrada en pantalla, K = 1..248 (clip a toda la pantalla) */
    for (rx = 1; rx <= 248; rx++) {
        int sym = 1, k;
        ry = rx + rx / 5;
        cx = 128; cy = 128;
        memset(buf, 0, sizeof buf);
        gfx_ellipse_xor(cx, cy, rx, ry, C_WHITE, -1000, -1000, 1000, 1000);     /* sin recorte efectivo (se dibuja lo que cabe) */
        for (y = 0; y < 256; y++) for (x = 0; x < 256; x++) if (lit(x, y) && cx - rx >= 0 && cx + rx < 256 && cy - ry >= 0 && cy + ry < 256) {
            if (!lit(2 * cx - x, y) || !lit(x, 2 * cy - y)) sym = 0;
        }
        check(sym, "medidas del juego: simetria (sin puntos borrados por XOR)");
        gfx_ellipse_xor(cx, cy, rx, ry, C_WHITE, -1000, -1000, 1000, 1000);
        for (k = 0; k < 32768 && buf[k] == 0; k++) ;
        check(k == 32768, "medidas del juego: dibujar dos veces borra");
    }
    /* semianchos por fila (gfx_ellipse_rows) = punto mas a la derecha del contorno en cada fila */
    for (i = 0; i < 300; i++) {
        static u8 half[512];
        int r1 = 1 + rand() % 120, r2 = 1 + rand() % 140, d, bad = 0;
        if (r2 > 3 * r1 || r1 > 3 * r2) continue;                      /* muy estrechas: ver arriba */
        memset(buf, 0, sizeof buf);
        gfx_ellipse_xor(128, 128, r1, r2, C_WHITE, 0, 0, 255, 255);
        gfx_ellipse_rows(r1, r2, half);
        for (d = 0; d <= r2 && 128 + d < 256; d++) {
            int xr = -1;
            for (x = 255; x >= 128; x--) if (lit(x, 128 + d)) { xr = x - 128; break; }
            if (xr != half[d]) bad = 1;
        }
        if (bad) printf("rows rx=%d ry=%d\n", r1, r2);
        check(!bad, "semianchos por fila = punto mas a la derecha del contorno");
    }
    /* rectangulos (gfx_frame_xor): cada punto una vez, perimetro exacto, borrado y recorte */
    for (i = 0; i < 300; i++) {
        int hw = 1 + rand() % 120, hh = 1 + rand() % 90, n2 = 0, k, ok = 1;
        memset(buf, 0, sizeof buf);
        gfx_frame_xor(128, 128, hw, hh, C_WHITE, 0, 0, 255, 255);
        for (y = 0; y < 256; y++) for (x = 0; x < 256; x++) if (lit(x, y)) {
            n2++;
            if (!(x == 128 - hw || x == 128 + hw || y == 128 - hh || y == 128 + hh)) ok = 0;
            if (x < 128 - hw || x > 128 + hw || y < 128 - hh || y > 128 + hh) ok = 0;
        }
        check(ok && n2 == 4 * (hw + hh), "rectangulo: solo el perimetro, cada punto una vez");
        gfx_frame_xor(128, 128, hw, hh, C_WHITE, 0, 0, 255, 255);
        for (k = 0; k < 32768 && buf[k] == 0; k++) ;
        check(k == 32768, "rectangulo: dibujar dos veces borra");
        memset(buf, 0, sizeof buf);
        gfx_frame_xor(100 + rand() % 100, rand() % 200, hw + 100, hh + 50, C_WHITE, 0, 0, 255, 159);
        for (k = 160 * 128; k < 32768 && buf[k] == 0; k++) ;
        check(k == 32768, "rectangulo: el recorte respeta la linea 159");
    }
    /* recorte: el trozo visible es el mismo que el de la elipse entera */
    for (i = 0; i < 400; i++) {
        int ccx = rand() % 400 - 70, ccy = rand() % 300 - 70, r1 = 1 + rand() % 150, r2 = 1 + rand() % 180, k;
        static u8 full[32768];
        memset(buf, 0, sizeof buf);
        gfx_ellipse_xor(ccx, ccy, r1, r2, C_WHITE, 0, 0, 255, 159);
        memcpy(full, buf, sizeof buf);
        for (k = 160 * 128; k < 32768; k++) if (full[k]) break;
        check(k == 32768, "el recorte no escribe por debajo de la linea 159");
        /* referencia: contorno dibujado sin recorte en una pantalla desplazada no cabe aqui; se compara con otro recorte mas grande */
        memset(buf, 0, sizeof buf);
        gfx_ellipse_xor(ccx, ccy, r1, r2, C_WHITE, 0, 0, 255, 255);
        for (k = 0; k < 160 * 128; k++) if (buf[k] != full[k]) break;
        check(k == 160 * 128, "el recorte conserva exactamente la parte visible");
    }
    printf("%ld elipses, %d comprobaciones, %d fallos\n", cases, total, fails);
    return fails != 0;
}
