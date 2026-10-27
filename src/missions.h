/* Missions of the original: mission 1 (the stolen Constrictor) and mission 2 (the Thargoid plans). The mission
   screens are extended tokens of the original printed with the detok.c engine. */
#ifndef MISSIONS_H
#define MISSIONS_H
#include "platform.h"
#define MIS_NONE 0
#define MIS_BRIEF 1
#define MIS_DEBRIEF 2
#define MIS_BRIEF2 3
#define MIS_BRIEF3 4
#define MIS_DEBRIEF2 5
extern void (*mis_ll9_hook)(void);     /* LL9 and MVEIT of the Constrictor in BRIEF (the tests replace them) */
extern void (*mis_mveit_hook)(void);
int  missions_choose(void);        /* the part of DOENTRY that decides the informative session (MIS_*) */
void missions_set_title_ink(u8 c);  /* colour of the first line (the title) of STATUS and DATA; 0 = the normal one */
void missions_hooks(void);         /* connects the token engine to the screen, keyboard and names */
int  bay(void);                    /* BAY: QQ12 = $FF and status screen */
int  missions_doentry(void);       /* DOENTRY after docking: RES2 and the informative session that applies; returns SC_STATUS (BAY) */
int  brief(void);                  /* BRIEF: mission 1, with the Constrictor spinning */
int  brief2(void);                 /* BRIEF2: mission 2, go to Ceerdi */
int  brief3(void);                 /* BRIEF3: at Ceerdi, the plans */
int  debrief(void);                /* DEBRIEF: Constrictor destroyed, 5,000 CR */
int  debrief2(void);               /* DEBRIEF2: plans delivered at Birera, energy unit */
void bris(void);                   /* BRIS */
void pas1(void);                   /* PAS1 */
void mis_pause(void);              /* PAUSE */
void mis_tt66(u8 view);            /* TT66 of the mission screens (= tt66) */
extern u8 scr_text, scr_viewdir;
extern void (*scr_clear_hook)(int graphics);
extern void (*scr_tt27)(u8 a);
extern void (*scr_clearrow_hook)(u8 row);
extern void (*scr_chpr)(u8 a);
extern void (*scr_trace)(char k, u8 a);
void tt66(u8 view);                 /* TT66/TTX66/TTX66K (and BOX in the graphic views) */
void scr_box(void);                 /* BOX */
void clyns(void);                   /* CLYNS */
int  pdesc(u8 zz, const u8 *q15);  /* PDESC: special description of system zz (1) or, with q15 (QQ15), the normal one (0) */
void tt25_raw(const u8 *q15, u8 q3, u8 q4, u8 q5, u8 q6, u8 p7lo, u8 p7hi, u8 d8lo, u8 d8hi, u8 zz);  /* TT25 */
void tt25(int n);                  /* TT25: data of system n (QQ8 = distance) */
void status_mode(void);            /* STATUS: status screen (tokens of the original) */
extern void (*mis_tt111_hook)(void);  /* TT111 of STATUS (the tests replace it) */
void missions_set_ink(u8 c);       /* colour of the token engine's letters */
#endif
