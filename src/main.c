/* Elite QL: punto de entrada. */
#include "platform.h"
#include "text.h"
#include "input.h"
#include "sound.h"
#include "app.h"

int main(void)
{
    plat_init();
    txt_init();
    in_init();
    snd_init();
    game_run();
    snd_off();
    plat_shutdown();
    return 0;
}
