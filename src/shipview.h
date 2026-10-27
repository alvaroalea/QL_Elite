#ifndef SHIPVIEW_H
#define SHIPVIEW_H
#include "ship3d.h"
#define SHIPVIEW_HEAP 256                    /* size of the per-ship line heap */
#define SHIPVIEW_ANG 64                      /* angles: 64 steps per turn */
void shipview_orient(int ay, int ax);        /* INWK orientation: yaw ay and pitch ax (0..63) */
void shipview_pos(int x, int y, int z);      /* position in units of 256 (hi) with sign; z >= 0 */
/* Draws a ship. fresh=1: empty heap (first time). fresh=0: the heap is reused and LL9 erases (XOR) the previous lines
   while drawing the new ones (animation). The caller did state_clear() and plat_init(). */
void shipview_draw(u8 type, int x, int y, int z, int ay, int ax, u8 *heap, u8 *slot, int fresh);
/* Same as shipview_draw with z in 1/256 of a unit (long): lets the ship approach gradually */
void shipview_draw16(u8 type, int x, int y, long z, int ay, int ax, u8 *heap, u8 *slot, int fresh);
int shipview_radius(u8 type);                /* size of the blueprint (largest coordinate of its vertices) */
#endif
