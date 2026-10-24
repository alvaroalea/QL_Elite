/* Elite QL - capa de plataforma (C89, apto para XTC68/qcc, donde int puede ser de 16 bits).
   Pantalla = formato nativo QL modo 8 (32 KB, 256x256, 128 bytes/linea). */
#ifndef PLATFORM_H
#define PLATFORM_H

typedef unsigned char u8;

#define SCR_W 256
#define SCR_H 256
#define SCR_STRIDE 128
#define SCR_BYTES 32768L          /* no cabe en un int de 16 bits: siempre long */

/* Colores QL: bit2=Verde, bit1=Rojo, bit0=Azul */
#define C_BLACK   0
#define C_BLUE    1
#define C_RED     2
#define C_MAGENTA 3
#define C_GREEN   4
#define C_CYAN    5
#define C_YELLOW  6
#define C_WHITE   7

extern u8 *screen;                /* buffer de pantalla en formato QL */

void plat_init(void);
unsigned long plat_ticks(void);    /* cuadros de 50 Hz desde el arranque (QL: tarea de la interrupcion de cuadro; PC: clock) */
void plat_present(void);          /* una vez por fotograma; en el QL tambien vacia la cola de teclado de la consola */
void plat_shutdown(void);         /* idempotente; en el QL se registra con atexit */
void plat_flush_input(void);
void plat_exit(void);             /* QL: termina el programa sin el mensaje "Press a key to exit" del runtime de c68 (MT.FRJOB); PC: no hace nada, main termina con return */
int  plat_default_dir(int prog, char *buf, int max); /* QL: directorio por defecto de Toolkit II (prog = 0: DATA_USE, 1: PROG_USE) en buf; 1 si hay uno, 0 si no. PC: 0 */
void plat_wait_key(void);         /* QL: espera una pulsacion (suelta, pulsa, suelta) para poder ver la pantalla antes de volver a QDOS; PC: no hace nada */      /* QL: descarta las teclas acumuladas en la consola (KEYROW no las consume) */

/* gfx.c */
void gfx_clear(u8 c);
void gfx_fill_lines(int y, int n, u8 c);   /* pone a un color n lineas completas desde la y: rapido (palabras largas en el QL) */
void gfx_copy(u8 *dst, const u8 *src, long n);   /* copia rapida de n bytes (multiplo de 32 para ir por palabras largas) */
void gfx_pixel(int x, int y, u8 c);
void gfx_xor_pixel(int x, int y, u8 c);
void gfx_yoff_set(int off);                         /* desplazamiento vertical de lo dibujado en XOR (panel del vuelo: +24) */
u8   gfx_get(int x, int y);
void gfx_line(int x0, int y0, int x1, int y1, u8 c, int xor_mode);
void gfx_hline(int x0, int x1, int y, u8 c);
void gfx_hline_xor(int x0, int x1, int y, u8 c);
void gfx_vline_xor(int x, int y0, int y1, u8 c);   /* columna de puntos (x, y0..y1) en XOR */
void gfx_hloin(int x1, int x2, int y, u8 c);        /* HLOIN del original: XOR, extremos a pares, cubre X1..X2-1 */
void gfx_vloin(int x, int y1, int y2, u8 c);         /* VLOIN del original: vertical de 2 puntos de ancho (x, x+1) en XOR, de y1 a y2 inclusive */
void gfx_cpix(int x, int y, u8 c);                   /* CPIX del original: un punto de color de 2 de ancho (x, x+1) en XOR */
void gfx_dust_6502(int x, int y, u8 zz);            /* PIXEL del original (la brujula; verificado contra el 6502 en make test_lines) */
void gfx_dust(int x, int y, u8 zz);                  /* polvo estelar del juego: zz >= 128 = 1 punto, 80..127 = 1 x 2, < 80 = 2 x 3 (ancho x alto) */
void gfx_rect(int x, int y, int w, int h, u8 c);
void gfx_clear_cells(int col, int row, int ncols, int nrows);   /* vacia (negro) un rectangulo de celdas de texto 8 x 8 (ensamblador en el QL) */
void gfx_ell_update(int ocx, int ocy, int ory, const u8 *otab, int ncx, int ncy, int nry, const u8 *ntab, u8 c, int cly1);   /* cambia una elipse por otra fila a fila (tablas de gfx_ell_spans; ory/nry = -1: no hay) */
void gfx_ell_spans(int rx, int ry, u8 *tab);                  /* tabla de tramos del contorno por fila (2 bytes por fila: lo, hi); 2 * (ry + 1) bytes */
void gfx_ellipse_rows(int rx, int ry, u8 *half);          /* semiancho a cada distancia vertical dy = 0..ry del centro de la elipse (el punto mas a la derecha de cada fila de su contorno) */
void gfx_frame_xor(int cx, int cy, int hw, int hh, u8 c, int clx0, int cly0, int clx1, int cly1);   /* contorno de rectangulo en XOR (cada punto una vez), recortado */
void gfx_ellipse_xor(int cx, int cy, int rx, int ry, u8 c, int clx0, int cly0, int clx1, int cly1);   /* contorno de elipse en XOR (punto medio, solo enteros), recortado al rectangulo; cada punto una sola vez */
#endif
