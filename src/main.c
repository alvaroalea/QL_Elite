/* Elite QL: punto de entrada. */
#include "platform.h"
#include "text.h"
#include "input.h"
#include "sound.h"
#include "app.h"
#include "dash.h"

int main(void)
{
    plat_init();
    txt_init();
    in_init();
    snd_init();
    dash_load();                                      /* lo primero: el panel se lee del disco una sola vez (sin mensajes) y se queda en memoria */
    game_run();
    snd_off();
    plat_shutdown();
    plat_exit();                                      /* QL: sin "Press a key to exit" */
    return 0;
}
