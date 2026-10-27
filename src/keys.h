/* QL keyboard and joystick by matrix (IPC, command 9 = KEYROW). C89.
   A key is identified by KEY(row, bit). Whole rows are read, so simultaneous keys are detected.
   Warnings: (1) three keys at three corners of a matrix rectangle generate a fourth ghost key;
   (2) KEYROW reads the physical keyboard directly, no matter which window has the focus. */
#ifndef KEYS_H
#define KEYS_H
#include "platform.h"

#define KEY(r, b) ((u8)(((r) << 3) | (b)))
#define KEY_ROW(k) ((k) >> 3)
#define KEY_BIT(k) ((k) & 7)

/* Row 0 */ enum {
 K_F4 = KEY(0,0), K_F1 = KEY(0,1), K_5 = KEY(0,2), K_F2 = KEY(0,3), K_F3 = KEY(0,4), K_F5 = KEY(0,5), K_4 = KEY(0,6), K_7 = KEY(0,7),
/* Row 1 */
 K_ENTER = KEY(1,0), K_LEFT = KEY(1,1), K_UP = KEY(1,2), K_ESC = KEY(1,3), K_RIGHT = KEY(1,4), K_BACKSLASH = KEY(1,5), K_SPACE = KEY(1,6), K_DOWN = KEY(1,7),
/* Row 2 */
 K_RBRACKET = KEY(2,0), K_Z = KEY(2,1), K_PERIOD = KEY(2,2), K_C = KEY(2,3), K_B = KEY(2,4), K_POUND = KEY(2,5), K_M = KEY(2,6), K_QUOTE = KEY(2,7),
/* Row 3 */
 K_LBRACKET = KEY(3,0), K_CAPS = KEY(3,1), K_K = KEY(3,2), K_S = KEY(3,3), K_F = KEY(3,4), K_EQUALS = KEY(3,5), K_G = KEY(3,6), K_SEMI = KEY(3,7),
/* Row 4 */
 K_L = KEY(4,0), K_3 = KEY(4,1), K_H = KEY(4,2), K_1 = KEY(4,3), K_A = KEY(4,4), K_P = KEY(4,5), K_D = KEY(4,6), K_J = KEY(4,7),
/* Row 5 */
 K_9 = KEY(5,0), K_W = KEY(5,1), K_I = KEY(5,2), K_TAB = KEY(5,3), K_R = KEY(5,4), K_MINUS = KEY(5,5), K_Y = KEY(5,6), K_O = KEY(5,7),
/* Row 6 */
 K_8 = KEY(6,0), K_2 = KEY(6,1), K_6 = KEY(6,2), K_Q = KEY(6,3), K_E = KEY(6,4), K_0 = KEY(6,5), K_T = KEY(6,6), K_U = KEY(6,7),
/* Row 7 */
 K_SHIFT = KEY(7,0), K_CTRL = KEY(7,1), K_ALT = KEY(7,2), K_X = KEY(7,3), K_V = KEY(7,4), K_SLASH = KEY(7,5), K_N = KEY(7,6), K_COMMA = KEY(7,7)
};

/* Backend: returns the row byte (bit set = key pressed). */
int ql_read_row(int row);

void key_init(void);
void key_set_rows(u8 mask);        /* rows to read on each key_scan (bit n = row n); each row costs one IPC call */
void key_set_scan(u8 fast, u8 slow);   /* fast: rows of each scan; slow: rows read one at a time, rotating */
void key_ctx_menu(void);           /* menus: rows 1 and 7 always, the rest rotating (3 reads per scan) */
void key_ctx_flight(void);         /* flight: rows 1, 3 and 4 always; 0, 2, 5, 6 and 7 rotating (4 reads per scan) */
void key_scan(void);               /* reads the rows, computes edges and fills the character queue */
int  key_down(u8 k);               /* key held */
int  key_pressed(u8 k);            /* became pressed in this scan */
int  key_released(u8 k);           /* became released in this scan */
int  key_raw(int row);             /* last byte read from a row (for debugging) */
int  key_any(void);                /* any key (except SHIFT/CTRL/ALT) pressed */
int  key_getchar(void);            /* next ASCII character (UK layout), or -1. CTRL+Left = 8 (delete) */

/* Joystick: on the QL they are keys. CTL1 = cursors + space (row 1); CTL2 = F1..F5 (row 0). */
#define JOY_UP    1
#define JOY_DOWN  2
#define JOY_LEFT  4
#define JOY_RIGHT 8
#define JOY_FIRE  16
int joy_read(int port);            /* port 1 or 2; uses the rows already read by key_scan */

/* Elite flight controls: cursors = roll and pitch (and CTL1 joystick), F accelerate, D brake, A or space laser, the rest as in the original */
struct controls {
    u8 slow_down, speed_up, roll_left, roll_right, pitch_up, pitch_down, fire;      /* primary */
    u8 bomb, escape_pod, target, unarm, missile, ecm, jump, dock, undock;            /* secondary */
};
extern u8 jstgy;                   /* JSTGY: $FF = joystick Y axis normal (default, like the IB disk); 0 = inverted */
extern u8 jste;                    /* JSTE: $FF = both joystick axes inverted */
extern int ctl_joystick;           /* 0 = keyboard only, 1 or 2 = also the joystick of that port */
void ctl_read(struct controls *c);
#endif
