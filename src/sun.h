/* SUN, CIRCLE and their support routines (Elite Apple II, unverified port: see docs/PENDIENTES.md).
   SUN draws a sun/planet filled line by line with an irregular edge (K3, K4 = 16-bit centre, K = radius);
   the LSO heap stores the half-width of each line to erase the previous drawing. CIRCLE/CIRCLE2 draw a circle
   of K pixels radius with segments (BLINE + LL145 clipping); the LSX2/LSY2 heap stores the points to erase it. */
#ifndef SUN_H
#define SUN_H
#include "platform.h"

extern int sun_yx2m1;                     /* Yx2M1: last line of the view area (135; 133 in the virtual space of the native view) */
#define SUN_YX2M1 sun_yx2m1
void view_virtual_begin(void);            /* native view: planets, suns and circles are calculated in the original's space (136 lines) and drawn with 6/5 vertically */
void view_virtual_end(void);

extern u8 sun_col;                        /* COL: drawing colour (QL palette index) */
extern u8 sun_lso[256];                   /* LSO: half-width of each line of the sun */
extern u8 sun_lsx;                        /* LSX: state of the line heap ($FF = empty) */
extern u8 circ_lsx2[256], circ_lsy2[256]; /* LSX2, LSY2: points of the last circle */
extern u8 circ_lsp, circ_flag, circ_stp, circ_cnt;   /* LSP, FLAG, STP, CNT */

void chart_xf_set(int mode);              /* drawing transformation of the charts: 0 none, 1 local (4:3, centred and bigger), 2 galactic (8 dots down and stretched 6/5) */
int  chart_xf_mode(void);
int  sun_xf_x(int x);                     /* screen coordinates from the original's */
int  sun_xf_y(int y);
void sun_xf_hloin(int x1, int x2, int y, u8 col);   /* gfx_hloin with the transformation */
void sun_xf_vloin(int x, int y1, int y2, u8 col);
void sun_xf_circle(int cx, int cy, int k, u8 col);   /* chart circle: midpoint ellipse with the chart's aspect */
void sun_xf_disc(int cx, int cy, int k, u8 col);   /* short chart planet (1-dot filled ellipse) */
void sun_xf_dust(int x, int y, u8 zz);
void sun_flflls(void);                    /* FLFLLS: empties the sun's line heap */
u8   sun_chkon(void);                     /* CHKON: 1 if the circle (K3, K4, K) falls outside the view */
void sun_draw(void);                      /* SUN: draws (XOR) the sun of K3, K4, K and erases the previous one */
void sun_wpls(void);                      /* WPLS: erases the previous sun */
u8   circle_draw(void);                   /* CIRCLE: returns the carry (1 = outside the view) */
u8   circle2_draw(void);                  /* CIRCLE2: like CIRCLE without CHKON and with STP already set; returns 0 */
void circle_wpls2(void);
void circle_update(void);                 /* native view: PL9 = erases the previous circle and draws the new one (nothing if unchanged); only with view_native */
int  sun_rows_get(int y, int *lo, int *hi);   /* tests: span [lo, hi] that the native view sun has lit on row y (0 if empty) */                  /* WPLS2: erases the previous circle */
extern void (*sun_hiss_hook)(int y);      /* sound of each tunnel ring (SOHISS) */
void sun_get_sunx(u8 *v);                 /* tests: SUNX */
void sun_set_sunx(u8 lo, u8 hi);
extern void (*sun_loin_hook)(void);       /* tests: each LOIN of the circle (X1..Y2 in zx15 already sorted, zswap) */
extern void (*sun_hloin_hook)(u8 l, u8 r, u8 y);   /* tests: each HLOIN of the sun that draws */
void sun_forget(void);                    /* after erasing the view: neither the sun nor the planet count as drawn */
extern void (*sun_blip_hook)(int x, int y);  /* SOBLIP of LL164 */
extern void (*sun_tt66_hook)(void);          /* TT66 of HFS2 */
extern void (*sun_circle_hook)(u8 k3, u8 k4, u8 k, u8 stp);
extern void (*sun_pace_hook)(void);              /* own: rhythm of the tunnel rings */
void tunnel_laun(void);                      /* LAUN */
void tunnel_ll164(void);                     /* LL164 */
void tunnel_draw(u8 stp);                 /* HFS2/HFS1: hyperspace tunnel (STP = 4) or launch tunnel (STP = 8) with the colour sun_col */
#endif
