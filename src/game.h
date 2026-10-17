/* Logica de juego acoplada al estado: sistema actual y objetivo, mercado, comercio, equipo, hiperespacio.
   Sin dibujo ni teclado: se prueba en el PC (tests/test_game.c). Las reglas siguen el Elite de Apple II
   (TT151/TT219/TT208/tnpr/EQSHP/refund); las diferencias estan marcadas con "DIFERENCIA". */
#ifndef GAME_H
#define GAME_H
#include "platform.h"
#include "universe.h"

#define NGOODS 17                /* 16 mercancias + objetos alienigenas */
#define NEQUIP 14                /* lista de EQSHP */

extern int cur_sys;              /* indice del sistema actual en uni_galaxy */
extern int tgt_sys;              /* sistema seleccionado para el hiperespacio */
extern uni_market mkt;           /* precios (decimas de credito) y cantidades de este momento */
extern u8 game_visit_rnd;        /* hook de pruebas: si !=0 se usa en lugar de DORND para qq26 al llegar a un sistema */

/* ---- dinero: CASH es un entero de 32 bits en big-endian (decimas de credito) ---- */
unsigned long cash_get(void);
void cash_set(unsigned long v);
int  cash_spend(unsigned long v);        /* 1 si habia saldo y lo resta; 0 si no (no toca nada) */
void cash_earn(unsigned long v);         /* suma con saturacion a 0xFFFFFFFF */

/* ---- partida ---- */
void game_new(void);                     /* state_clear + comandante por defecto + galaxia 1, sistema de partida (Lave) */
void game_after_load(void);              /* tras cargar un fichero: reconstruye galaxia, sistema actual, mercado (con avl guardado) */
void game_enter_system(int idx, int fresh);   /* pone qq0..qq8 del sistema; fresh = visita nueva: qq26 aleatorio, mercado nuevo */
void game_set_target(int idx);           /* qq9/qq10/qq8 del objetivo */

/* ---- comercio (indice de mercancia 0..16) ---- */
#define TR_OK       0
#define TR_NOCASH   1
#define TR_NOROOM   2
#define TR_NOSTOCK  3
#define TR_NONE     4                    /* cantidad 0 o nada que vender */
/* TRADE_FAITHFUL = 1: aritmetica del 6502 tal cual (verificada contra el original, ver tools/gen_trade.py):
     - tnpr suma en 8 bits con acarreo: en kg/g, 198 + 63 da la vuelta (=5) y el original dice que "cabe";
     - GCASH calcula importe = precio * cantidad * 4 en 16 bits: vender >= 64K decimas de credito de una vez pierde los bits altos.
   TRADE_FAITHFUL = 0: aritmetica corregida (sin vueltas, importe en 32 bits). */
#ifndef TRADE_FAITHFUL
#define TRADE_FAITHFUL 1
#endif
int  trade_fits(int good, unsigned qty); /* 1 si qty de esa mercancia cabe en la bodega (carry limpio de tnpr) */
int  trade_hold_t(void);                 /* toneladas ocupadas (mercancias 0..12) + tribbles */
int  trade_room(int good);               /* cuanto mas cabe de esa mercancia (tnpr) */
unsigned trade_price(int good);          /* decimas de credito por unidad en este sistema */
int  trade_buy(int good, unsigned qty);
int  trade_sell(int good, unsigned qty);
unsigned trade_max_buy(int good);        /* lo que se puede comprar ahora: min(existencias, dinero/precio, espacio) */

/* ---- equipo (indices de PRXS: 0 combustible, 1 misil, 2 bodega grande, 3 ECM, 4 laser de pulsos, ..., 13 laser minero) ---- */
#define EQ_OK      0
#define EQ_NOCASH  1
#define EQ_PRESENT 2                     /* ya lo tienes (o ya llevas el maximo de misiles) */
#define EQ_BAD     3
extern const char *eq_names[NEQUIP];
int      equip_count(void);              /* cuantos articulos ofrece este sistema (segun el nivel tecnologico) */
unsigned equip_price(int item);          /* decimas de credito; el combustible depende del deposito */
int      equip_is_laser(int item);
int      equip_buy(int item, int view);  /* view 0..3 (frontal, trasera, izquierda, derecha) solo para lasers */

/* ---- estado del comandante ---- */
const char *rating_name(void);           /* segun TALLY (STATUS: st4/st3) */
const char *legal_name(void);            /* FIST: Clean / Offender / Fugitive */
const char *laser_name(u8 v);            /* nombre del laser segun su valor */

/* ---- hiperespacio ---- */
unsigned jump_distance(int idx);         /* decimas de anio luz desde el sistema actual */
int  jump_possible(int idx);             /* 1 si idx != actual y hay combustible */
int  jump_do(int idx);                   /* gasta combustible y llega; 0 si no era posible */
int  jump_galactic(void);                /* necesita hiperimpulsor galactico; pasa a la galaxia siguiente */
#endif
