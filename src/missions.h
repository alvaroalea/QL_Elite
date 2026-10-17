/* Misiones del original: mision 1 (el Constrictor robado) y mision 2 (los planos de los Thargoides). Las pantallas de
   las misiones son fichas extendidas del original impresas con el motor de detok.c. */
#ifndef MISSIONS_H
#define MISSIONS_H
#include "platform.h"
#define MIS_NONE 0
#define MIS_BRIEF 1
#define MIS_DEBRIEF 2
#define MIS_BRIEF2 3
#define MIS_BRIEF3 4
#define MIS_DEBRIEF2 5
extern void (*mis_ll9_hook)(void);     /* LL9 y MVEIT del Constrictor de BRIEF (las pruebas los sustituyen) */
extern void (*mis_mveit_hook)(void);
int  missions_choose(void);        /* la parte de DOENTRY que decide la sesion informativa (MIS_*) */
void missions_hooks(void);         /* conecta el motor de fichas con la pantalla, el teclado y los nombres */
int  missions_doentry(void);       /* DOENTRY tras atracar: RES2 y la sesion informativa que toque; devuelve SC_STATUS (BAY) */
int  brief(void);                  /* BRIEF: mision 1, con el Constrictor girando */
int  brief2(void);                 /* BRIEF2: mision 2, ir a Ceerdi */
int  brief3(void);                 /* BRIEF3: en Ceerdi, los planos */
int  debrief(void);                /* DEBRIEF: Constrictor destruido, 5.000 CR */
int  debrief2(void);               /* DEBRIEF2: planos entregados en Birera, unidad de energia */
void bris(void);                   /* BRIS */
void pas1(void);                   /* PAS1 */
void mis_pause(void);              /* PAUSE */
void mis_tt66(u8 view);            /* TT66 de las pantallas de las misiones */
int  pdesc(u8 zz);                 /* PDESC: descripcion especial del sistema zz (1 si la imprime) */
void missions_set_ink(u8 c);       /* color de las letras del motor de fichas */
#endif
