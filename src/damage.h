/* Danos a nuestra nave (paso 3, segunda parte): OOPS (escudos y energia), OUCH/ou2/ou3 (perdida de carga y equipo),
   colisiones, recogida de cargas y atraque por la ranura (bucle de vuelo partes 9 y 10: MA58..MA63, ISDK, tnpr1),
   comprobaciones periodicas de la parte 15 (ENERGY LOW, altitud, temperatura de cabina junto al sol, recogida de
   combustible) y la muerte (DEATH, D1/D2 con Ze y fq1). Transcripcion del 6502; verificacion en docs/PENDIENTES.md. */
#ifndef DAMAGE_H
#define DAMAGE_H
#include "platform.h"

/* MESS: tokens de los mensajes en vuelo que usa el original (flight.c los convierte en texto) */
#define MESS_ENERGY_LOW 100              /* "ENERGY LOW" + pitido */
#define MESS_ECM        108              /* "E.C.M.SYSTEM" */
#define MESS_FUEL_SCOOPS 111             /* "FUEL SCOOPS" */
#define MESS_BOMB       113              /* "ENERGY BOMB" (114 "ENERGY UNIT", 115 "DOCKING COMPUTERS") */
#define MESS_DOCK_ON    123              /* "DOCKING COMPUTERS ON" */
#define MESS_SCOOP_ON   160              /* "FUEL SCOOPS ON" + pitido */
#define MESS_GOOD       208              /* 208 + mercancia (0..16): nombre de la mercancia */

#define COL_NONE  0                      /* combat_collide: nada (o recogida, o choque sin muerte) */
#define COL_DOCK  1                      /* entrada correcta por la ranura: DOENTRY */
#define COL_DEATH 2                      /* DEATH */

extern u8 mess_de;                       /* "de": bit 1 = anadir " DESTROYED" al mensaje (lo lee el gancho de MESS) */
extern u8 player_dead;                   /* 1 = el original saltaria a DEATH */

u8   oops(u8 a, u8 c);                   /* OOPS: dano a (con el acarreo c de entrada del SBC); 1 si morimos */
void ouch(void);                         /* OUCH: con los escudos agotados, quiza se pierde carga o equipo */
u8  *ouch_item(u8 x);                    /* QQ20+X (0..21): mercancias 0..16, ECM, BST, BOMB, ENGY, DKCMP */
int  combat_collide(void);               /* partes 9 y 10 para la nave de INWK (tras MVEIT y MAL3): COL_* */
u8   damage_checks(u8 mc);               /* parte 15 (MA22/MA93): mensajes, altitud, cabina, combustible; 1 = muerte */
#endif
