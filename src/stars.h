/* Stardust (STARS, STARS1, STARS6, STARS2, FLIP, NWSTARS, nWq, PIXEL2 of the reference source): unverified port,
   see docs/PENDIENTES.md. Each particle stores x, y, z in 8 bits plus an 8-bit low part (sxl, syl, szl); the x and y coordinates
   are sign-magnitude centred on the screen and z is the distance (255 = far). They are drawn and erased with XOR (gfx_dust).
   Uses ALPHA, BETA, ALP1, ALP2, BET1 and BET2 of space.c and DELTA to move the dust with the ship's turns and speed. */
#ifndef STARS_H
#define STARS_H
#include "platform.h"

#define NOST 12                          /* dust particles in normal space */

extern u8 stars_nostm;                   /* NOSTM: particles in use (NOST normally; fewer in witchspace) */
extern u8 sx[NOST + 1], sy[NOST + 1], sz[NOST + 1];       /* indices 1..NOSTM */
extern u8 sxl[NOST + 1], syl[NOST + 1], szl[NOST + 1];
extern u8 delt4[2];                      /* DELT4 (16 bits, low byte first) = speed * 64 */

#ifdef DEBUG
extern u8 stars_dbg_nodraw;
#endif
extern void (*stars_pixel_hook)(u8 x, u8 y, u8 zz);
void stars_set_speed(u8 delta);          /* DELT4 = DELTA * 64 (main loop, MA68) */
void stars_update(u8 view);              /* STARS: moves, erases and draws the dust according to the view (0 front, 1 rear, 2 left, 3 right) */
void stars_new(void);                    /* nWq: new random cloud drawn on screen */
void stars_flip(void);                   /* FLIP: mirrors the dust on the diagonal (view change) and redraws it */
void stars_draw_all(void);              /* test aid: redraws all the particles at their current place */
#endif
