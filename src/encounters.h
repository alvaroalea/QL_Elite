/* Parts 1 to 4 of the main loop: ships appearing every 256 iterations (asteroids, cargo, hermits, traders,
   police, Thargoids, pirates and bounty hunters, and the Constrictor of mission 1). */
#ifndef ENCOUNTERS_H
#define ENCOUNTERS_H
#include "platform.h"
extern u8 enc_ev;                  /* EV */
extern u8 enc_c;                   /* output carry */
u8   there(void);                  /* THERE: 1 if we are in the Constrictor's system */
void encounters_tick(u8 c);        /* parts 2-4 (and 1, MTT4) with incoming carry c */
#endif
