/* Text screens of the game (docked): status, inventory, market, buy/sell, equipment, system data,
   charts and disk menu. Each screen is drawn, handles its keys and returns the next screen (SC_*). */
#ifndef MENU_H
#define MENU_H
#include "platform.h"

#define SC_STATUS    0
#define SC_BUY       1
#define SC_SELL      2
#define SC_EQUIP     3
#define SC_GALCHART  4
#define SC_LOCCHART  5
#define SC_DATA      6
#define SC_MARKET    7
#define SC_INVENTORY 8
#define SC_DISK      9
#define SC_LAUNCH    10
#define SC_QUIT      11
#define SC_FLIGHT    12                  /* in flight: back to the current space view */
#define SC_VIEW0     13                  /* in flight: front, rear, left, right view (keys 1 to 4) */
#define SC_VIEW1     14
#define SC_VIEW2     15
#define SC_VIEW3     16
#define SC_HYPER     17                  /* in flight: H, hyperspace jump */
#define SC_DBG_BUY   19                  /* DEBUG: SHIFT-1, SHIFT-2, SHIFT-3 */
#define SC_DBG_SELL  20
#define SC_DBG_EQUIP  21
#define SC_GHYPER    18                  /* in flight: G, galactic jump */

#define TITLE_NEW  1
#define TITLE_LOAD 2
#define TITLE_QUIT 0

int scr_title(void);             /* title screen: TITLE_NEW / TITLE_LOAD / TITLE_QUIT (loading is done inside) */
int scr_status(void);
int scr_inventory(void);
int scr_market(void);
int scr_buy(void);
int scr_sell(void);
int scr_equip(void);
int scr_data(void);
int scr_chart(int local);        /* galactic (0) or local (1) chart; returns the next screen */
int scr_disk(void);              /* disk menu: returns SC_STATUS, or SC_QUIT */
int scr_dispatch(int sc);        /* runs screen sc */

/* shared drawing utilities (also used by flight) */
void ui_clear(void);
void ui_title(const char *t);                    /* title bar on row 0 */
void ui_footer(void);                            /* key help on rows 30 and 31 */
void ui_num(unsigned long v, int width);         /* right-aligned integer */
int  dec_digits(unsigned long v, char *b);              /* decimal digits of v in b (no leading zeros, no trailing 0); returns how many */
void ui_tenths(unsigned long v);                 /* tenths -> "123.4" */
void ui_msg(const char *s, int error);           /* message on row 28 with sound */
int  ui_nav(int ch);                             /* screen key ('0'..'9','i') -> SC_*, or -1 */
#endif
