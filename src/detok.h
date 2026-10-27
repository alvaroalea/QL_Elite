/* Text tokens of the original: normal (TT27, QQ18 table) and extended (DETOK, TKN1 and RUTOK tables), with the justified
   printing of DASC/TT26 and output through CHPR (cursor XC, YC). 6502 transcription; whatever depends on the machine
   (drawing a letter, clearing the screen, waiting for a key...) goes through hooks. */
#ifndef DETOK_H
#define DETOK_H
#include "platform.h"

#define DT_LL 30                             /* LL: length of the justified lines */

extern u8 dt_xc, dt_yc;                      /* XC, YC: text cursor (column 0..31, row 0..23) */
extern u8 dtw1, dtw2, dtw3, dtw4, dtw5, dtw6, dtw8;
extern u8 dt_nojustify;                      /* 1 = unjustified text (only break the line between words) */
extern u8 dt_buf[256];                       /* BUF: current line of the justified text */

/* hooks (0 = does nothing) */
extern void (*dt_letter_hook)(u8 ch, u8 xc, u8 yc);  /* draw a letter at (XC, YC) (letter / RR5) */
extern void (*dt_clss_hook)(void);           /* clss: the text went past row 23; clears the screen (XC = YC = 0) */
extern void (*dt_beep_hook)(void);           /* BEEP (character 7) */
extern void (*dt_tt66_hook)(u8 view);        /* TT66 with view A (MT9 and PAUSE -> PAS1) */
extern void (*dt_pause2_hook)(void);         /* PAUSE2: wait for a key */
extern void (*dt_delay_hook)(u8 n);          /* DELAY: n waits (about 20 ms each) */
extern void (*dt_nlin4_hook)(void);          /* NLIN4: line under the title */
extern void (*dt_clyns_hook)(void);          /* CLYNS: clears the bottom part of the screen */
extern void (*dt_name_hook)(u8 which);       /* system names through TT27: 2 = current (ypl), 3 = selected (cpl) */

void dt_bprnt(const u8 *k4, u8 u, u8 c);     /* BPRNT: K(0..3) (high byte first) with u digits; c = point before the last one */
void dt_tt11(u8 a, u8 x, u8 y, u8 c);        /* TT11: (y x) with a digits */
void dt_pr2(u8 x, u8 c);                     /* pr2: x with 3 digits */
void dt_pr5(u8 x, u8 y, u8 c);               /* pr5: (y x) with 5 digits */
void dt_pr6(u8 x, u8 y);                     /* pr6: (y x) with 5 digits, no point */
void dt_csh(void);                           /* csh */
void dt_fwl(void);                           /* fwl */
void dt_tt67(void); void dt_tt69(void); void dt_ttx69(void); void dt_tt60(u8 a); void dt_tt162(void);
void dt_spc(u8 a); void dt_prq(u8 a); void dt_tt73(void); void dt_tt68(u8 a); void dt_plf(u8 a); void dt_plf2(u8 a); void dt_feed(void);
void dt_reset(void);                         /* initial values of DTW1..DTW8 (those of the original on load) */
void dt_chpr(u8 a);                          /* CHPR */
void dt_dasc(u8 a);                          /* DASC / TT26 (with justification if DTW4 bit 7) */
void dt_ex(u8 a);                            /* ex: recursive token A of QQ18 */
void dt_tt27(u8 a);                          /* TT27: normal token (or letter, or control code) */
void detok(u8 a);                            /* DETOK: extended token A of TKN1 */
void detok2(u8 a);                           /* DETOK2: one byte of an extended token */
void detok3(u8 a);                           /* DETOK3: token A of RUTOK */
void dt_dts(u8 a);                           /* DTS: letter with the upper/lower case of DTW1/DTW6/DTW8 */
void dt_mt2(void);                           /* MT2: Sentence Case in the extended tokens */
void dt_mt14(void);                          /* MT14: justified text */
void dt_mt15(void);                          /* MT15: left-aligned text */
u8   dt_rupla(u8 i);                         /* RUPLA (i = 1..TOK_NRU) */
u8   dt_rugal(u8 i);                         /* RUGAL */
u8   dt_nru(void);                           /* number of RUPLA entries */
#endif
