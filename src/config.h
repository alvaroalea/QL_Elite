/* Configuracion del original (teclas con el juego en pausa): TGINT, DKS3 y FREEZE (la pausa de DK4). */
#ifndef CONFIG_H
#define CONFIG_H
#include "platform.h"
#include "keys.h"

extern u8 patg;                    /* PATG: $FF = autores en el titulo y salto fallido a mano (TAB en el hiperespacio) */
extern u8 jstk;                    /* JSTK: 0 = teclado, $FF = joystick (en el QL: tambien el joystick CTL1) */
extern u8 uptog;                   /* UPTOG: $FF = textos de las misiones solo en mayusculas */
extern u8 disk_opt;                /* DISK: cinta o disco (solo se guarda; el menu de disco es propio) */
extern const char tgint[9];        /* TGINT: teclas de configuracion, en el orden de DAMP..DISK */

u8   *config_opt(u8 y);            /* la variable y (0..8): DAMP, DJD, PATG, FLH, JSTGY, JSTE, JSTK, UPTOG, DISK */
void dks3(u8 x, u8 y);             /* DKS3: si la tecla x es la de la opcion y, la invierte (EOR #$FF) con pitidos */
int  freeze(void);                 /* FREEZE: pausa hasta '-'; Q/S sonido, teclas de TGINT; ESC devuelve 1 (DEATH2) */
void config_apply(void);           /* lleva JSTK al joystick del QL */
#endif
