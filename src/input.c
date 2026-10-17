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
        key_scan();                                       /* con guion tambien se lee la matriz (pruebas: host_matrix) para los controles de vuelo */
        ch = in_script();
        if (ch == KC_QUIT) in_quit = 1;
        return ch;
    }
    key_scan();
    plat_present();                                   /* QL: vacia la cola de teclado de la consola */
    snd_tick(20);                                     /* ~50 fotogramas por segundo: provisional */
    if (key_down(K_SHIFT) && key_down(K_ESC)) { in_quit = 1; return KC_QUIT; }
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
