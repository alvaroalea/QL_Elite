/* Damage to our ship (step 3, second part): OOPS (shields and energy), OUCH/ou2/ou3 (loss of cargo and equipment),
   collisions, cargo scooping and docking through the slot (flight loop parts 9 and 10: MA58..MA63, ISDK, tnpr1),
   periodic checks of part 15 (ENERGY LOW, altitude, cabin temperature near the sun, fuel
   scooping) and death (DEATH, D1/D2 with Ze and fq1). 6502 transcription; verification in docs/PENDIENTES.md. */
#ifndef DAMAGE_H
#define DAMAGE_H
#include "platform.h"

/* MESS: tokens of the in-flight messages used by the original (flight.c turns them into text) */
#define MESS_ENERGY_LOW 100              /* "ENERGY LOW" + beep */
#define MESS_ECM        108              /* "E.C.M.SYSTEM" */
#define MESS_FUEL_SCOOPS 111             /* "FUEL SCOOPS" */
#define MESS_BOMB       113              /* "ENERGY BOMB" (114 "ENERGY UNIT", 115 "DOCKING COMPUTERS") */
#define MESS_DOCK_ON    123              /* "DOCKING COMPUTERS ON" */
#define MESS_SCOOP_ON   160              /* "FUEL SCOOPS ON" + beep */
#define MESS_GOOD       208              /* 208 + commodity (0..16): commodity name */

#define COL_NONE  0                      /* combat_collide: nothing (or scooping, or a collision without death) */
#define COL_DOCK  1                      /* correct entry through the slot: DOENTRY */
#define COL_DEATH 2                      /* DEATH */

extern u8 mess_de;                       /* "de": bit 1 = append " DESTROYED" to the message (read by the MESS hook) */
extern u8 player_dead;                   /* 1 = the original would jump to DEATH */

u8   oops(u8 a, u8 c);                   /* OOPS: damage a (with the incoming carry c of the SBC); 1 if we die */
void ouch(void);                         /* OUCH: with the shields exhausted, cargo or equipment may be lost */
u8  *ouch_item(u8 x);                    /* QQ20+X (0..21): commodities 0..16, ECM, BST, BOMB, ENGY, DKCMP */
int  combat_collide(void);               /* parts 9 and 10 for the ship in INWK (after MVEIT and MAL3): COL_* */
u8   damage_checks(u8 mc);               /* part 15 (MA22/MA93): messages, altitude, cabin, fuel; 1 = death */
#endif
