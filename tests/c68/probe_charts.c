/* Cartas (TT22/TT23 con SUN y CIRCLE) desde 24 sistemas distintos y con distinto combustible: hash de la pantalla. */
#include "platform.h"
#include "text.h"
#include "chart.h"
#include "sun.h"
#include "game.h"
#include "state.h"
#include "emath.h"
typedef unsigned short u16s;
static u16s hv[64];
int main(void)
{
    int k, n = 0, i;
    long j;
    u16s h;
    plat_init(); txt_init();
    game_new();
    em_rand[0] = 5; em_rand[1] = 6; em_rand[2] = 7; em_rand[3] = 8;
    for (k = 0; k < 24; k++) {
        game_enter_system((k * 37 + 11) & 255, 1);
        qq14 = (u8)(70 - (k * 3) % 60);
        game_set_target((k * 53 + 5) & 255);
        for (i = 0; i < 2; i++) {
            gfx_clear(C_BLACK);
            sun_flflls();
            circ_lsp = 1;
            chart_draw(i);
            chart_cursor(i);
            h = 0;
            for (j = 0; j < 32768L; j++) h = (u16s)(((h << 5) | (h >> 11)) ^ screen[j]);
            hv[n++] = h;
        }
    }
    gfx_clear(C_BLACK);
    for (i = 0; i < n; i++) { screen[2 * i] = (u8)hv[i]; screen[2 * i + 1] = (u8)(hv[i] >> 8); }
    screen[200] = (u8)n;
    plat_present();
    return 0;
}
