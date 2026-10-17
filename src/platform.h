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
void plat_flush_input(void);      /* QL: descarta las teclas acumuladas en la consola (KEYROW no las consume) */

/* gfx.c */
void gfx_clear(u8 c);
void gfx_pixel(int x, int y, u8 c);
void gfx_xor_pixel(int x, int y, u8 c);
u8   gfx_get(int x, int y);
void gfx_line(int x0, int y0, int x1, int y1, u8 c, int xor_mode);
void gfx_hline(int x0, int x1, int y, u8 c);
void gfx_rect(int x, int y, int w, int h, u8 c);
#endif
