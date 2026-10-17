/* Fichas de texto del original: normales (TT27, tabla QQ18) y extendidas (DETOK, tablas TKN1 y RUTOK), con la impresion
   justificada de DASC/TT26 y la salida por CHPR (cursor XC, YC). Transcripcion del 6502; lo que depende de la maquina
   (dibujar una letra, borrar la pantalla, esperar una tecla...) va por ganchos. */
#ifndef DETOK_H
#define DETOK_H
#include "platform.h"

#define DT_LL 30                             /* LL: largo de las lineas justificadas */

extern u8 dt_xc, dt_yc;                      /* XC, YC: cursor de texto (columna 0..31, fila 0..23) */
extern u8 dtw1, dtw2, dtw3, dtw4, dtw5, dtw6, dtw8;
extern u8 dt_buf[256];                       /* BUF: linea en curso del texto justificado */

/* ganchos (0 = no hace nada) */
extern void (*dt_letter_hook)(u8 ch, u8 xc, u8 yc);  /* dibujar una letra en (XC, YC) (letter / RR5) */
extern void (*dt_clss_hook)(void);           /* clss: el texto paso de la fila 23; borra la pantalla (XC = YC = 0) */
extern void (*dt_beep_hook)(void);           /* BEEP (caracter 7) */
extern void (*dt_tt66_hook)(u8 view);        /* TT66 con la vista A (MT9 y PAUSE -> PAS1) */
extern void (*dt_pause2_hook)(void);         /* PAUSE2: esperar una tecla */
extern void (*dt_delay_hook)(u8 n);          /* DELAY: n esperas (unos 20 ms cada una) */
extern void (*dt_nlin4_hook)(void);          /* NLIN4: linea bajo el titulo */
extern void (*dt_clyns_hook)(void);          /* CLYNS: borra la parte de abajo de la pantalla */
extern void (*dt_name_hook)(u8 which);       /* nombres de sistema por TT27: 2 = actual (ypl), 3 = seleccionado (cpl) */

void dt_reset(void);                         /* valores iniciales de DTW1..DTW8 (los del original al cargar) */
void dt_chpr(u8 a);                          /* CHPR */
void dt_dasc(u8 a);                          /* DASC / TT26 (con justificacion si DTW4 bit 7) */
void dt_ex(u8 a);                            /* ex: ficha recursiva A de QQ18 */
void dt_tt27(u8 a);                          /* TT27: ficha normal (o letra, o codigo de control) */
void detok(u8 a);                            /* DETOK: ficha extendida A de TKN1 */
void detok2(u8 a);                           /* DETOK2: un byte de una ficha extendida */
void detok3(u8 a);                           /* DETOK3: ficha A de RUTOK */
void dt_dts(u8 a);                           /* DTS: letra con las mayusculas/minusculas de DTW1/DTW6/DTW8 */
void dt_mt2(void);                           /* MT2: Sentence Case en las fichas extendidas */
void dt_mt14(void);                          /* MT14: texto justificado */
void dt_mt15(void);                          /* MT15: texto alineado a la izquierda */
u8   dt_rupla(u8 i);                         /* RUPLA (i = 1..TOK_NRU) */
u8   dt_rugal(u8 i);                         /* RUGAL */
u8   dt_nru(void);                           /* numero de entradas de RUPLA */
#endif
