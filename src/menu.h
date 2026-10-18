/* Pantallas de texto del juego (atracado): estado, inventario, mercado, compra/venta, equipo, datos del sistema,
   cartas y menu de disco. Cada pantalla se dibuja, atiende sus teclas y devuelve la siguiente pantalla (SC_*). */
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
#define SC_FLIGHT    12                  /* en vuelo: volver a la vista espacial actual */
#define SC_VIEW0     13                  /* en vuelo: vista frontal, trasera, izquierda, derecha (teclas 1 a 4) */
#define SC_VIEW1     14
#define SC_VIEW2     15
#define SC_VIEW3     16
#define SC_HYPER     17                  /* en vuelo: H, salto de hiperespacio */
#define SC_DBG_BUY   19                  /* DEBUG: SHIFT-1, SHIFT-2, SHIFT-3 */
#define SC_DBG_SELL  20
#define SC_DBG_EQUIP  21
#define SC_GHYPER    18                  /* en vuelo: G, salto galactico */

#define TITLE_NEW  1
#define TITLE_LOAD 2
#define TITLE_QUIT 0

int scr_title(void);             /* pantalla de titulo: TITLE_NEW / TITLE_LOAD / TITLE_QUIT (la carga se hace dentro) */
int scr_status(void);
int scr_inventory(void);
int scr_market(void);
int scr_buy(void);
int scr_sell(void);
int scr_equip(void);
int scr_data(void);
int scr_chart(int local);        /* carta galactica (0) o local (1); devuelve la siguiente pantalla */
int scr_disk(void);              /* menu de disco: devuelve SC_STATUS, o SC_QUIT */
int scr_dispatch(int sc);        /* ejecuta la pantalla sc */

/* utilidades de dibujo compartidas (tambien las usa el vuelo) */
void ui_clear(void);
void ui_title(const char *t);                    /* barra de titulo en la fila 0 */
void ui_footer(void);                            /* ayuda de teclas en las filas 30 y 31 */
void ui_num(unsigned long v, int width);         /* entero alineado a la derecha */
void ui_tenths(unsigned long v);                 /* decimas -> "123.4" */
void ui_msg(const char *s, int error);           /* mensaje en la fila 28 con sonido */
int  ui_nav(int ch);                             /* tecla de pantalla ('0'..'9','i') -> SC_*, o -1 */
#endif
