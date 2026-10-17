/* Naves en el espacio local: lista de ranuras (K%, FRIN, MANY), creacion (ZINF, NWSHP), bucle por nave del bucle principal reducido
   (MVEIT, PLUT, LL9), explosiones (DOEXP), planeta y sol (PLANET), SOLAR y NWSPS, y KILLSHP. Port sin verificar, ver PENDIENTES.md.
   Cada ranura tiene su monton de lineas (heaps) para dibujar y borrar sus lineas en XOR, y para la nube de la explosion. */
#ifndef SHIPS_H
#define SHIPS_H
#include "platform.h"

#define HEAP_BYTES 256                           /* monton de lineas por nave */
#define SHIPS_EV_STATION 1                       /* ships_step: la estacion se ha perdido de vista (queda el sol) */
#define SHIPS_EV_DOCK 2                          /* ships_step: hemos entrado por la ranura (DOENTRY) */
#define SHIPS_EV_DEATH 4                         /* ships_step: choque mortal (DEATH) */

const u8 *ship_bp_for(u8 type);                  /* plano de la nave (la estacion es un Dodo con nivel tecnologico >= 10) */
void ship_zinf(void);                            /* ZINF: INWK a cero con la orientacion por defecto */
int  ship_new(u8 type);                          /* NWSHP: anade la nave de INWK a la primera ranura libre; -1 si no cabe */
void ship_kill(int slot);                        /* KILLSHP: quita la nave y desplaza las demas */
void ships_clear(void);                          /* ninguna nave */
void ships_launch(void);                         /* planeta (ranura 0) y estacion (ranura 1) al despegar (SOLAR y NWSPS) */
void ships_arrive(void);                         /* planeta y sol (SOLAR) tras el hiperespacio */
int  ships_spawn(u8 type);                       /* contacto de prueba delante de la nave; devuelve la ranura o -1 */
int  ships_explode_nearest(void);                /* prueba: hace explotar la nave mas cercana (no planeta ni estacion); devuelve la ranura o -1 */
u8   ships_step(u8 view);                        /* un fotograma: mueve, dibuja, borra y quita naves; devuelve SHIPS_EV_* */
void ships_hooks(void);                          /* pone DOEXP y PLANET como ganchos de LL9 */
void ships_forget_lines(void);                   /* tras borrar la pantalla entera: ninguna nave figura como dibujada */
void ships_erase_all(void);                      /* borra de la pantalla las lineas de todas las naves (antes de repintar o cambiar de sistema) */
void sfs1(u8 ai, u8 type);                      /* SFS1: nave hija de la de INF (zinf) con IA = ai (NWSHP) */
void spin(u8 type);                              /* SPIN: al 50 %, restos de ese tipo de la nave destruida */
void spin2(u8 cnt, u8 type);                     /* SPIN2: cnt restos de ese tipo */
int  ships_death_debris_one(void);              /* una vuelta de D1 (devuelve la ranura o -1) */
void ships_death_debris(void);                   /* DEATH (D1): restos de nuestra nave hasta llenar la ranura 4 */
int  frs1(u8 type);                              /* FRS1: nave nueva delante de nosotros (devuelve la ranura o -1) */
void gthg(void);                                 /* GTHG: Thargoid y Thargon */
void ships_escape(void);                         /* ESCAPE: nuestra Cobra se aleja (97 fotogramas) */
void sescp(void);                                /* SESCP: la nave actual lanza una capsula */
void doexp(void);                                /* DOEXP: nube de la nave que explota */
void planet(void);                               /* PLANET: dibuja o borra el planeta o el sol de INWK */
#endif
