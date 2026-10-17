#include "app.h"
#include "menu.h"
#include "flight.h"
#include "game.h"
#include "input.h"
#include "state.h"
#include "emath.h"

/* Bucle principal. El atracado es una maquina de pantallas (cada pantalla devuelve la siguiente); el vuelo es flight_run. */
int game_run(void)
{
    int t, sc, r;
    unsigned long f;

title:
    t = scr_title();
    if (t == TITLE_QUIT || in_quit) return 0;
    f = in_frames();                                  /* semilla del generador: tiempo hasta la primera pulsacion */
    em_rand[0] = (u8)(f); em_rand[1] = (u8)(f >> 8); em_rand[2] = (u8)(f ^ 0x5A); em_rand[3] = (u8)(0xA7 + (f >> 4));
    if (t == TITLE_NEW) game_new();
    sc = SC_STATUS;
    for (;;) {
        if (in_quit || sc == SC_QUIT) break;
        if (sc == SC_LAUNCH) {
            qq12 = 0;
            r = flight_run();
            if (r == FL_QUIT) break;
            if (r == FL_DEATH) goto title;            /* DEATH2: vuelta al titulo (nuevo comandante o cargar) */
            qq12 = 0xFF;
            sc = SC_STATUS;
            continue;
        }
        sc = scr_dispatch(sc);
    }
    return 0;
}
