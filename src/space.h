/* Local space (unverified, see docs/PENDIENTES.md): ship turning (roll/pitch with damping), planet and station
   relative to the ship for the compass, altitude. Transcription of DOKEY (turning part), BUMP2, REDU2, cntr, the part of the main
   loop that computes ALPHA/BETA, MV40, MVT1 and MV45 (translation by speed), and the ALTIT calculation. */
#ifndef SPACE_H
#define SPACE_H
#include "platform.h"
#include "keys.h"

extern u8 jstx, jsty;                /* X and Y "joystick": 128 = centre; the keyboard moves them in steps of 20 */
extern u8 alpha, beta;               /* roll and pitch rate (sign-magnitude: bit 7 = sign) */
extern u8 alp1, bet1;                /* magnitudes for the indicator */
extern u8 alp2[2], bet2[2];          /* signs (ALP2, ALP2+1, BET2, BET2+1) */
extern u8 mcnt;                      /* MCNT: frame counter of the main loop */
extern u8 flh;                       /* FLH: 0 = fixed bars (default), $FF = flashing bars */
extern u8 damp;                      /* DAMP: 0 = turn damping enabled (default), $FF = disabled */
extern u8 djd;                       /* DJD: 0 = on crossing the centre the turn stops at 128 (default) */
extern u8 space_view;                /* VIEW: current view; PLUT uses it for the ship axes */
extern u8 energy, fsh, ash;          /* energy and front and rear shields (255 = full) */
extern u8 altit, cabtmp, gntmp;      /* altitude above the planet (255 = far), cabin and laser temperature */

void space_reset_all(void);         /* RESET: ZERO + factory flight values + RES2 */
void space_reset(void);              /* RES2: initial values of the turns and speed */
void space_launch(void);             /* planet and station relative to the ship on launch (simplified SOLAR) */
void space_controls(const struct controls *c);   /* keys -> JSTX/JSTY -> ALPHA/BETA and speed */
#define SPACE_EV_STATION 1           /* space_step: the station has gone out of sight */
#define SPACE_EV_DOCK 2              /* space_step: we have entered through the station slot */
#define SPACE_EV_DEATH 4             /* space_step: death (collision, planet, sun or no energy) */
u8   space_bad(void);                /* BAD: illegal cargo level */
u8   space_step(void);               /* one frame: MV40 of each ship, radar (SCAN), altitude; returns the SPACE_EV_* */
extern void (*space_scan_hook)(u8 type); /* tests: replaces SCAN (logs the call) */
void space_scan(u8 type);            /* SCAN: draws (XOR) the ship in INWK on the radar */
void space_scanner_reset(void);      /* after repainting the panel: no ship counts as drawn */
void space_scanner_clear(void);      /* clears from the radar everything drawn (before changing system) */
void space_arrive(void);             /* arrival after hyperspace: planet far away and no station */
int  space_spawn(u8 type);           /* test contact in a free slot; -1 if there is no room */
u8   space_bump2(u8 x, u8 a);        /* BUMP2 */
u8   space_redu2(u8 x, u8 a);        /* REDU2 */
u8   space_cntr(u8 x);               /* cntr: one damping step towards 128 */
int  space_signed(u8 v);             /* sign-magnitude -> integer */
u8 space_scacol(u8 type);                 /* scacol: the original's COL for the ship type (tests) */
#endif
