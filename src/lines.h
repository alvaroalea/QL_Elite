/* Lineas del original (Drawing lines): LOIN transcrito, en coordenadas del juego sobre la pantalla del QL */
#ifndef LINES_H
#define LINES_H
#include "platform.h"

extern void (*loin_hook)(u8 x1, u8 y1, u8 x2, u8 y2);    /* pruebas: cada LOIN con los extremos de la llamada */
void loin_draw(u8 x1, u8 y1, u8 x2, u8 y2, u8 c);       /* LOIN: linea en XOR de (x1, y1) a (x2, y2), sin uno de los extremos */
extern void (*lines_hloin_hook)(u8 x1, u8 x2, u8 y, u8 c);  /* pruebas: cada HLOIN de lines_hloin */
void lines_hloin(u8 x1, u8 x2, u8 y, u8 c);                 /* HLOIN (gfx_hloin) con el gancho de pruebas */
void nlin2(u8 y);                                           /* NLIN2: HLOIN de (0, y) a (255, y) en azul */
void nlin4(void);                                           /* NLIN4: NLIN2 con y = 19 (en el QL no hay modo texto: ver NLI4) */
#endif
