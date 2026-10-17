#include "platform.h"
#include "text.h"
#include "input.h"
#include "keys.h"
#include "sound.h"
#include "menu.h"
#include "game.h"
#include "app.h"
#include "state.h"
#include "save.h"
typedef unsigned short u16s;
static u16s hv[64];
static int nh = 0;
static void snap(void)
{
    long j; u16s h = 0;
    for (j = 0; j < 32768L; j++) h = (u16s)(((h << 5) | (h >> 11)) ^ screen[j]);
    if (nh < 60) hv[nh++] = h;
}
#define SNAP (-100)
static const int script[] = {
    SNAP, '1', -1, -1, -1, SNAP,
    '7', SNAP, '1', KC_DOWN, KC_DOWN, '5', KC_ENTER, SNAP, 'm', KC_ESC,
    '2', KC_UP, 'm', SNAP, KC_ESC,
    '3', KC_DOWN, KC_ENTER, SNAP, KC_ESC,
    '9', SNAP, '4', KC_RIGHT, KC_RIGHT, KC_DOWN, SNAP,
    '5', KC_LEFT, KC_UP, SNAP, '6', SNAP, '8', SNAP,
    '5', KC_RIGHT, KC_RIGHT, KC_RIGHT, KC_RIGHT, KC_DOWN, SNAP,
    'u', -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, SNAP, 'h', -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    SNAP, '2', '3', -1, -1, SNAP, 'c', '8', SNAP, 'i', SNAP, KC_ESC, '7', SNAP, 'i', 'q', 'y'
};
static unsigned sp = 0;
static int feed(void)
{
    int v;
    for (;;) {
        if (sp >= sizeof script / sizeof script[0]) return KC_QUIT;
        v = script[sp++];
        if (v == SNAP) { snap(); continue; }
        return v;
    }
}
int main(void)
{
    int i;
    plat_init(); txt_init(); in_init(); snd_init();
    save_set_device("flp1_");                      /* igual en el PC y en c68 (el valor por defecto de save.c depende del compilador) */
    in_script = feed;
    game_run();
    snap();
    gfx_clear(C_BLACK);
    for (i = 0; i < nh; i++) { screen[2 * i] = (u8)hv[i]; screen[2 * i + 1] = (u8)(hv[i] >> 8); }
    screen[200] = (u8)nh;
    plat_present();
    return 0;
}
