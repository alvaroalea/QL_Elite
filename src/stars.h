/* Polvo estelar (STARS, STARS1, STARS6, STARS2, FLIP, NWSTARS, nWq, PIXEL2 de la fuente de referencia): port sin verificar,
   ver PENDIENTES.md. Cada particula guarda x, y, z en 8 bits mas una parte baja de 8 bits (sxl, syl, szl); las coordenadas x e y
   son signo-magnitud centradas en la pantalla y z es la distancia (255 = lejos). Se dibujan y se borran con XOR (gfx_dust).
   Usa ALPHA, BETA, ALP1, ALP2, BET1 y BET2 de space.c y DELTA para mover el polvo con los giros y la velocidad de la nave. */
#ifndef STARS_H
#define STARS_H
#include "platform.h"

#define NOST 12                          /* particulas de polvo en el espacio normal */

extern u8 stars_nostm;                   /* NOSTM: particulas en uso (NOST normalmente; menos en el espacio de las brujas) */
extern u8 sx[NOST + 1], sy[NOST + 1], sz[NOST + 1];       /* indices 1..NOSTM */
extern u8 sxl[NOST + 1], syl[NOST + 1], szl[NOST + 1];
extern u8 delt4[2];                      /* DELT4 (16 bits, byte bajo primero) = velocidad * 64 */

extern void (*stars_pixel_hook)(u8 x, u8 y, u8 zz);
void stars_set_speed(u8 delta);          /* DELT4 = DELTA * 64 (bucle principal, MA68) */
void stars_update(u8 view);              /* STARS: mueve, borra y dibuja el polvo segun la vista (0 delante, 1 atras, 2 izq., 3 der.) */
void stars_new(void);                    /* nWq: nube aleatoria nueva dibujada en la pantalla */
void stars_flip(void);                   /* FLIP: refleja el polvo en la diagonal (cambio de vista) y lo redibuja */
void stars_draw_all(void);              /* ayuda de pruebas: redibuja todas las particulas en su sitio actual */
#endif
