/* Instrument panel (dashboard) of Apple II Elite, unverified port (see docs/PENDIENTES.md).
   The background is a bitmap the original loads from disk (A.SCREEN): here it is converted with tools/gen_dashboard.py to
   data/elite_dash (a 256 x 56 pixel strip in mode 8) and loaded from <drive>elite_dash. If the file is missing a minimal panel
   (frame, tracks and labels) is drawn so the instruments work the same.
   Instruments (DIALS): speed, roll, pitch, 4 energy banks, front and rear shields, fuel, altitude,
   cabin and laser temperature, and the compass (COMPAS). Everything is drawn in XOR: erase = redraw. */
#ifndef DASH_H
#define DASH_H
#include "platform.h"

#define DASH_Y0     160                 /* first line of the panel on screen (the view takes 0..159; the original has it at 136, see view_set_native and gfx_yoff_set) */
#define DASH_LINES  56
#define DASH_BYTES  (DASH_LINES * 128)  /* 7168 bytes: 56 lines of 128 bytes (mode 8) */

#define DASH_OK        0
#define DASH_E_OPEN   (-1)
#define DASH_E_FORMAT (-2)

int  dash_load(void);                   /* reads <drive>elite_dash; DASH_OK, DASH_E_OPEN or DASH_E_FORMAT */
int  dash_loaded(void);                 /* 1 if the bitmap is in memory */
int  dash_loads(void);                  /* times the file has been read from disk: once loaded it stays in memory */
void dash_show(void);                   /* draws the background (or the minimal panel) and resets the instruments */
void dash_reset(void);                  /* clears the memory of what was drawn last (after repainting the background) */
void dash_update(void);                 /* DIALS: updates the instruments and the compass */
void dash_msbar(u8 x, u8 color);         /* MSBAR: indicator X (1..4) with the original colour (0 = off) */
void dash_msblob(void);                 /* msblob: missile indicators (NOMSL in green, the rest off) */
extern u8 dash_comz;                    /* z (sign-magnitude, 96 = 1) of the vector towards the station calculated by COMPAS */
void dash_spblb(void);                  /* SPBLB: switches (XOR) the station-near S bulb on or off */
void dash_mblb(u8 on);                  /* own: red M bulb (J jump blocked) in the free corner of the radar; only paints if it changes */
void dash_mblb_forget(void);            /* the panel has been repainted: the M does not count as lit */
void dash_ecblb(void);                  /* ECBLB: switches (XOR) the ECM E bulb on or off */
extern void (*dash_trace)(char kind, u8 a, u8 b, u8 c, u8 d);   /* tests: 'h' x1 x2 y col, 'v' x y1 y2 col, 'p' x y zz, 'c' x y col (original colours) */
extern u8 dash_dials[13], dash_dialc[13], dash_comx, dash_comy, dash_comc, dash_mscol[4];   /* dials, dialc, COMX, COMY, COMC and mscol of the original */
#endif
