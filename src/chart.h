/* Galactic and local charts (TT22, TT23, TT14, TT15, TT103, TT105 of the reference source), with SUN and CIRCLE.
   Unverified port (see docs/PENDIENTES.md). Geometry of the source: 192 x 96 pixel galactic chart (x 32..223, y 19..115,
   scale 0.75 in x and 0.375 in y); local chart with 3 pixels per unit in x and 1.5 in y, centred at (105, 75). */
#ifndef CHART_H
#define CHART_H
#include "platform.h"

#define CHART_GCYT 19                 /* GCYT: y of the top of the galactic chart */
#define CHART_GCYB 115                /* GCYB = GCYT + 0.75 * 128 */

#define CHART_TOP_FULLWIDTH 1         /* 1 = line under the title from 0 to 255 like the original (NLIN5); 0 = only closes the frame (x 30..227) */

extern void (*chart_line_hook)(u8 kind, u8 a, u8 b, u8 c);   /* tests: HLOIN (0: x1, x2, y) and VLOIN (1: x, y1, y2) */
void chart_tt15(int local, u8 x, u8 y, u8 size);   /* TT15 with QQ19 = (x, y, size); local = QQ11 negative */
extern void (*chart_event_hook)(char kind, u8 a, u8 b, u8 c);
void chart_tt16(int local, u8 x, u8 y);  /* TT16: moves the crosshair (X to the right, Y up) with TT123 and redraws it */
void chart_draw(int local);           /* draws the whole chart (on an already cleared screen) */
void chart_sight(u8 view);            /* SIGHT: laser sight of the view (0..3) in the space view (after erasing the view) */
void chart_sight_forget(void);        /* the view has been erased without repainting the sight */
void chart_sight_update(u8 view);     /* every frame: cross or X shape (station near) according to the S */
void chart_cursor(int local);         /* TT103: cross of the target system (XOR: two calls erase it) */
void chart_ping_set(void);        /* ping without drawing */
void chart_ping(int local);           /* ping + TT103: the cursor returns to the current system (O key) */
void chart_hm(int local);             /* hm: system nearest the cursor + cross (D key) */
int  chart_find_system(const char *name);   /* HME2: index of the system with that name (case-insensitive), or -1 */
#endif
