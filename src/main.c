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
    txt_color(C_WHITE, C_BLACK);
    txt_print_at(0, 0, "LOADING ELITE_DASH...");         /* lo primero: el panel se lee del disco una sola vez y se queda en memoria */
    plat_present();
    dash_load();
    game_run();
    snd_off();
    plat_shutdown();
    plat_exit();                                      /* QL: sin "Press a key to exit" */
    return 0;
}
