/* Game logic coupled to the state: current and target system, market, trade, equipment, hyperspace.
   No drawing or keyboard: tested on the PC (tests/test_game.c). The rules follow Apple II Elite
   (TT151/TT219/TT208/tnpr/EQSHP/refund); differences are marked with "DIFERENCIA". */
#ifndef GAME_H
#define GAME_H
#include "platform.h"
#include "universe.h"

#define NGOODS 17                /* 16 commodities + alien items */
#define NEQUIP 14                /* EQSHP list */

extern int cur_sys;              /* index of the current system in uni_galaxy */
extern int tgt_sys;              /* system selected for hyperspace */
extern uni_market mkt;           /* prices (tenths of a credit) and quantities of the moment */
extern u8 game_visit_rnd;        /* test hook: if !=0 it is used instead of DORND for qq26 on arriving at a system */

/* ---- money: CASH is a 32-bit big-endian integer (tenths of a credit) ---- */
unsigned long cash_get(void);
void cash_set(unsigned long v);
int  cash_spend(unsigned long v);        /* 1 if there was a balance and subtracts it; 0 if not (touches nothing) */
void cash_earn(unsigned long v);         /* addition saturating at 0xFFFFFFFF */

/* ---- game ---- */
void game_new(void);                     /* state_clear + default commander + galaxy 1, starting system (Lave) */
void game_after_load(void);              /* after loading a file: rebuilds galaxy, current system, market (with the saved avl) */
void game_enter_system(int idx, int fresh);   /* sets qq0..qq8 of the system; fresh = new visit: random qq26, new market */
void game_set_target(int idx);
void game_tt111(void);                   /* TT111 from (QQ9, QQ10): the selection may pass to the lowest-index twin */
void game_target_only(int idx);          /* target and qq8 without moving the crosshair (qq9/qq10): the chart crosshair is free (TT16) */           /* qq9/qq10/qq8 of the target */

/* ---- trade (commodity index 0..16) ---- */
#define TR_OK       0
#define TR_NOCASH   1
#define TR_NOROOM   2
#define TR_NOSTOCK  3
#define TR_NONE     4                    /* quantity 0 or nothing to sell */
/* TRADE_FAITHFUL = 1: 6502 arithmetic as is (verified against the original, see tools/gen_trade.py):
     - tnpr adds in 8 bits with carry: for kg/g, 198 + 63 wraps (=5) and the original says it "fits";
     - GCASH computes amount = price * quantity * 4 in 16 bits: selling >= 64K tenths of a credit at once loses the high bits.
   TRADE_FAITHFUL = 0: corrected arithmetic (no wrapping, 32-bit amount). */
#ifndef TRADE_FAITHFUL
#define TRADE_FAITHFUL 1
#endif
int  trade_fits(int good, unsigned qty); /* 1 if qty of that commodity fits in the hold (clear carry of tnpr) */
int  trade_hold_t(void);                 /* tonnes occupied (commodities 0..12) + tribbles */
int  trade_room(int good);               /* how much more of that commodity fits (tnpr) */
unsigned trade_price(int good);          /* tenths of a credit per unit in this system */
int  trade_buy(int good, unsigned qty);
int  trade_sell(int good, unsigned qty);
unsigned trade_max_buy(int good);        /* what can be bought now: min(stock, money/price, space) */

/* ---- equipment (PRXS indices: 0 fuel, 1 missile, 2 large cargo bay, 3 ECM, 4 pulse laser, ..., 13 mining laser) ---- */
#define EQ_OK      0
#define EQ_NOCASH  1
#define EQ_PRESENT 2                     /* you already have it (or already carry the maximum missiles) */
#define EQ_BAD     3
extern const char *eq_names[NEQUIP];
int      equip_count(void);              /* how many items this system offers (according to the tech level) */
unsigned equip_price(int item);          /* tenths of a credit; fuel depends on the tank */
int      equip_is_laser(int item);
int      equip_buy(int item, int view);  /* view 0..3 (front, rear, left, right) for lasers only */

/* ---- commander status ---- */
const char *rating_name(void);           /* according to TALLY (STATUS: st4/st3) */
const char *legal_name(void);            /* FIST: Clean / Offender / Fugitive */
const char *laser_name(u8 v);            /* laser name according to its value */

/* ---- hyperspace ---- */
unsigned jump_distance(int idx);         /* tenths of a light year from the current system */
#ifdef DEBUG
extern int dbg_free;                     /* DEBUG: buying spends no credits and selling removes no cargo */
extern int dbg_range;                    /* DEBUG: the next jump ignores the fuel limit */
#endif
int  jump_possible(int idx);             /* 1 if idx != current and there is fuel */
int  jump_do(int idx);                   /* spends fuel and arrives; 0 if it was not possible */
int  jump_galactic(void);                /* needs a galactic hyperdrive; moves to the next galaxy */
#endif
