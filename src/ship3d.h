/* Projection and drawing of ships (LL9) with line clipping and a line heap per ship (XOR).
   Port of the Apple II version; follows the 6502 flow. Lines are drawn with ship_loin_hook
   (by default gfx_line in XOR on the 256x136 area). */
#ifndef SHIP3D_H
#define SHIP3D_H
#include "state.h"
#include "emath.h"
#include "ship_math.h"
#include "shipdata.h"

extern u8 ztype;                    /* TYPE: type of the current ship (bit 7 = planet/sun) */
extern const u8 *zxx0;              /* XX0: ship blueprint (see SHIP_BP) */
extern u8 *zxx19;                   /* XX19: ship's line heap (byte 0 = bytes used; then 4 per line) */
extern u8 *zinf;                    /* INF: ship block in K% (its bytes 28 and 30 are set to 0 on exploding) */
extern u8 zxx4, zxx13, zxx17, zxx20, zcnt, zswap, zlsnum, zlsnum2;
extern u8 zxx18[9];
extern u8 xx3[258];                 /* XX3: screen coordinates of the vertices (4 bytes each) */
#define xx2 zk3                     /* XX2: visible faces (shares memory with K3, like the original) */
extern u8 ship_c_in;                /* carry with which LL9 is entered (used by the first DORND on exploding) */

/* Hooks: LOIN (draws XX15 = X1,Y1,X2,Y2), DOEXP (explosion) and PLANET (planet/sun slot) */
extern void (*ship_loin_hook)(void);
extern void (*ship_doexp_hook)(void);
extern void (*ship_planet_hook)(void);

void ship_ll9(void);                /* draws (or erases) the ship in INWK */
void ship_ll9_faces_c(const u8 *v);  /* LL86..LL88 in C (reference for ql_ll9_faces; the tests call them) */
void ship_ll9_verts_c(void);
void ship_ll9_edges_c(const u8 *v);   /* LL75..LL81 in C (reference for ql_ll9_edges) */        /* LL48..LL50 in C (reference for ql_ll9_verts) */
void ship_ll51(void);               /* XX12 = XX15 (vector) by the matrix XX16 */
void ship_shppt(void);              /* distant ship: a dot */
u8   ship_clip(void);               /* clips XX15/XX12; returns 1 if the line ends up outside */
u8   ship_clip2(u8 a);              /* CLIP2 entry (a = XX15+5) */
void ship_lsput(void);              /* stores the line in the heap and draws the new one and erases the old */
extern u8 ship_dbg_skip;           /* DEBUG (SHIFT-D): bit 0 = no maths, bit 1 = no drawing */
void ship_redraw_heap(void);        /* DEBUG: paints the lines of the heap twice (drawing cost without LL9) */
void ship_lsclr(void);              /* erases the lines of the heap that are left unreplaced */
#endif
