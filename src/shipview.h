#ifndef SHIPVIEW_H
#define SHIPVIEW_H
#include "ship3d.h"
#define SHIPVIEW_HEAP 256                    /* tamano del monton de lineas por nave */
#define SHIPVIEW_ANG 64                      /* angulos: 64 pasos por vuelta */
void shipview_orient(int ay, int ax);        /* orientacion de INWK: guinada ay y cabeceo ax (0..63) */
void shipview_pos(int x, int y, int z);      /* posicion en unidades de 256 (hi) con signo; z >= 0 */
/* Dibuja una nave. fresh=1: monton vacio (primera vez). fresh=0: se reutiliza el monton y LL9 borra (XOR) las lineas anteriores
   mientras dibuja las nuevas (animacion). El llamante hizo state_clear() y plat_init(). */
void shipview_draw(u8 type, int x, int y, int z, int ay, int ax, u8 *heap, u8 *slot, int fresh);
#endif
