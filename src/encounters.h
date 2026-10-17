/* Partes 1 a 4 del bucle principal: aparicion de naves cada 256 vueltas (asteroides, cargas, ermitanos, comerciantes,
   policia, Thargoides, piratas y cazarrecompensas, y el Constrictor de la mision 1). */
#ifndef ENCOUNTERS_H
#define ENCOUNTERS_H
#include "platform.h"
extern u8 enc_ev;                  /* EV */
extern u8 enc_c;                   /* acarreo de salida */
u8   there(void);                  /* THERE: 1 si estamos en el sistema del Constrictor */
void encounters_tick(u8 c);        /* partes 2-4 (y 1, MTT4) con el acarreo c de entrada */
#endif
