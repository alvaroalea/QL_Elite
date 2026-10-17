/* Rutinas aritmeticas de Elite que dependen del estado del juego (INWK, K%, K, K3, XX15, CASH).
   Cada funcion sigue el flujo del 6502, incluidas las "caidas" a la rutina siguiente:
     TAS2 termina en NORM; SPS1 y SPS4 terminan en TAS2 (y por tanto en NORM); TIS3 termina en DVIDT;
     TAS3 y TAS4 terminan en MAD; SCALEY cae en SCALEY2; LCASH cae en MCASH si no hay saldo.
   Los registros de salida del 6502 se devuelven como valor (A) o en sm_x, sm_y, sm_c (X, Y y C).
   Las posiciones/vectores van en signo-magnitud (bit 7 = signo), como en el original.
   No existe TAS5 en esta version (Apple II). */
#ifndef SHIP_MATH_H
#define SHIP_MATH_H
#include "state.h"
#include "emath.h"

extern u8 sm_x, sm_y, sm_c;                /* X, Y y acarreo de la ultima rutina */
extern u8 sm_a;                            /* A a la salida de PROJ (SHPPT lo usa aunque haya desbordado) */

/* K(3 2 1) += vector (x_sign x_hi x_lo) de INWK + x; K+3 lleva el signo. Devuelve A (= K+3). */
u8   sm_mvt3(u8 x);
/* INWK(x) = INWK(x) + 2 * (vector de INWK + y); devuelve A = signo resultante sin bit 7 */
u8   sm_mas1(u8 y, u8 x);
u8   sm_mas2(u8 a, u8 y);                  /* A = (a | signos x,y,z de la ranura en K% + y) y 0x7F */
u8   sm_mas2m(u8 y);                       /* entrada .m: igual con A = 0 */
u8   sm_mas3(u8 y);                        /* A = x_hi^2 + y_hi^2 + z_hi^2 (ranura K% + y), 255 si desborda */
u8   sm_mas4(u8 a);                        /* A = a | x_hi | y_hi | z_hi de INWK */

/* K3(0..8) = vector INWK(0..8) - vector de v (posicion x,y,z de otra nave, 3 bytes cada eje) */
void sm_vcsub(const u8 *v);
void sm_vcsu1(void);                       /* idem con v = ranura 1 de K% */
void sm_tas1(const u8 *v, u8 y);           /* un eje: K3[y-2 ..] = INWK[y-2 ..] - v[y-2 ..]; y = 2, 5 u 8 */
u8   sm_tas2(void);                        /* normaliza K3 a XX15 (y cae en NORM) */
u8   sm_tas3(u8 y);                        /* (A X) = XX15 . vector de INWK + y (usa 3 componentes, paso 2) */
u8   sm_tas4(u8 y);                        /* idem con el vector de la ranura 1 de K% */
void sm_tas6(void);                        /* XX15 = -XX15 */

void sm_norm(void);                        /* XX15 = XX15 / |XX15| (1.0 = 96) */
u8   sm_tis3(u8 a, u8 x, u8 y);            /* componente ortogonal para TIDY; cae en DVIDT */
void sm_tidy(void);                        /* reortogonaliza y normaliza la matriz de orientacion de INWK */

void sm_sps1(void);                        /* XX15 = vector unitario hacia el planeta (ranura 0 de K%) */
u8   sm_sps2(u8 a);                        /* (Y X) = a * 2 / 20 con signo; X devuelto, Y en sm_y */
void sm_sps3(u8 x);                        /* K3(x..x+2) = coordenada de la ranura 0 de K% desplazada 8 bits */
void sm_sps4(void);                        /* XX15 = vector unitario hacia la ranura 1 de K% */

u8   sm_pls6(u8 a);                        /* (X K) = (a P+1 P) / z de INWK; devuelve el acarreo (1 = desbordado); X en sm_x */
u8   sm_proj(void);                        /* proyecta INWK a pantalla: K3(1 0) = x, K4(1 0) = y; devuelve el acarreo */

u8   sm_lcash(u8 x, u8 y);                 /* CASH -= (y x); acarreo 1 = habia saldo; si no, lo deja como estaba */
u8   sm_mcash(u8 x, u8 y);                 /* CASH += (y x); devuelve 0 */
u8   sm_gcash(void);                       /* (Y X) = P * Q * 4; X devuelto, Y en sm_y */

u8   sm_scaley2(u8 a);                     /* acarreo de salida en sm_c */
u8   sm_scaley(u8 a);
u8   sm_scalex(u8 a);
#endif
