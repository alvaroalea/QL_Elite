#include "config.h"
#include "space.h"
#include "sound.h"
#include "keys.h"
#include "input.h"

u8 patg = 0, jstk = 0, uptog = 0, disk_opt = 0;      /* JSTGY y JSTE estan en keys.c (los usa ctl_read) */
const char tgint[9] = { 'D', 'A', 'X', 'F', 'Y', 'J', 'K', 'U', 'T' };

u8 *config_opt(u8 y)
{
    switch (y) {
    case 0: return &damp;
    case 1: return &djd;
    case 2: return &patg;
    case 3: return &flh;
    case 4: return &jstgy;
    case 5: return &jste;
    case 6: return &jstk;
    case 7: return &uptog;
    }
    return &disk_opt;
}

static void delay(int n) { int i; if (in_script) return; for (i = 0; i < n; i++) in_poll(); }

void config_apply(void)
{
    if ((jstk & 0x80) && ctl_joystick == 0) ctl_joystick = 1;
    else if (!(jstk & 0x80)) ctl_joystick = 0;
}

void dks3(u8 x, u8 y)
{
    u8 *p;
    if (x != (u8)tgint[y]) return;               /* CMP TGINT,Y; BNE Dk3 */
    p = config_opt(y);
    *p = (u8)(*p ^ 0xFF);
    if (*p & 0x80) snd_bell();                   /* encendida: dos pitidos */
    snd_bell();
    delay(20);                                   /* DELAY 20 */
    config_apply();
}

/* FREEZE (DK4 con '='): bucle de pausa; cada vuelta, WSCAN y RDKEY */
int freeze(void)
{
    int ch;
    u8 x, y;
    for (;;) {
        ch = in_poll();                          /* WSCAN y RDKEY */
        if (in_quit) return 0;
        if (ch < 0) continue;
        x = (u8)((ch >= 'a' && ch <= 'z') ? ch - 32 : ch);
        if (x == 'Q') snd_enable(0);             /* DNOIZ = $FF */
        for (y = 0; y < 9; y++) dks3(x, y);
        if (x == 'S') snd_enable(1);             /* DNOIZ = 0 */
        if (x == 27) return 1;                   /* ESC: DEATH2 */
        if (x == '-') return 0;
    }
}
