/* Demo: dibuja varias naves con LL9 (proyeccion, caras ocultas y recorte) usando XOR sobre la pantalla.
   En el PC dibuja un cuadro y termina; en el QL espera una tecla. Sin coma flotante: tabla de senos entera. */
#include "shipview.h"
#include "text.h"

static u8 heaps[6][256];

int main(void)
{
    static const u8 types[6] = { SHIPT_COBRA_MK_3, SHIPT_PYTHON, SHIPT_VIPER, SHIPT_CORIOLIS, SHIPT_ASP_MK_2, SHIPT_THARGOID };
    static const int px[6] = { -3, 0, 3, -4, 0, 4 };
    static const int py[6] = { -1, -1, -1, 1, 1, 1 };
    static const int pz[6] = { 7, 12, 6, 24, 7, 14 };
    int i;
    plat_init();
    gfx_clear(C_BLACK);
    state_clear();
    for (i = 0; i < 6; i++) {
        shipview_draw(types[i], px[i], py[i], pz[i], 6 + 3 * i, 5 + i, heaps[i], state_slot(i), 1);
    }
    gfx_rect(0, 136, 256, 1, C_RED);
    txt_init();
    txt_at(0, 18); txt_puts("LL9: 6 NAVES, XOR");
    plat_present();
    plat_wait_key();                              /* QL: sin esto el runtime borra la pantalla al salir */
    plat_shutdown();
    plat_exit();                                  /* QL: sin "Press a key to exit" */
    return 0;
}
