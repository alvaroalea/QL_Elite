/* Elite QL - capa de plataforma. Pantalla = formato nativo QL modo 8 (32 KB, 256x256, 128 bytes/linea). */
#ifndef PLATFORM_H
#define PLATFORM_H
#include <stdint.h>

#define SCR_W 256
#define SCR_H 256
#define SCR_STRIDE 128
#define SCR_BYTES (SCR_STRIDE * SCR_H)

/* Colores QL: bit2=Verde, bit1=Rojo, bit0=Azul */
enum { C_BLACK, C_BLUE, C_RED, C_MAGENTA, C_GREEN, C_CYAN, C_YELLOW, C_WHITE };

extern uint8_t *screen;           /* buffer de pantalla en formato QL */

void plat_init(void);             /* modo 8 + puntero de pantalla */
void plat_present(void);          /* vuelca/refresca (en QL: nada si se dibuja directo) */
void plat_shutdown(void);

/* gfx.c */
void gfx_clear(uint8_t c);
void gfx_pixel(int x, int y, uint8_t c);
void gfx_xor_pixel(int x, int y, uint8_t c);
uint8_t gfx_get(int x, int y);
void gfx_line(int x0, int y0, int x1, int y1, uint8_t c, int xor_mode);
void gfx_hline(int x0, int x1, int y, uint8_t c);
void gfx_rect(int x, int y, int w, int h, uint8_t c);
#endif
