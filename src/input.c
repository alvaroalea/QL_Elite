#include "input.h"
#include "keys.h"
#include "sound.h"

int in_quit = 0;
int (*in_script)(void) = 0;
static unsigned long frames = 0;

void in_init(void)
{
    key_init();
    key_ctx_menu();
    in_quit = 0;
    frames = 0;
}

unsigned long in_frames(void) { return frames; }

int in_up(int ch) { return (ch >= 'A' && ch <= 'Z') ? ch - 'A' + 'a' : ch; }

int in_held(int code)
{
    if (in_script) return 0;
    if (code == KC_UP) return key_down(K_UP);
    if (code == KC_DOWN) return key_down(K_DOWN);
    if (code == KC_LEFT) return key_down(K_LEFT);
    if (code == KC_RIGHT) return key_down(K_RIGHT);
    return 0;
}

int in_poll(void)
{
    int ch;
    frames++;
    if (in_script) {
        key_scan();                                       /* with a script the matrix is also read (tests: host_matrix) for the flight controls */
        ch = in_script();
        if (ch == KC_QUIT) in_quit = 1;
        return ch;
    }
    key_scan();
    plat_present();                                   /* QL: empties the console keyboard queue */
    snd_tick(20);                                     /* ~50 frames per second: provisional */
    ch = key_getchar();
    if (ch >= 0) return ch;
    if (key_pressed(K_UP)) return KC_UP;
    if (key_pressed(K_DOWN)) return KC_DOWN;
    if (key_pressed(K_LEFT)) return KC_LEFT;
    if (key_pressed(K_RIGHT)) return KC_RIGHT;
    return -1;
}

int in_wait(void)
{
    int ch;
    for (;;) {
        ch = in_poll();
        if (ch >= 0) return ch;
    }
}
