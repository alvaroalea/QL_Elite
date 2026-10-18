/* IA de las naves (TACTICS, 7 partes; la cola TA151..TA10 esta en dockit.c), misiles (FRMIS, FR1, SFRMIS, KS2, ABORT,
   ABORT2 y los tramos del bucle de vuelo: teclas T, U, M, fijar blanco en la parte 11) y ECM (ECBLB2, ECMOF, tecla E y
   parte 16). Transcripcion del 6502 (ver PENDIENTES.md). */
#ifndef TACTICS_H
#define TACTICS_H
#include "platform.h"

#define MS_BLACK 0                       /* colores del indicador de misil (los del original: BLACK, GREEN, WHITE, RED) */
#define MS_GREEN 8
#define MS_WHITE 12
#define MS_RED   20
#define MESS_INCOMING 120                /* "INCOMING MISSILE" */
#define MESS_TARGET_LOST 200             /* "TARGET LOST" */
#define MESS_JAMMED 201                  /* "MISSILE JAMMED" */

extern u8 ecma;                          /* ECMA: cuenta del ECM que esta sonando */
extern void (*tac_msbar_hook)(u8 x, u8 col);   /* pruebas: cada MSBAR de ABORT/ABORT2 */
extern u8 msar;                          /* MSAR: el misil armado busca blanco */

void tactics(void);                      /* TACTICS sobre INWK (X = TYPE) */
void ecm_on(void);                       /* ECBLB2 */
void ecm_off(void);                      /* ECMOF */
void ecm_key(void);                      /* tecla E */
void ecm_frame(void);                    /* parte 16: ECM */
void abort2(u8 slot, u8 col);            /* ABORT2 */
void abort_missile(u8 col);              /* ABORT */
void missile_bar(u8 col);                /* MSBAR del misil NOMSL */
void missile_unarm(void);                /* tecla U */
void missile_target(void);               /* tecla T */
int  missile_fire(void);                 /* tecla M: 1 si no habia blanco (el original salta al final de la parte 3) */
void missile_lock(u8 slot);              /* parte 11: fijar blanco si la nave esta en la mira */
void frmis(void);                        /* FRMIS */
void fr1(void);                          /* FR1 */
void sfrmis(void);                       /* SFRMIS */
#endif
