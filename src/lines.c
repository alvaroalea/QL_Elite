/* LOIN del original (Apple II), transcrito: la pendiente sale de las tablas de logaritmos (log, logL, alogh), el error de la
   pendiente empieza en S = 128 y se dibuja un punto por cada x (o y) del juego, en XOR. Las lineas mas horizontales (|dy| < |dx|)
   van de izquierda a derecha y las demas de abajo arriba; segun hacia donde va y si hubo que intercambiar los extremos (SWAP),
   el original no dibuja uno de ellos:
     - mas horizontal y subiendo (Y2 < Y1 tras ordenar): falta (X2, Y2) de la llamada;
     - mas horizontal y bajando o plana: falta (X1, Y1) de la llamada;
     - mas vertical: falta (X1, Y1) de la llamada.
   Los acarreos que entran en S + Q (o S + P) son siempre 0 en el original (se comprueba en tools/gen_lines.py). */
#include "lines.h"
#include "emath.h"

void (*loin_hook)(u8 x1, u8 y1, u8 x2, u8 y2) = 0;

/* La transcripcion del LOIN del original (verificada punto a punto contra el 6502: make test_lines). Ya no la usa el juego. */
void loin_draw_6502(u8 x1, u8 y1, u8 x2, u8 y2, u8 c)
{
    u8 p, q, s = 128, swap = 0, t, n, x, y;
    unsigned w;
    if (loin_hook) loin_hook(x1, y1, x2, y2);
    p = (u8)(x2 >= x1 ? x2 - x1 : x1 - x2);
    q = (u8)(y2 >= y1 ? y2 - y1 : y1 - y2);
    if (q < p) {                                        /* STPX: un punto por cada x */
        if (x1 >= x2) { swap = 0xFF; t = x1; x1 = x2; x2 = t; t = y1; y1 = y2; y2 = t; }
        q = q ? em_logdiv(q, p) : 0;                    /* Q = |dy| / |dx| * 256 */
        n = (u8)(p + 1);
        x = x1; y = y1;
        if (y2 >= y1) {                                 /* DOWN: y crece */
            if (!swap) goto li9;
            n--;
            for (;;) {
                gfx_xor_pixel(x, y, c);
li9:            x++;
                w = (unsigned)s + q; s = (u8)w;
                if (w >> 8) y++;
                if (--n == 0) return;
            }
        } else {                                        /* y decrece */
            if (swap) goto li6;
            n--;
            for (;;) {
                gfx_xor_pixel(x, y, c);
li6:            x++;
                w = (unsigned)s + q; s = (u8)w;
                if (w >> 8) y--;
                if (--n == 0) return;
            }
        }
    } else {                                            /* STPY: un punto por cada y, de abajo arriba */
        if (y1 < y2) { swap = 0xFF; t = x1; x1 = x2; x2 = t; t = y1; y1 = y2; y2 = t; }
        if (p) p = em_logdiv(p, q);                     /* P = |dx| / |dy| * 256 (LIfudge si |dx| = 0) */
        n = (u8)(q + 1);
        x = x1; y = y1;
        if (x2 >= x1) {                                 /* hacia la derecha */
            if (!swap) goto li17;
            n--;
            for (;;) {
                gfx_xor_pixel(x, y, c);
li17:           y--;
                w = (unsigned)s + p; s = (u8)w;
                if (w >> 8) x++;
                if (--n == 0) return;
            }
        } else {                                        /* LFT: hacia la izquierda */
            if (!swap) goto li18;
            n--;
            for (;;) {
                gfx_xor_pixel(x, y, c);
li18:           y--;
                w = (unsigned)s + p; s = (u8)w;
                if (w >> 8) x--;
                if (--n == 0) return;
            }
        }
    }
}

/* Linea del juego: Bresenham puro en XOR, con los dos extremos. Los extremos se ordenan siempre igual (x menor primero; con la misma x,
   y menor primero), asi que dibujar (A, B) o (B, A) enciende los mismos puntos y borrar = volver a dibujar. Un punto por cada x (la
   linea es mas horizontal que vertical, |dy| <= |dx|) o por cada y; el error empieza en la mitad. En el QL (ASM_LINE, ql_line.asm) lo
   hace una rutina de ensamblador que escribe directamente en la pantalla; en el PC (y para comprobarla) esta version en C. */
void loin_bres_c(u8 x1, u8 y1, u8 x2, u8 y2, u8 c)
{
    int x = x1, y = y1, dx, dy, sy, e, n, hr = gfx_m4;
    if (x1 > x2 || (x1 == x2 && y1 > y2)) { x = x2; y = y2; x2 = x1; y2 = y1; }
    dx = (int)x2 - x;
    dy = (int)y2 - y;
    if (hr) { x *= 2; dx *= 2; }                        /* modo 4: los 512 puntos (x fisica = 2 x del juego) */
    sy = 1;
    if (dy < 0) { dy = -dy; sy = -1; }
    if (dx >= dy) {                                     /* un punto por cada x */
        e = dx >> 1;
        for (n = dx; n >= 0; n--) {
            if (hr) gfx_xor_pixel_hr(x, y, c); else gfx_xor_pixel(x, y, c);
            e -= dy;
            if (e < 0) { y += sy; e += dx; }
            x++;
        }
    } else {                                            /* un punto por cada y */
        e = dy >> 1;
        for (n = dy; n >= 0; n--) {
            if (hr) gfx_xor_pixel_hr(x, y, c); else gfx_xor_pixel(x, y, c);
            e -= dx;
            if (e < 0) { x++; e += dy; }
            y += sy;
        }
    }
}

#ifdef ASM_LINE
extern void ql_line(long x1, long y1, long x2, long y2, long c, long scr);   /* ql_line.asm */
#endif

void loin_draw(u8 x1, u8 y1, u8 x2, u8 y2, u8 c)
{
    if (loin_hook) loin_hook(x1, y1, x2, y2);
#ifdef ASM_LINE
    ql_line((long)x1, (long)y1, (long)x2, (long)y2, (long)c, (long)screen);
#else
    loin_bres_c(x1, y1, x2, y2, c);
#endif
}

void (*lines_hloin_hook)(u8 x1, u8 x2, u8 y, u8 c) = 0;

void lines_hloin(u8 x1, u8 x2, u8 y, u8 c)
{
    if (lines_hloin_hook) lines_hloin_hook(x1, x2, y, c);
    gfx_hloin(x1, x2, y, c);
}

void nlin2(u8 y) { lines_hloin(0, 255, y, C_BLUE); }        /* STA Y1; COL = BLUE; X1 = 0; X2 = 255; JMP HLOIN */

/* NLIN4: en el modo texto del Apple II (las vistas de texto) NLI4 pone guiones en la fila 2 bajo los caracteres de la fila 1;
   el QL no tiene modo texto (las vistas de texto se dibujan en la pantalla grafica), asi que siempre va por NLIN2 con y = 19,
   que cae en medio de la fila 2 */
void nlin4(void) { nlin2(19); }
