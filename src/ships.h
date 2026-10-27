/* Ships in local space: slot list (K%, FRIN, MANY), creation (ZINF, NWSHP), per-ship loop of the reduced main loop
   (MVEIT, PLUT, LL9), explosions (DOEXP), planet and sun (PLANET), SOLAR and NWSPS, and KILLSHP. Unverified port, see docs/PENDIENTES.md.
   Each slot has its line heap (heaps) to draw and erase its lines in XOR, and for the explosion cloud. */
#ifndef SHIPS_H
#define SHIPS_H
#include "platform.h"

#define HEAP_BYTES 256                           /* line heap per ship */
#define SHIPS_EV_STATION 1                       /* ships_step: the station has gone out of sight (the sun remains) */
#define SHIPS_EV_DOCK 2                          /* ships_step: we have entered through the slot (DOENTRY) */
#define SHIPS_EV_DEATH 4                         /* ships_step: fatal collision (DEATH) */

const u8 *ship_bp_for(u8 type);                  /* ship blueprint (the station is a Dodo with tech level >= 10) */
u8   ship_e_newb(u8 type);                       /* E%: initial NEWB of the type (bit 7: carries an escape pod) */
void ship_zinf(void);                            /* ZINF: INWK to zero with the default orientation */
int  ship_new(u8 type);                          /* NWSHP: adds the ship in INWK to the first free slot; -1 if it does not fit */
void ship_kill(int slot);                        /* KILLSHP: removes the ship and shifts the others */
void ks2(int slot);                              /* KS2: readjusts the missile targets after removing the slot */
void ships_zero(void);                           /* ZERO: FRIN..de to 0 (see ships.c) */
void ships_slsp_reset(void);                      /* SLSP = LS% */
void ships_clear(void);                          /* no ships */
void ships_launch(void);                         /* planet (slot 0) and station (slot 1) on launch (SOLAR and NWSPS) */
void ships_arrive(void);
int  ships_part14(u8 mc);                       /* part 14: the station appears near the planet (1 if it appears) */                         /* planet and sun (SOLAR) after hyperspace */
int  ships_spawn(u8 type);                       /* test contact ahead of the ship; returns the slot or -1 */
#ifdef DEBUG
int  ships_dbg_near_station(void);              /* DEBUG (SHIFT-J): station ahead of us; 0 if there is none */
#endif
int  ships_explode_nearest(void);                /* test: makes the nearest ship explode (not planet or station); returns the slot or -1 */
u8   ships_step(u8 view);                        /* one frame: moves, draws, erases and removes ships; returns SHIPS_EV_* */
void ships_hooks(void);                          /* sets DOEXP and PLANET as LL9 hooks */
void ships_forget_lines(void);                   /* after erasing the whole screen: no ship counts as drawn */
void ships_erase_all(void);                      /* erases from the screen the lines of all the ships (before repainting or changing system) */
int  sfs1(u8 ai, u8 type);                      /* SFS1: child ship of the one at INF (zinf) with AI = ai (NWSHP) */
void spin(u8 type);                              /* SPIN: at 50 %, debris of that type from the destroyed ship */
void spin2(u8 cnt, u8 type);                     /* SPIN2: cnt debris of that type */
int  ships_death_debris_one(void);              /* one iteration of D1 (returns the slot or -1) */
void ships_death_debris(void);                   /* DEATH (D1): debris of our ship until slot 4 is filled */
int  frs1(u8 type);                              /* FRS1: new ship ahead of us (returns the slot or -1) */
u8   gthg(void);                                 /* GTHG: Thargoid and Thargon (returns the carry of the last NWSHP) */
u8  *ships_heap(int n);                         /* line heap of slot n */
u8   ships_ze(void);                             /* Ze: INWK of a random aggressive ship; returns the A of DORND2 */
void ships_escape(void);                         /* ESCAPE: our Cobra moves away (97 frames) */
void sescp(void);                                /* SESCP: the current ship launches a pod */
void doexp(void);                                /* DOEXP: cloud of the exploding ship */
void planet(void);                               /* PLANET: draws or erases the planet or sun of INWK */
extern void (*ships_cpix_hook)(u8 x, u8 y, u8 col);
extern void (*ships_spblb_hook)(void);   /* SPBLB from NWSPS (S on) and KS4 (S off), like the original */
#endif
