/* Entrada del juego: teclas de menu (caracteres y cursores) con un gancho para pruebas guiadas por guion. */
#ifndef INPUT_H
#define INPUT_H
#include "platform.h"

#define KC_ENTER 13
#define KC_ESC   27
#define KC_BS    8
#define KC_TAB   9
#define KC_UP    256
#define KC_DOWN  257
#define KC_LEFT  258
#define KC_RIGHT 259
#define KC_QUIT  260                 /* SHIFT+ESC: salida de emergencia (pone in_quit) */

extern int in_quit;                  /* !=0 tras SHIFT+ESC: las pantallas terminan y el programa sale */
extern int (*in_script)(void);       /* pruebas: si no es NULL, devuelve la siguiente tecla (o -1) y no se toca el teclado */

void in_init(void);
int  in_poll(void);                  /* tecla pendiente o -1; en cada llamada avanza un fotograma (teclado, cola de la consola, sonido) */
int  in_wait(void);                  /* espera una tecla */
int  in_held(int code);              /* KC_UP/DOWN/LEFT/RIGHT mantenida (0 con guion) */
int  in_up(int ch);                  /* mayusculas -> minusculas (solo letras) */
unsigned long in_frames(void);       /* fotogramas desde in_init (semilla para el generador aleatorio) */
#endif
