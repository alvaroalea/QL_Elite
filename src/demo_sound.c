/* Demo de sonido: cada tecla lanza un efecto de Elite. SHIFT+ESC sale. Sirve para ajustar de oido los parametros
   (SND_NOISE_FUZZ/SND_NOISE_RND en sound.c) en un QL real o emulador. */
#include "text.h"
#include "keys.h"
#include "sound.h"

static const char *names[] = {
    "1 beep (corto y agudo)", "2 boop (largo y grave)", "3 campana", "4 laser propio",
    "5 golpe de laser (exno)", "6 colision / misil (exno3)", "7 nave destruida (exno2)", "8 muerte",
    "9 ECM", "0 hiperespacio", "Q siseo de lanzamiento", "W bomba de energia", "E silenciar"
};

static void pause_frame(void) { volatile long i; for (i = 0; i < 4000L; i++) { } }

int main(void)
{
    int i, ch;
    plat_init(); txt_init(); key_init(); snd_init();
    key_set_rows(0xFF);
    gfx_clear(C_BLACK);
    txt_color(C_YELLOW, C_BLUE);
    txt_print_at(0, 0, " PRUEBA DE SONIDO (SHIFT+ESC) ");
    txt_color(C_WHITE, C_BLACK);
    for (i = 0; i < 13; i++) txt_print_at(1, 2 + i, names[i]);
    for (;;) {
        key_scan();
        while ((ch = key_getchar()) >= 0) {
            switch (ch) {
            case '1': snd_beep(); break;
            case '2': snd_boop(); break;
            case '3': snd_bell(); break;
            case '4': snd_lasnoise(); break;
            case '5': snd_exno(); break;
            case '6': snd_exno3(); break;
            case '7': snd_exno2(); break;
            case '8': snd_death(); break;
            case '9': snd_soblip(224, 20); break;
            case '0': snd_soblip(255, 90); break;
            case 'q': snd_sohiss(120); break;
            case 'w': snd_sobomb(); break;
            case 'e': snd_off(); break;
            default: break;
            }
        }
        snd_tick(20);
        plat_present();
        pause_frame();
        if (key_down(K_SHIFT) && key_down(K_ESC)) break;
#ifdef HOST_DEMO
        break;
#endif
    }
    snd_off();
    plat_shutdown();
    return 0;
}
