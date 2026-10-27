/* Game lines (Bresenham) and the transcribed original LOIN, in game coordinates on the QL screen */
#ifndef LINES_H
#define LINES_H
#include "platform.h"

extern void (*loin_hook)(u8 x1, u8 y1, u8 x2, u8 y2);    /* tests: each LOIN with the endpoints of the call */
void loin_draw(u8 x1, u8 y1, u8 x2, u8 y2, u8 c);       /* XOR line from (x1, y1) to (x2, y2): pure Bresenham with both endpoints (assembly on the QL) */
void loin_bres_c(u8 x1, u8 y1, u8 x2, u8 y2, u8 c);     /* the same line in C (PC, and reference for ql_line.asm) */
void loin_draw_6502(u8 x1, u8 y1, u8 x2, u8 y2, u8 c);  /* LOIN transcribed from the original (verified; the game no longer uses it) */
extern void (*lines_hloin_hook)(u8 x1, u8 x2, u8 y, u8 c);  /* tests: each HLOIN of lines_hloin */
void lines_hloin(u8 x1, u8 x2, u8 y, u8 c);                 /* HLOIN (gfx_hloin) with the test hook */
void nlin2(u8 y);                                           /* NLIN2: HLOIN from (0, y) to (255, y) in blue */
void nlin4(void);                                           /* NLIN4: NLIN2 with y = 19 (the QL has no text mode: see NLI4) */
#endif
