/* Proyeccion y dibujo de naves (LL9) con recorte de lineas y monton de lineas por nave (XOR).
   Port de la version Apple II; sigue el flujo del 6502. Las lineas se dibujan con ship_loin_hook
   (por defecto gfx_line en XOR sobre el area de 256x136). */
#ifndef SHIP3D_H
#define SHIP3D_H
#include "state.h"
#include "emath.h"
#include "ship_math.h"
#include "shipdata.h"

extern u8 ztype;                    /* TYPE: tipo de la nave actual (bit 7 = planeta/sol) */
extern const u8 *zxx0;              /* XX0: plano de la nave (ver SHIP_BP) */
extern u8 *zxx19;                   /* XX19: monton de lineas de la nave (byte 0 = bytes usados; luego 4 por linea) */
extern u8 *zinf;                    /* INF: bloque de la nave en K% (se ponen a 0 sus bytes 28 y 30 al explotar) */
extern u8 zxx4, zxx13, zxx17, zxx20, zcnt, zswap, zlsnum, zlsnum2;
extern u8 zxx18[9];
extern u8 xx3[258];                 /* XX3: coordenadas de pantalla de los vertices (4 bytes cada uno) */
#define xx2 zk3                     /* XX2: caras visibles (comparte memoria con K3, como el original) */
extern u8 ship_c_in;                /* acarreo con que se entra a LL9 (lo usa el primer DORND al explotar) */

/* Ganchos: LOIN (dibuja XX15 = X1,Y1,X2,Y2), DOEXP (explosion) y PLANET (ranura de planeta/sol) */
extern void (*ship_loin_hook)(void);
extern void (*ship_doexp_hook)(void);
extern void (*ship_planet_hook)(void);

void ship_ll9(void);                /* dibuja (o borra) la nave de INWK */
void ship_ll51(void);               /* XX12 = XX15 (vector) por la matriz XX16 */
void ship_shppt(void);              /* nave lejana: un punto */
u8   ship_clip(void);               /* recorta XX15/XX12; devuelve 1 si la linea queda fuera */
u8   ship_clip2(u8 a);              /* entrada CLIP2 (a = XX15+5) */
void ship_lsput(void);              /* guarda la linea en el monton y dibuja la nueva y borra la vieja */
void ship_lsclr(void);              /* borra las lineas del monton que quedan sin sustituir */
#endif
