/* Movimiento de las naves en el espacio local: MVEIT completo (partes 1 a 9), MV40, MV45, MVT1, MVT6, MVS4, MVS5 y PLUT de la fuente de
   referencia. Port sin verificar, ver PENDIENTES.md. Opera sobre INWK (la nave actual) con TYPE = ztype, y usa ALPHA, BETA, ALP1, ALP2,
   BET1, BET2 y DELTA (space.h) para girar y trasladar la posicion respecto a nuestra nave. */
#ifndef MOVE_H
#define MOVE_H
#include "platform.h"

extern u8 move_xsav;                     /* XSAV: indice de la nave en curso (MVEIT lo mezcla con MCNT para espaciar TIDY y TACTICS) */
extern void (*move_tactics_hook)(void);  /* TACTICS (IA): aun no existe; las naves con INWK+32 bit 7 la llamarian */

void mveit(void);                        /* MVEIT: mueve la nave de INWK (planetas y soles por MV40) y la dibuja en el radar (SCAN) */
void mv40(void);                         /* MV40: gira la posicion por nuestro alabeo y cabeceo y sigue por MV45 */
void mv40_pre(void);                     /* MV40 sin la llamada final a MV45 */
void mv45(void);                         /* MV45: z -= DELTA, giro de los vectores de orientacion, contadores de alabeo y cabeceo y radar */
void mvt1(u8 a, u8 x);                   /* MVT1: (x_sg x_hi x_lo) +/- (A R): A = signo de la cantidad (bit 7) y alto (bits 0-6) */
u8 mvt6(u8 a, u8 x);                     /* MVT6: (A P+2 P+1) = (x_sg x_hi x_lo de INWK+X) + (A P+2 P+1); devuelve el signo en A */
void mvs4(u8 y);                         /* MVS4: gira el vector de INWK+Y por ALPHA y BETA */
void mvs5(u8 x, u8 y);                   /* MVS5: gira 3,6 grados el par de vectores (INWK+X, INWK+Y); usa RAT2 */
extern u8 move_p2, move_rat2;            /* P+2 y RAT2 */
void plut(u8 view);                      /* PLUT: ejes de INWK para la vista (0 delante, 1 atras, 2 izquierda, 3 derecha) */
#endif
