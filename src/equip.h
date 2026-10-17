/* Equipo en vuelo: bomba de energia (BOMBON, BOMBOFF, BOMBEFF, BOMBEFF2, BOMBPOS, BOMBTBX, BOMBTBY y los tramos de las
   partes 3 y 16 del bucle de vuelo) y salto dentro del sistema (WARP). Transcripcion del 6502 (ver PENDIENTES.md). */
#ifndef EQUIP_H
#define EQUIP_H
#include "platform.h"

extern u8 bombtbx[10], bombtby[10];      /* BOMBTBX, BOMBTBY: vertices del rayo */
extern u8 bomb_col;                      /* color del rayo (el original usa el COL que haya) */
extern void (*bomb_line_hook)(u8 x1, u8 y1, u8 x2, u8 y2);   /* pruebas: cada LOIN del rayo */

void bomb_off(void);                     /* BOMBOFF: dibuja o borra (XOR) el rayo */
void bomb_on(u8 c);                      /* BOMBON: rayo nuevo (c = acarreo de entrada del primer DORND) */
void bomb_eff(void);                     /* BOMBEFF */
void bomb_eff2(void);                    /* BOMBEFF2: cuatro BOMBEFF */
void bomb_key(void);                     /* parte 3: tecla de la bomba */
void bomb_frame(void);                   /* parte 16: la bomba sigue explotando */
int  warp(void);                         /* WARP: 1 si salta (hay que repintar la vista), 0 si no (BOOP) */
#endif
