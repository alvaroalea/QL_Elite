/* Ship AI (TACTICS, 7 parts; the tail TA151..TA10 is in dockit.c), missiles (FRMIS, FR1, SFRMIS, KS2, ABORT,
   ABORT2 and the flight loop stretches: keys T, U, M, locking a target in part 11) and ECM (ECBLB2, ECMOF, E key and
   part 16). 6502 transcription (see docs/PENDIENTES.md). */
#ifndef TACTICS_H
#define TACTICS_H
#include "platform.h"

#define MS_BLACK 0                       /* colours of the missile indicator (the original's: BLACK, GREEN, WHITE, RED) */
#define MS_GREEN 8
#define MS_WHITE 12
#define MS_RED   20
#define MESS_INCOMING 120                /* "INCOMING MISSILE" */
#define MESS_TARGET_LOST 200             /* "TARGET LOST" */
#define MESS_JAMMED 201                  /* "MISSILE JAMMED" */

extern u8 ecma;                          /* ECMA: count of the ECM that is sounding */
extern void (*tac_msbar_hook)(u8 x, u8 col);   /* tests: each MSBAR of ABORT/ABORT2 */
extern u8 msar;                          /* MSAR: the armed missile looks for a target */

void tactics(void);                      /* TACTICS on INWK (X = TYPE) */
void ecm_on(void);                       /* ECBLB2 */
void ecm_off(void);                      /* ECMOF */
void ecm_key(void);                      /* E key */
void ecm_frame(void);                    /* part 16: ECM */
void abort2(u8 slot, u8 col);            /* ABORT2 */
void abort_missile(u8 col);              /* ABORT */
void missile_bar(u8 col);                /* MSBAR of the NOMSL missile */
void missile_unarm(void);                /* U key */
void missile_target(void);               /* T key */
int  missile_fire(void);                 /* M key: 1 if there was no target (the original jumps to the end of part 3) */
void missile_lock(u8 slot);              /* part 11: lock a target if the ship is in the sights */
void frmis(void);                        /* FRMIS */
void fr1(void);                          /* FR1 */
void sfrmis(void);                       /* SFRMIS */
#endif
