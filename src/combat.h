/* Laser propio y danos a otras naves (paso 3, primera parte). Port sin verificar salvo donde se dice (ver docs/PENDIENTES.md):
   disparo del bucle principal (parte 3, MA68), LASLI/LASLI2 (lineas del laser), HITCH (la nave esta en la mira),
   los danos del laser (parte 11, MA47..MA14: EXNO, BURN, SPIN, EXNO2, ANGRY), la recompensa al terminar de explotar
   (parte 12, MA15..KS1S), el fin del pulso (parte 16, MA23), el enfriamiento y el contador del pulso (MLOOP),
   DENGY, la recarga de energia y escudos (parte 16, MA77..paen1, SHD) y el retardo de los mensajes (DLY, MESS). */
#ifndef COMBAT_H
#define COMBAT_H
#include "platform.h"

#define LASER_COL C_WHITE                /* el original no fija color (el "LDA #RED" de LASLI2 esta comentado) */
#define MESS_CASH 0                      /* MESS #0: dinero actual y " CR" */
#define MESS_RIGHT_ON 101                /* MESS #101: "RIGHT ON COMMANDER!" */
#define MESS_DLY 20                      /* fotogramas que dura un mensaje en vuelo */

extern u8 las;                           /* LAS: potencia del laser este fotograma (0 = no dispara) */
extern u8 las2;                          /* LAS2: potencia del ultimo disparo; != 0 = hay lineas del laser en pantalla */
extern u8 lasct;                         /* LASCT: cuenta del pulso (0 = puede disparar; 0 siempre con haz y militar) */
extern u8 lasx, lasy;                    /* LASX, LASY: vertice de las lineas del laser */
extern u8 dly;                           /* DLY: fotogramas que le quedan al mensaje en vuelo (0 = ninguno) */
extern u8 combat_view_on;                /* QQ11 = 0: estamos en una vista del espacio (LASLI2 dibuja) */

extern void (*combat_mess_hook)(u8 token);   /* pinta el mensaje (flight.c); NULL = no se pinta */
extern void (*combat_mess_off_hook)(void);   /* borra el mensaje cuando DLY llega a 0 */

void combat_reset(void);                 /* al despegar tras atracar (DOENTRY/RES2): energia, escudos y laser a punto */
void laser_forget(void);                 /* TTX66: se ha borrado la vista, las lineas del laser ya no estan */
void laser_fire(u8 view, u8 fire);       /* parte 3 (MA68): LAS = 0 y, si se puede, dispara (LASNOISE + LASLI) */
void lasli(void);                        /* LASLI: vertice nuevo, calienta el laser, gasta energia, dibuja */
void lasli2(void);                       /* LASLI2: vuelve a dibujar las mismas lineas (las borra, XOR) */
void laser_frame_end(void);              /* parte 16 (MA23): fin de un pulso */
void laser_cool(void);                   /* MLOOP: GNTMP - 1 y LASCT - 2 (o - 1) */
u8   hitch(void);                        /* HITCH: 1 (acarreo) si la nave de INWK esta en la mira */
void combat_hit(void);                   /* parte 11 tras PLUT: HITCH y danos del laser a la nave de INWK */
void angry(u8 type);                     /* ANGRY: la nave de INF (zinf) se vuelve hostil; la estacion, la de la ranura 1 */
void exno2(u8 type);                     /* EXNO2: suma al TALLY la nave de ese tipo, mensaje si sube el byte alto, sonido */
u8   dengy(void);                        /* DENGY: energia - 1 (nunca 0); devuelve 1 si llego a 0 (bandera Z del DEC) */
u8   shd(u8 x);                          /* SHD: x + 1 sin pasar de 255; si sube, gasta 1 de energia (DENGY) */
void energy_tick(u8 mcnt);               /* parte 16 (MA77): cada 8 fotogramas recarga escudos (energia >= 128) y energia */
int  combat_killed(void);                /* parte 12 (MA15): la nave de INWK termino de explotar o fue recogida: FIST y
                                            recompensa; devuelve 1 si hay que quitarla (KS1) */
void mess(u8 token);                     /* MESS: mensaje en vuelo durante MESS_DLY fotogramas */
void dly_tick(void);                     /* TT100: DLY - 1 y borra el mensaje al llegar a 0 */
#endif
