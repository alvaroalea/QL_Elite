#include "missions.h"
#include "lines.h"
#include "detok.h"
#include "state.h"
#include "emath.h"
#include "ship_math.h"
#include "ship3d.h"
#include "shipdata.h"
#include "ships.h"
#include "move.h"
#include "space.h"
#include "sun.h"
#include "combat.h"
#include "damage.h"
#include "game.h"
#include "universe.h"
#include "text.h"
#include "input.h"
#include "sound.h"
#include "menu.h"
#include "config.h"
#include "cmdr.h"

#define CONHIEGHT 80                             /* conhieght: gap left for the spinning Constrictor */

/* ---------------------------------------------------------------- screen hooks of the token engine */

static u8 mis_ink = C_WHITE;
static u8 mis_title_ink = 0;                     /* ink of the first line (row 1: the title) of STATUS and DATA; 0 = the normal one */

static void letter(u8 ch, u8 xc, u8 yc)
{
    if ((uptog & 0x80) && ch >= '[') ch = (u8)(ch - 0x20);      /* RR5: UPTOG, capitals only (text screens) */
    txt_color((mis_title_ink && yc == 1) ? mis_title_ink : mis_ink, C_BLACK);
    txt_at(xc, yc);
    txt_putc(ch);
}

static void clss(void) { gfx_clear(C_BLACK); }
static void beep(void) { snd_bell(); }

static void pause2(void)                         /* PAUSE2: a key is released and pressed */
{
    int k;
    do k = in_wait(); while (k < 0 && !in_quit);
}

static void delay(u8 n)                          /* DELAY: about 20 ms per iteration (one frame) */
{
    u8 i;
    if (in_script) return;
    for (i = 0; i < n; i++) in_poll();
}

/* system names through TT27 (cpl and ypl): the letters go through TT27 one by one, like the two-letter tokens */
static void name(u8 which)
{
    const char *s = uni_galaxy[which == 3 ? tgt_sys : cur_sys].name;
    u8 c;
    for (; *s; s++) {
        c = (u8)*s;
        if (c >= 'a' && c <= 'z') c = (u8)(c - 32);
        dt_tt27(c);
    }
}

/* ---------------------------------------------------------------- Drawing the screen: TT66, TTX66, TTX66K, BOX, CLYNS */

u8 scr_text = 0;                                 /* text: bit 7 = text view (Apple II text mode) */
u8 scr_viewdir = 0;                              /* VIEW: space view (0 front, 1 rear, 2 left, 3 right) for TTX66 */
void (*scr_clear_hook)(int graphics) = 0;        /* screen clearing (cleargrap or cleartext); 0 = the default clear */
void (*scr_tt27)(u8 a) = dt_tt27;                /* tests: the tokens TTX66 prints */
void (*scr_clearrow_hook)(u8 row) = 0;           /* tests: each clearrow of CLYNS */
void (*scr_chpr)(u8 a) = dt_chpr;                /* tests: the spaces of CLYNS */
void (*scr_trace)(char k, u8 a) = 0;             /* tests: MT2 (m), FLFLLS (f) and ee3 (e) of TTX66 */

/* BOX: blue line from 0 to 255 on row 0 (HLOIN: covers 0..253) and the corners (bytes 1 and 37 of the row, %10101010: dot 254
   also stays lit) */
void scr_box(void)
{
    if (view_native) return;                   /* 160-line flight view: without the blue line at the top (own) */
    lines_hloin(0, 255, 0, C_BLUE);
    gfx_pixel(254, 0, C_BLUE);
}

/* TTX66K: the graphic views (space, QQ11 = 13 and the charts) clear the top part (cleargrap, 17 text rows) and
   draw BOX; the text ones clear the text screen (the QL has no text mode: the whole screen is cleared) */
static void ttx66k(void)
{
    int graphics = (qq11 == 0 || qq11 == 13 || (qq11 & 0xC0));
    if (scr_clear_hook) scr_clear_hook(graphics);
    else if (!graphics) gfx_clear(C_BLACK);
    else {
        if (qq11 == 13) gfx_clear(C_BLACK);      /* the title and mission 1 of the port take up the whole screen */
        else gfx_fill_lines(0, clip_h, C_BLACK);
        scr_box();
    }
    if (graphics) { dt_xc = 0; dt_yc = 0; scr_text = 0; }       /* cleargrap: XC = YC = 0; HGR */
    else scr_text = 0x80;                                       /* TEXT */
}

/* TT66 / TTX66: view A; clears, resets the text (MT2, Sentence Case, XC = YC = 1), FLFLLS on the graphic ones, LAS2, DLY and de to 0,
   the hyperspace counter at (1, 1) if it is counting, and in the space view the name ("FRONT VIEW") at column 11 */
void tt66(u8 view)
{
    qq11 = view;
    dt_mt2();
    if (scr_trace) scr_trace('m', 0);
    circ_lsp = 0;                                /* LSP = 0 */
    qq17 = 0x80;
    dtw2 = 0x80;
    dt_xc = 1; dt_yc = 1;
    ttx66k();
    if (!(scr_text & 0x80)) {                    /* FLFLLS (and the port forgets what was drawn) */
        if (scr_trace) scr_trace('f', 0);
        sun_flflls(); ships_forget_lines(); sun_forget();
        circ_lsp = 0;                            /* sun_forget sets LSP = 1; TTX66 leaves it at 0 */
    }
    laser_forget();                              /* LAS2 = 0 */
    dly = 0;
    mess_de = 0;
    if (qq22[1]) {                               /* ee3: the counter with 3 digits at (1, 1) */
        if (scr_trace) scr_trace('e', qq22[1]);
        else { dt_xc = 1; dt_yc = 1; dt_tt11(3, qq22[1], 0, 0); }
    }
    dt_yc = 1;                                   /* OLDBOX */
    if (qq11 == 0 && !view_native) {             /* in the native flight view the name already appears under the panel (own) */
        dt_xc = 11;
        scr_tt27((u8)(scr_viewdir | 0x60));      /* FRONT, REAR, LEFT or RIGHT */
        scr_tt27(' ');                           /* TT162 */
        scr_tt27(175);                           /* VIEW */
    }
    dt_xc = 1; dt_yc = 1;                        /* tt66 */
    qq17 = 0;
}

void mis_tt66(u8 view) { tt66(view); }

/* CLYNS: DLY = de = 0; in the text views, 64 spaces from (1, 21); in the graphic ones text rows 15 and 16 are erased
   (y = 120..135) and the cursor stays at (1, 15). DTW2 = $FF and QQ17 = $80 */
void clyns(void)
{
    u8 x;
    dly = 0;
    mess_de = 0;
    dt_yc = 21; dt_xc = 1;                       /* CLYS1 */
    dtw2 = 0xFF;
    qq17 = 0x80;
    if (scr_text & 0x80) {
        for (x = 64; x != 0; x--) scr_chpr(' ');
        dt_yc = 21; dt_xc = 1;
        return;
    }
    dt_yc = 15; dt_xc = 1;                       /* CLY1 */
    if (scr_clearrow_hook) { scr_clearrow_hook(15); scr_clearrow_hook(16); }
    else gfx_fill_lines(120, 16, C_BLACK);
}

static void nlin4_h(void) { nlin4(); }                         /* NLIN4 (lines.c) */

void missions_hooks(void)
{
    dt_letter_hook = letter;
    dt_clss_hook = clss;
    dt_beep_hook = beep;
    dt_tt66_hook = mis_tt66;
    dt_pause2_hook = pause2;
    dt_delay_hook = delay;
    dt_name_hook = name;
    dt_nlin4_hook = nlin4_h;
    dt_clyns_hook = clyns;
}

/* ---------------------------------------------------------------- mission routines */

void pas1(void) { mis_tt66(1); }                 /* PAS1 */
void mis_pause(void) { pause2(); pas1(); }       /* PAUSE */
void bris(void) { detok(216); delay(100); }      /* BRIS: INCOMING MESSAGE and 2 seconds */

static int brp(u8 a) { detok(a); return bay(); }      /* BRP: the token and BAY (status screen) */

int brief2(void) { tp = (u8)(tp | 0x04); return brp(11); }
int brief3(void) { tp = (u8)((tp & 0xF0) | 0x0A); return brp(222); }

int debrief2(void)
{
    tp = (u8)(tp | 0x04);                        /* mission 2 completed */
    engy = 2;                                    /* navy energy unit */
    tally[1]++;
    return brp(223);
}

int debrief(void)
{
    tp = (u8)(tp & 0xFE);                        /* LSR TP; ASL TP */
    sm_mcash(50000 & 255, 50000 >> 8);           /* 5.000 CR */
    return brp(15);
}

void (*mis_ll9_hook)(void) = ship_ll9;
void (*mis_mveit_hook)(void) = mveit;

static void frame(void)                          /* LL9 and MVEIT of the Constrictor from INWK */
{
    if (!in_script) in_poll();
    mis_ll9_hook();
    mis_mveit_hook();
}

int brief(void)
{
    int n;
    u8 x;
    tp = (u8)((tp & 0xFE) | 1);                  /* LSR TP; SEC; ROL TP: mission 1 in progress */
    bris();
    ship_zinf();
    ztype = SHIPT_CONSTRICTOR;
    n = ship_new(SHIPT_CONSTRICTOR);
    dt_xc = 1;
    inwk[7] = 1;                                 /* z_hi = 1 */
    mis_tt66(13);
    mcnt = 64;
    if (n >= 0) { zinf = state_slot(n); zxx19 = ships_heap(n); }
    zxx0 = ship_bp_for(SHIPT_CONSTRICTOR);
    move_xsav = 0;
    ships_hooks();
    do {                                         /* BRL1: spins 64 iterations */
        inwk[29] = 0x7F;
        inwk[30] = 0x7F;
        frame();
        mcnt--;
    } while (mcnt && !in_quit);
    for (;;) {                                   /* BRL2: moves away upwards */
        if (in_quit) break;
        inwk[0] = (u8)(inwk[0] >> 1);
        if (++inwk[6] == 0) break;
        if (++inwk[6] == 0) break;
        x = (u8)(inwk[3] + 1);
        if (x >= CONHIEGHT) x = CONHIEGHT;
        inwk[3] = x;
        frame();
        mcnt--;
    }
    inwk[7]++;                                   /* BR2 */
    pas1();
    return brp(10);
}

/* DOENTRY (the missions part): which session applies (MIS_*). Mission 1 in galaxies 1 and 2 with Competent rank;
   mission 2 in galaxy 3 */
int missions_choose(void)
{
    u8 a = (u8)(tp & 3);
    if (a == 0) {
        if (tally[1] == 0) return MIS_NONE;
        if (gcnt >> 1) return MIS_NONE;
        return MIS_BRIEF;
    }
    if (a == 3) return MIS_DEBRIEF;              /* EN1 */
    if (gcnt != 2) return MIS_NONE;              /* EN2 */
    a = (u8)(tp & 0x0F);
    if (a == 0x02) return tally[1] < 5 ? MIS_NONE : MIS_BRIEF2;
    if (a == 0x06) return (qq0 == 215 && qq1 == 84) ? MIS_BRIEF3 : MIS_NONE;      /* EN3: at Ceerdi */
    if (a == 0x0A) return (qq0 == 63 && qq1 == 72) ? MIS_DEBRIEF2 : MIS_NONE;     /* EN5: at Birera with the plans */
    return MIS_NONE;                             /* EN4: BAY */
}

int bay(void) { qq12 = 0xFF; return SC_STATUS; }       /* BAY: docked and to the status screen (FRCE with f8) */

int missions_doentry(void)
{
    missions_hooks();
    space_reset();                               /* RES2 */
    ships_clear();
    zDELTA = 0;
    switch (missions_choose()) {
    case MIS_BRIEF:    return brief();
    case MIS_DEBRIEF:  return debrief();
    case MIS_BRIEF2:   return brief2();
    case MIS_BRIEF3:   return brief3();
    case MIS_DEBRIEF2: return debrief2();
    }
    return bay();
}

/* PDESC (the RUPLA/RUGAL/RUTOK part): special description of system zz if we are docked in it. Returns 1 if it
   printed it (in dt_xc/dt_yc); 0 if the normal description applies. RUPLA has 26 entries: the original sets NRU% = 0 and
   walks 256 memory positions (a bug that hangs it in some systems); here only the 26 are walked */
int pdesc(u8 zz, const u8 *q15)
{
    u8 y, g, x;
    if (qq8[0] | qq8[1]) goto pd1;
    if (!(qq12 & 0x80)) goto pd1;
    for (y = dt_nru(); y; y--) {                 /* PDL1 */
        if (dt_rupla(y) != zz) continue;
        g = dt_rugal(y);
        if ((u8)(g & 0x7F) != gcnt) continue;
        if (g & 0x80) detok2(176);               /* PD3 */
        else {
            if (!(tp & 1)) goto pd1;             /* only during mission 1 */
            dt_mt14();
            detok2(1);                           /* LDA #1 (skips the LDA #176): DETOK2 with jump token 1 (MT1) */
        }
        detok3(y);
        detok(177);                              /* PD4 */
        return 1;
    }
pd1:
    if (!q15) return 0;
    for (x = 0; x < 4; x++) em_rand[x] = q15[2 + x];   /* PD1: RAND = QQ15+2..5 */
    detok(5);                                    /* the description ("goat soup") with the token engine */
    return 0;
}


/* ---------------------------------------------------------------- TT25: system data */


/* TT25 with the raw system data (QQ3..QQ7, QQ15, QQ8, ZZ) */
void tt25_raw(const u8 *q15, u8 q3, u8 q4, u8 q5, u8 q6, u8 p7lo, u8 p7hi, u8 d8lo, u8 d8hi, u8 zz)
{
    u8 a, q19;
    if (dt_tt66_hook) dt_tt66_hook(1);           /* TRADEMODE with view 1 */
    dt_xc = 9;
    dt_tt27(163);                                /* NLIN3: "DATA ON <sistema>" */
    if (dt_nlin4_hook) dt_nlin4_hook();
    dt_ttx69();
    if (d8lo | d8hi) {                           /* TT146 */
        dt_tt68(191);
        dt_pr5(d8lo, d8hi, 1);
        dt_tt60(195);
    } else dt_yc++;
    dt_tt68(194);                                /* ECONOMY: */
    a = (u8)((q3 + 1) >> 1);
    if (a == 2) dt_tt27(173);                    /* TT70: MAINLY */
    else {
        a = q3;
        if (((q3 + 1) >> 1) > 2) a = (u8)(a - 5); /* SBC #5 */
        dt_tt27((u8)(a + 170));                  /* TT71 */
    }
    dt_tt60((u8)((q3 >> 2) + 168));              /* TT72 */
    dt_tt68(162);                                /* GOVERNMENT: */
    dt_tt60((u8)(q4 + 177));
    dt_tt68(196);                                /* TECH.LEVEL: */
    dt_pr2((u8)(q5 + 1), 0);
    dt_ttx69();
    dt_tt68(192);                                /* POPULATION: */
    dt_pr2(q6, 1);
    dt_tt60(198);
    dt_tt27('(');
    if (q15[4] & 0x80) {                         /* TT75: alien species */
        a = (u8)(q15[5] >> 2);
        if ((a & 7) < 3) dt_spc((u8)((a & 7) + 227));
        a = (u8)(a >> 3);
        if (a < 6) dt_spc((u8)(a + 230));
        q19 = (u8)((q15[3] ^ q15[1]) & 7);
        if (q19 < 6) dt_spc((u8)(q19 + 236));
        dt_tt27((u8)((((q15[5] & 3) + q19) & 7) + 242));
    } else dt_tt27(188);                         /* HUMAN COLONIAL */
    dt_tt27('S');                                /* TT76 */
    dt_tt60(')');
    dt_tt68(193);                                /* GROSS PRODUCTIVITY: */
    dt_pr6(p7lo, p7hi);
    dt_tt162();
    qq17 = 0;
    dt_tt27('M');
    dt_tt60(226);                                /* " CR" */
    dt_tt68(250);                                /* AVERAGE RADIUS: */
    dt_pr5(q15[3], (u8)((q15[5] & 15) + 11), 0);
    dt_tt162();
    dt_dasc('k');
    dt_dasc('m');
    dt_ttx69();
    pdesc(zz, q15);
}

/* TT25 of system n of the current galaxy, with the distance in QQ8 */
void tt25(int n)
{
    const uni_system *p = &uni_galaxy[n];
    u8 q15[6];
    q15[0] = (u8)(p->seed0 & 255); q15[1] = (u8)(p->seed0 >> 8);
    q15[2] = p->goatsoupseed.a; q15[3] = p->goatsoupseed.b; q15[4] = p->goatsoupseed.c; q15[5] = p->goatsoupseed.d;
    dt_nojustify = 1;                                  /* data screen: normal text, breaking between words */
    tt25_raw(q15, p->economy, p->govtype, p->techlev, p->population, (u8)(p->productivity & 255), (u8)(p->productivity >> 8),
             qq8[0], qq8[1], (u8)n);
    dt_nojustify = 0;
}

void missions_set_ink(u8 c) { mis_ink = c; }
void missions_set_title_ink(u8 c) { mis_title_ink = c; }

/* ---------------------------------------------------------------- STATUS (status screen) */

static void mis_tt111_default(void) { game_tt111(); }
void (*mis_tt111_hook)(void) = mis_tt111_default;      /* TT111: the tests replace it */

/* STATUS: 6502 transcription with the token engine (cmn, BAY and the title "COMMANDER <name>" come from tokens) */
void status_mode(void)
{
    u8 a, x, k, y;
    u8 *eq[4];
    eq[0] = &bomb; eq[1] = &engy; eq[2] = &dkcmp; eq[3] = &ghyp;      /* BOMB, ENGY, DKCMP, GHYP: contiguous in the original */
    if (dt_tt66_hook) dt_tt66_hook(8);           /* TRADEMODE with view 8 */
    if (mis_tt111_hook) mis_tt111_hook();        /* TT111 */
    dt_xc = 7;
    dt_tt27(126);                                /* NLIN3 */
    if (dt_nlin4_hook) dt_nlin4_hook();
    if (qq12) {                                  /* wearedocked */
        detok(205);
        dt_tt67();
    } else {
        a = 230;                                 /* green; with ships nearby: yellow or red according to ENERGY */
        if (frin[2 + junk_count]) a = (u8)(a + 1 + (energy >= 128));
        dt_plf(a);                               /* st6 */
    }
    dt_spc(125);
    a = 19;                                      /* legal status */
    if (fist) a = (u8)(20 + (fist >= 50));
    dt_plf(a);                                   /* st5 */
    dt_spc(16);
    if (tally[1]) {                              /* st4 */
        x = 9;
        if (tally[1] < 25) { x = 8; if (tally[1] < 10) { x = 7; if (tally[1] < 2) x = 6; } }
    } else {
        x = 0; a = (u8)(tally[0] >> 2);
        do { x++; a = (u8)(a >> 1); } while (a);
    }
    dt_plf((u8)(x + 21));                        /* st3: rank */
    dt_plf2(18);
    if (escp) dt_plf2(112);
    if (bst) dt_plf2(111);
    if (ecm_fitted) dt_plf2(108);
    for (k = 0; k < 4; k++) if (*eq[k]) dt_plf2((u8)(113 + k));       /* stqv */
    for (x = 0; x < 4; x++) {                    /* st */
        y = laser[x];
        if (!y) continue;
        dt_spc((u8)(96 + x));
        a = 103;
        if (y == LASER_BEAM) a = 104;
        if (y == LASER_MIL) a = 117;
        if (y == LASER_MINING) a = 118;
        dt_plf2(a);
    }
}

