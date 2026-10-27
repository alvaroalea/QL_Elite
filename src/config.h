/* Configuration of the original (keys with the game paused): TGINT, DKS3 and FREEZE (the DK4 pause). */
#ifndef CONFIG_H
#define CONFIG_H
#include "platform.h"
#include "keys.h"

extern u8 patg;                    /* PATG: $FF = authors on the title and manual failed jump (TAB in hyperspace) */
extern u8 jstk;                    /* JSTK: 0 = keyboard, $FF = joystick (on the QL: also the CTL1 joystick) */
extern u8 uptog;                   /* UPTOG: $FF = mission texts in capitals only */
extern u8 disk_opt;                /* DISK: tape or disk (only saved; the disk menu is our own) */
extern const char tgint[9];        /* TGINT: configuration keys, in the order DAMP..DISK */

extern int (*config_options_hook)(void);   /* own: pause "Options" window (O key); 1 = ESC, 0 = '-' */
extern u8 config_options_used;             /* set to 1 on opening: the caller repaints what it covered */

u8   *config_opt(u8 y);            /* variable y (0..8): DAMP, DJD, PATG, FLH, JSTGY, JSTE, JSTK, UPTOG, DISK */
void dks3(u8 x, u8 y);             /* DKS3: if key x is the one for option y, it inverts it (EOR #$FF) with beeps */
int  freeze(void);                 /* FREEZE: pause until '-'; Q/S sound, TGINT keys; ESC returns 1 (DEATH2) */
void config_apply(void);           /* takes JSTK to the QL joystick */
#endif
