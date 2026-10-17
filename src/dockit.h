/* Ordenador de atraque: DOCKIT, DCS1/TAS7, TA2, GOPL, TA151 y la cola de TACTICS que usan (TA152, TA15, TA11, TA6, PH10E,
   TA9), y el tramo de DOKEY que convierte su resultado en teclas virtuales. TACTICS lo reutilizara. */
#ifndef DOCKIT_H
#define DOCKIT_H
#include "platform.h"
#include "keys.h"

extern u8 t_rat, t_rat2, t_cnt, t_cnt2;  /* RAT, RAT2, CNT, CNT2 de TACTICS */

void dockit(void);                       /* DOCKIT sobre INWK (ztype negativo = nuestra nave) */
void dcs1(void);                         /* DCS1 */
void ta151(void);                        /* TA151 (y TA152..TA10) */
void gopl(void);                         /* GOPL: girar hacia el planeta */
void dokey_auto(struct controls *c);     /* DOKEY con auto != 0: teclas virtuales del ordenador de atraque */
#endif
