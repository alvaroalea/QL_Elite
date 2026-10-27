/* Docking computer: DOCKIT, DCS1/TAS7, TA2, GOPL, TA151 and the tail of TACTICS they use (TA152, TA15, TA11, TA6, PH10E,
   TA9), and the stretch of DOKEY that turns its result into virtual keys. TACTICS will reuse it. */
#ifndef DOCKIT_H
#define DOCKIT_H
#include "platform.h"
#include "keys.h"

extern u8 t_rat, t_rat2, t_cnt, t_cnt2;  /* RAT, RAT2, CNT, CNT2 of TACTICS */

void dockit(void);                       /* DOCKIT on INWK (negative ztype = our ship) */
void dcs1(void);                         /* DCS1 */
void ta151(void);                        /* TA151 (y TA152..TA10) */
void ta152(u8 a);                        /* TA152: CNT = a and TA15 */
void ta15(void);                         /* TA15..TA10: pitch, roll and acceleration towards XX15 */
void gopl(void);                         /* GOPL: turn towards the planet */
void dokey_auto(struct controls *c);     /* DOKEY with auto != 0: virtual keys of the docking computer */
#endif
