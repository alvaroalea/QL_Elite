/* Teclado y joystick del QL por matriz (IPC, comando 9 = KEYROW). C89.
   Una tecla se identifica por KEY(fila, bit). Se leen filas completas, asi que se detectan teclas simultaneas.
   Avisos: (1) tres teclas en tres esquinas de un rectangulo de la matriz generan una cuarta fantasma;
   (2) KEYROW lee el teclado fisico directamente, sin importar que ventana tenga el foco. */
#ifndef KEYS_H
#define KEYS_H
#include "platform.h"

#define KEY(r, b) ((u8)(((r) << 3) | (b)))
#define KEY_ROW(k) ((k) >> 3)
#define KEY_BIT(k) ((k) & 7)

/* Fila 0 */ enum {
 K_F4 = KEY(0,0), K_F1 = KEY(0,1), K_5 = KEY(0,2), K_F2 = KEY(0,3), K_F3 = KEY(0,4), K_F5 = KEY(0,5), K_4 = KEY(0,6), K_7 = KEY(0,7),
/* Fila 1 */
 K_ENTER = KEY(1,0), K_LEFT = KEY(1,1), K_UP = KEY(1,2), K_ESC = KEY(1,3), K_RIGHT = KEY(1,4), K_BACKSLASH = KEY(1,5), K_SPACE = KEY(1,6), K_DOWN = KEY(1,7),
/* Fila 2 */
 K_RBRACKET = KEY(2,0), K_Z = KEY(2,1), K_PERIOD = KEY(2,2), K_C = KEY(2,3), K_B = KEY(2,4), K_POUND = KEY(2,5), K_M = KEY(2,6), K_QUOTE = KEY(2,7),
/* Fila 3 */
 K_LBRACKET = KEY(3,0), K_CAPS = KEY(3,1), K_K = KEY(3,2), K_S = KEY(3,3), K_F = KEY(3,4), K_EQUALS = KEY(3,5), K_G = KEY(3,6), K_SEMI = KEY(3,7),
/* Fila 4 */
 K_L = KEY(4,0), K_3 = KEY(4,1), K_H = KEY(4,2), K_1 = KEY(4,3), K_A = KEY(4,4), K_P = KEY(4,5), K_D = KEY(4,6), K_J = KEY(4,7),
/* Fila 5 */
 K_9 = KEY(5,0), K_W = KEY(5,1), K_I = KEY(5,2), K_TAB = KEY(5,3), K_R = KEY(5,4), K_MINUS = KEY(5,5), K_Y = KEY(5,6), K_O = KEY(5,7),
/* Fila 6 */
 K_8 = KEY(6,0), K_2 = KEY(6,1), K_6 = KEY(6,2), K_Q = KEY(6,3), K_E = KEY(6,4), K_0 = KEY(6,5), K_T = KEY(6,6), K_U = KEY(6,7),
/* Fila 7 */
 K_SHIFT = KEY(7,0), K_CTRL = KEY(7,1), K_ALT = KEY(7,2), K_X = KEY(7,3), K_V = KEY(7,4), K_SLASH = KEY(7,5), K_N = KEY(7,6), K_COMMA = KEY(7,7)
};

/* Backend: devuelve el byte de la fila (bit a 1 = tecla pulsada). */
int ql_read_row(int row);

void key_init(void);
void key_set_rows(u8 mask);        /* filas a leer en cada key_scan (bit n = fila n); cada fila cuesta una llamada al IPC */
void key_scan(void);               /* lee las filas, calcula flancos y llena la cola de caracteres */
int  key_down(u8 k);               /* tecla mantenida */
int  key_pressed(u8 k);            /* paso a pulsada en este scan */
int  key_released(u8 k);           /* paso a soltada en este scan */
int  key_raw(int row);             /* ultimo byte leido de una fila (para depurar) */
int  key_any(void);                /* alguna tecla (excepto SHIFT/CTRL/ALT) pulsada */
int  key_getchar(void);            /* siguiente caracter ASCII (disposicion UK), o -1. CTRL+Izq = 8 (borrar) */

/* Joystick: en el QL son teclas. CTL1 = cursores + espacio (fila 1); CTL2 = F1..F5 (fila 0). */
#define JOY_UP    1
#define JOY_DOWN  2
#define JOY_LEFT  4
#define JOY_RIGHT 8
#define JOY_FIRE  16
int joy_read(int port);            /* port 1 o 2; usa las filas ya leidas por key_scan */

/* Controles de vuelo de Elite (teclas del original Apple II) y joystick opcional */
struct controls {
    u8 slow_down, speed_up, roll_left, roll_right, pitch_up, pitch_down, fire;      /* primarios */
    u8 bomb, escape_pod, target, unarm, missile, ecm, jump, dock, undock;            /* secundarios */
};
extern int ctl_joystick;           /* 0 = solo teclado, 1 o 2 = tambien joystick de ese puerto */
void ctl_read(struct controls *c);
#endif
