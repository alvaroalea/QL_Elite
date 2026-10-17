/* Cartas galactica y local (TT22, TT23, TT14, TT15, TT103, TT105 de la fuente de referencia), con SUN y CIRCLE.
   Port sin verificar (ver PENDIENTES.md). Geometria de la fuente: carta galactica de 192 x 96 pixeles (x 32..223, y 19..115,
   escala 0,75 en x y 0,375 en y); carta local con 3 pixeles por unidad en x y 1,5 en y, centrada en (105, 75). */
#ifndef CHART_H
#define CHART_H
#include "platform.h"

#define CHART_GCYT 19                 /* GCYT: y de la parte superior de la carta galactica */
#define CHART_GCYB 115                /* GCYB = GCYT + 0,75 * 128 */

void chart_draw(int local);           /* dibuja la carta completa (sobre pantalla ya borrada) */
void chart_cursor(int local);         /* TT103: cruz del sistema objetivo (XOR: dos llamadas la borran) */
#endif
