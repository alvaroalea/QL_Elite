/* In-flight equipment: energy bomb (BOMBON, BOMBOFF, BOMBEFF, BOMBEFF2, BOMBPOS, BOMBTBX, BOMBTBY and the stretches of
   parts 3 and 16 of the flight loop) and jump within the system (WARP). 6502 transcription (see docs/PENDIENTES.md). */
#ifndef EQUIP_H
#define EQUIP_H
#include "platform.h"

extern u8 bombtbx[10], bombtby[10];      /* BOMBTBX, BOMBTBY: vertices of the bolt */
extern u8 bomb_col;                      /* bolt colour (the original uses whatever COL there is) */
extern void (*bomb_line_hook)(u8 x1, u8 y1, u8 x2, u8 y2);   /* tests: each LOIN of the bolt */

void bomb_off(void);                     /* BOMBOFF: draws or erases (XOR) the bolt */
void bomb_on(u8 c);                      /* BOMBON: new bolt (c = incoming carry of the first DORND) */
void bomb_eff(void);                     /* BOMBEFF */
void bomb_eff2(void);                    /* BOMBEFF2: four BOMBEFF */
void bomb_key(void);                     /* part 3: bomb key */
void bomb_frame(void);                   /* part 16: the bomb keeps exploding */
int  warp_blocked(void);                /* 1 if WARP cannot jump (ships or station near, hyperspace, planet or sun too close) */
int  warp(void);                         /* WARP: 1 if it jumps (the view must be repainted), 0 if not (BOOP) */
#endif
