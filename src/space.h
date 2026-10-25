/* Espacio local (sin verificar, ver docs/PENDIENTES.md): giro de la nave (roll/pitch con amortiguacion), planeta y estacion
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
extern u8 space_view;                /* VIEW: vista actual; PLUT la usa para los ejes de las naves */
extern u8 energy, fsh, ash;          /* energia y escudos delantero y trasero (255 = lleno) */
extern u8 altit, cabtmp, gntmp;      /* altitud sobre el planeta (255 = lejos), temperatura de cabina y de los lasers */

void space_reset_all(void);         /* RESET: ZERO + valores de fabrica de vuelo + RES2 */
void space_reset(void);              /* RES2: valores iniciales de los giros y la velocidad */
void space_launch(void);             /* planeta y estacion respecto a la nave al despegar (SOLAR simplificado) */
void space_controls(const struct controls *c);   /* teclas -> JSTX/JSTY -> ALPHA/BETA y velocidad */
#define SPACE_EV_STATION 1           /* space_step: la estacion se ha perdido de vista */
#define SPACE_EV_DOCK 2              /* space_step: hemos entrado por la ranura de la estacion */
#define SPACE_EV_DEATH 4             /* space_step: muerte (choque, planeta, sol o sin energia) */
u8   space_bad(void);                /* BAD: nivel de carga ilegal */
u8   space_step(void);               /* un fotograma: MV40 de cada nave, radar (SCAN), altitud; devuelve los SPACE_EV_* */
extern void (*space_scan_hook)(u8 type); /* pruebas: sustituye a SCAN (registra la llamada) */
void space_scan(u8 type);            /* SCAN: dibuja (XOR) la nave de INWK en el radar */
void space_scanner_reset(void);      /* tras repintar el panel: ninguna nave figura como dibujada */
void space_scanner_clear(void);      /* borra del radar todo lo dibujado (antes de cambiar de sistema) */
void space_arrive(void);             /* llegada tras el hiperespacio: planeta lejos y sin estacion */
int  space_spawn(u8 type);           /* contacto de prueba en una ranura libre; -1 si no hay sitio */
u8   space_bump2(u8 x, u8 a);        /* BUMP2 */
u8   space_redu2(u8 x, u8 a);        /* REDU2 */
u8   space_cntr(u8 x);               /* cntr: un paso de amortiguacion hacia 128 */
int  space_signed(u8 v);             /* signo-magnitud -> entero */
u8 space_scacol(u8 type);                 /* scacol: COL del original para el tipo de nave (pruebas) */
#endif
