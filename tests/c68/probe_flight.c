/* Vuelo: polvo estelar en las 4 vistas, radar con contactos, mira, tunel, misiles y bombillas con controles pseudoaleatorios. */
#include "platform.h"
#include "text.h"
#include "dash.h"
#include "space.h"
#include "stars.h"
#include "sun.h"
#include "chart.h"
#include "game.h"
#include "state.h"
#include "emath.h"
#include "keys.h"
#include "shipdata.h"
typedef unsigned short u16s;
static u16s hv[40];
static u16s rs = 9876;
static unsigned rnd(void) { rs = (u16s)(rs * 25173u + 13849u); return (unsigned)(rs >> 4); }
static int n = 0;
static void hash(void)
{
    long j;
    u16s h = 0;
    for (j = 0; j < 32768L; j++) h = (u16s)(((h << 5) | (h >> 11)) ^ screen[j]);
    if (n < 38) hv[n++] = h;
}
int main(void)
{
    int view, f, i;
    struct controls c;
    plat_init(); txt_init();
    for (view = 0; view < 4; view++) {
        game_new();
        em_rand[0] = 3; em_rand[1] = 5; em_rand[2] = 7; em_rand[3] = (u8)(11 + view);
        gfx_clear(C_BLACK);
        space_launch();
        dash_show(); space_scanner_reset(); dash_msblob(); dash_spblb();
        chart_sight((u8)view);
        stars_new();
        for (i = 0; i < 3; i++) space_spawn((u8)(SHIPT_VIPER + i));
        for (f = 0; f < 200; f++) {
            c.speed_up = (u8)((rnd() & 3) == 0); c.slow_down = (u8)((rnd() & 7) == 0);
            c.roll_left = (u8)((rnd() & 3) == 0); c.roll_right = (u8)((rnd() & 3) == 1);
            c.pitch_up = (u8)((rnd() & 3) == 0); c.pitch_down = (u8)((rnd() & 3) == 2);
            c.fire = c.bomb = c.escape_pod = c.target = c.unarm = c.missile = c.ecm = c.jump = c.dock = c.undock = 0;
            space_controls(&c);
            if (space_step() & SPACE_EV_STATION) dash_spblb();
            stars_set_speed(zDELTA);
            stars_update((u8)view);
            dash_update();
            if (f % 50 == 49) hash();
        }
    }
    gfx_clear(C_BLACK); sun_col = C_WHITE; tunnel_draw(4); hash();
    gfx_clear(C_BLACK); tunnel_draw(8); hash();
    gfx_clear(C_BLACK);
    for (i = 0; i < n; i++) { screen[2 * i] = (u8)hv[i]; screen[2 * i + 1] = (u8)(hv[i] >> 8); }
    screen[200] = (u8)n;
    plat_present();
    return 0;
}
