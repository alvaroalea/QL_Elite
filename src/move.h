/* Ship movement in local space: full MVEIT (parts 1 to 9), MV40, MV45, MVT1, MVT6, MVS4, MVS5 and PLUT of the reference
   source. Unverified port, see docs/PENDIENTES.md. Operates on INWK (the current ship) with TYPE = ztype, and uses ALPHA, BETA, ALP1, ALP2,
   BET1, BET2 and DELTA (space.h) to rotate and translate the position relative to our ship. */
#ifndef MOVE_H
#define MOVE_H
#include "platform.h"

extern u8 move_xsav;                     /* XSAV: index of the current ship (MVEIT mixes it with MCNT to space out TIDY and TACTICS) */
extern void (*move_tactics_hook)(void);  /* TACTICS (AI): does not exist yet; ships with INWK+32 bit 7 would call it */

void mveit(void);                        /* MVEIT: moves the ship in INWK (planets and suns via MV40) and draws it on the radar (SCAN) */
void mv40(void);                         /* MV40: rotates the position by our roll and pitch and continues through MV45 */
void mv40_pre(void);                     /* MV40 without the final call to MV45 */
void mv45(void);                         /* MV45: z -= DELTA, rotation of the orientation vectors, roll and pitch counters and radar */
void mvt1(u8 a, u8 x);                   /* MVT1: (x_sg x_hi x_lo) +/- (A R): A = sign of the amount (bit 7) and high part (bits 0-6) */
u8 mvt6(u8 a, u8 x);                     /* MVT6: (A P+2 P+1) = (x_sg x_hi x_lo of INWK+X) + (A P+2 P+1); returns the sign in A */
void mvs4(u8 y);                         /* MVS4: rotates the vector at INWK+Y by ALPHA and BETA */
void mvs5(u8 x, u8 y);                   /* MVS5: rotates the vector pair (INWK+X, INWK+Y) by 3.6 degrees; uses RAT2 */
extern u8 move_p2, move_rat2;            /* P+2 y RAT2 */
void plut(u8 view);                      /* PLUT: INWK axes for the view (0 front, 1 rear, 2 left, 3 right) */
#endif
