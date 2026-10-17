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
void plat_present(void);          /* una vez por fotograma; en el QL tambien vacia la cola de teclado de la consola */
void plat_shutdown(void);         /* idempotente; en el QL se registra con atexit */
void plat_flush_input(void);
void plat_exit(void);             /* QL: termina el programa sin el mensaje "Press a key to exit" del runtime de c68 (MT.FRJOB); PC: no hace nada, main termina con return */
void plat_wait_key(void);         /* QL: espera una pulsacion (suelta, pulsa, suelta) para poder ver la pantalla antes de volver a QDOS; PC: no hace nada */      /* QL: descarta las teclas acumuladas en la consola (KEYROW no las consume) */

/* gfx.c */
void gfx_clear(u8 c);
void gfx_pixel(int x, int y, u8 c);
void gfx_xor_pixel(int x, int y, u8 c);
u8   gfx_get(int x, int y);
void gfx_line(int x0, int y0, int x1, int y1, u8 c, int xor_mode);
void gfx_hline(int x0, int x1, int y, u8 c);
void gfx_hline_xor(int x0, int x1, int y, u8 c);
void gfx_hloin(int x1, int x2, int y, u8 c);        /* HLOIN del original: XOR, extremos a pares, cubre X1..X2-1 */
void gfx_dust(int x, int y, u8 zz);                  /* PIXEL del original (XOR, blanco): zz >= 128 = 2 puntos, 80..127 = 3, < 80 = 3 puntos en dos lineas */   /* como gfx_hline pero en XOR (sol: borrar = redibujar) */
void gfx_rect(int x, int y, int w, int h, u8 c);
#endif
