/* Own laser and damage to other ships (step 3, first part). Unverified port except where stated (see docs/PENDIENTES.md):
   main-loop firing (part 3, MA68), LASLI/LASLI2 (laser lines), HITCH (the ship is in the sights),
   laser damage (part 11, MA47..MA14: EXNO, BURN, SPIN, EXNO2, ANGRY), the reward when an explosion ends
   (part 12, MA15..KS1S), the end of the pulse (part 16, MA23), cooling and the pulse counter (MLOOP),
   DENGY, energy and shield recharge (part 16, MA77..paen1, SHD) and the message delay (DLY, MESS). */
#ifndef COMBAT_H
#define COMBAT_H
#include "platform.h"

#define LASER_COL C_WHITE                /* the original sets no colour (the "LDA #RED" in LASLI2 is commented out) */
#define MESS_CASH 0                      /* MESS #0: current money and " CR" */
#define MESS_RIGHT_ON 101                /* MESS #101: "RIGHT ON COMMANDER!" */
#define MESS_DLY 20                      /* frames an in-flight message lasts */

extern u8 las;                           /* LAS: laser power this frame (0 = does not fire) */
extern u8 las2;                          /* LAS2: power of the last shot; != 0 = there are laser lines on screen */
extern u8 lasct;                         /* LASCT: pulse count (0 = can fire; always 0 with beam and military) */
extern u8 lasx, lasy;                    /* LASX, LASY: vertex of the laser lines */
extern u8 dly;                           /* DLY: frames left for the in-flight message (0 = none) */
extern u8 combat_view_on;                /* QQ11 = 0: we are in a space view (LASLI2 draws) */

extern void (*combat_mess_hook)(u8 token);   /* paints the message (flight.c); NULL = not painted */
extern void (*combat_mess_off_hook)(void);   /* erases the message when DLY reaches 0 */

void combat_reset(void);                 /* on launch after docking (DOENTRY/RES2): energy, shields and laser ready */
void laser_forget(void);                 /* TTX66: the view has been erased, the laser lines are no longer there */
void laser_fire(u8 view, u8 fire);       /* part 3 (MA68): LAS = 0 and, if possible, fires (LASNOISE + LASLI) */
void lasli(void);                        /* LASLI: new vertex, heats the laser, uses energy, draws */
void lasli2(void);                       /* LASLI2: draws the same lines again (erases them, XOR) */
void laser_frame_end(void);              /* part 16 (MA23): end of a pulse */
void laser_cool(void);                   /* MLOOP: GNTMP - 1 and LASCT - 2 (or - 1) */
u8   hitch(void);                        /* HITCH: 1 (carry) if the ship in INWK is in the sights */
void combat_hit(void);                   /* part 11 after PLUT: HITCH and laser damage to the ship in INWK */
void angry(u8 type);                     /* ANGRY: the ship at INF (zinf) turns hostile; the station, the one in slot 1 */
void exno2(u8 type);                     /* EXNO2: adds the ship of that type to TALLY, message if the high byte rises, sound */
u8   dengy(void);                        /* DENGY: energy - 1 (never 0); returns 1 if it reached 0 (Z flag of the DEC) */
u8   shd(u8 x);                          /* SHD: x + 1 without passing 255; if it rises, uses 1 energy (DENGY) */
void energy_tick(u8 mcnt);               /* part 16 (MA77): every 8 frames recharges shields (energy >= 128) and energy */
int  combat_killed(void);                /* part 12 (MA15): the ship in INWK finished exploding or was scooped: FIST and
                                            reward; returns 1 if it must be removed (KS1) */
void mess(u8 token);                     /* MESS: in-flight message for MESS_DLY frames */
void dly_tick(void);                     /* TT100: DLY - 1 and erases the message on reaching 0 */
#endif
