/* Game input: menu keys (characters and cursors) with a hook for script-driven tests. */
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
#define KC_QUIT  260                 /* SHIFT+ESC: emergency exit (sets in_quit) */

extern int in_quit;                  /* !=0 after SHIFT+ESC: the screens end and the program exits */
extern int (*in_script)(void);       /* tests: if not NULL, returns the next key (or -1) and the keyboard is not touched */

void in_init(void);
int  in_poll(void);                  /* pending key or -1; each call advances a frame (keyboard, console queue, sound) */
int  in_wait(void);                  /* waits for a key */
int  in_held(int code);              /* KC_UP/DOWN/LEFT/RIGHT held (0 with a script) */
int  in_up(int ch);                  /* capitals -> lower case (letters only) */
unsigned long in_frames(void);       /* frames since in_init (seed for the random generator) */
#endif
