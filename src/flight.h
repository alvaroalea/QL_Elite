/* Flight (PROVISIONAL): space view with stardust, basic instrument panel, speed controls,
   view change, hyperspace and docking. There is still no MVEIT, TACTICS, STARS or DIALS of the original:
   the dust is a simple substitute and docking does not require reaching the station. */
#ifndef FLIGHT_H
#define FLIGHT_H
#define FL_DOCK 1                    /* the pilot has docked */
#define FL_QUIT 2                    /* emergency exit */
#define FL_DEATH 3                   /* we have died (DEATH): back to the title (DEATH2) */
extern int flight_testkeys;          /* tests: N (ship), K (explode) and C (docking without computer) */
int flight_run(void);
void misjump(void);                  /* MJP: failed jump into the space between systems (for the tests) */
#endif
