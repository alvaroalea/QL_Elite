/* Espacio local (sin verificar, ver PENDIENTES.md): giro de la nave (roll/pitch con amortiguacion), planeta y estacion
   respecto a la nave para la brujula, altitud. Transcripcion de DOKEY (parte de giro), BUMP2, REDU2, cntr, la parte del bucle
   principal que calcula ALPHA/BETA, MV40, MVT1 y MV45 (traslacion por la velocidad), y el calculo de ALTIT. */
#ifndef SPACE_H
#define SPACE_H
#include "platform.h"
#include "keys.h"

extern u8 jstx, jsty;                /* "joystick" X e Y: 128 = centro; el teclado los mueve de 20 en 20 */
extern u8 alpha, beta;               /* velocidad de alabeo y de cabeceo (signo-magnitud: bit 7 = signo) */
extern u8 alp1, bet1;                /* magnitudes para el indicador */
extern u8 alp2[2], bet2[2];          /* signos (ALP2, ALP2+1, BET2, BET2+1) */
extern u8 mcnt;                      /* MCNT: contador de fotogramas del bucle principal */
extern u8 flh;                       /* FLH: 0 = barras fijas (por defecto), $FF = barras parpadeantes */
extern u8 damp;                      /* DAMP: 0 = amortiguacion de los giros activada (por defecto), $FF = desactivada */
extern u8 djd;                       /* DJD: 0 = al cruzar el centro el giro se detiene en 128 (por defecto) */
extern u8 energy, fsh, ash;          /* energia y escudos delantero y trasero (255 = lleno) */
extern u8 altit, cabtmp, gntmp;      /* altitud sobre el planeta (255 = lejos), temperatura de cabina y de los lasers */

void space_reset(void);              /* RES2: valores iniciales de los giros y la velocidad */
void space_launch(void);             /* planeta y estacion respecto a la nave al despegar (SOLAR simplificado) */
void space_controls(const struct controls *c);   /* teclas -> JSTX/JSTY -> ALPHA/BETA y velocidad */
void space_step(void);               /* un fotograma: MV40 del planeta y de la estacion, altitud */
u8   space_bump2(u8 x, u8 a);        /* BUMP2 */
u8   space_redu2(u8 x, u8 a);        /* REDU2 */
u8   space_cntr(u8 x);               /* cntr: un paso de amortiguacion hacia 128 */
int  space_signed(u8 v);             /* signo-magnitud -> entero */
#endif
