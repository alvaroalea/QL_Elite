/* Panel: giros, velocidad, planeta/estacion, instrumentos y brujula con controles pseudoaleatorios: hash de la pantalla. */
#include "platform.h"
#include "text.h"
#include "dash.h"
#include "space.h"
#include "game.h"
#include "state.h"
#include "emath.h"
#include "keys.h"
typedef unsigned short u16s;
static u16s hv[32];
static u16s rs = 4321;
static unsigned rnd(void) { rs = (u16s)(rs * 25173u + 13849u); return (unsigned)(rs >> 4); }
int main(void)
{
    int run, f, n = 0, i;
    long j;
    u16s h;
    struct controls c;
    plat_init(); txt_init();
    for (run = 0; run < 3; run++) {
        game_new();
        gfx_clear(C_BLACK);
        dash_show();
        space_launch();
        for (f = 0; f < 240; f++) {
            c.slow_down = (u8)((rnd() & 7) == 0); c.speed_up = (u8)((rnd() & 3) == 0);
            c.roll_left = (u8)((rnd() & 3) == 0); c.roll_right = (u8)((rnd() & 3) == 1);
            c.pitch_up = (u8)((rnd() & 3) == 0); c.pitch_down = (u8)((rnd() & 3) == 2);
            c.fire = c.bomb = c.escape_pod = c.target = c.unarm = c.missile = c.ecm = c.jump = c.dock = c.undock = 0;
            space_controls(&c);
            space_step();
            if (run == 1) { energy = (u8)(energy - 3); fsh = (u8)(fsh - 5); qq14 = (u8)(70 - (f % 70)); gntmp = (u8)(f * 2); cabtmp = (u8)(f * 3); }
            if (run == 2) flh = 0xFF;
            dash_update();
            if (f % 60 == 59) {
                h = 0;
                for (j = 0; j < 32768L; j++) h = (u16s)(((h << 5) | (h >> 11)) ^ screen[j]);
                hv[n++] = h;
                hv[n++] = (u16s)(jstx | (jsty << 8));
                hv[n++] = (u16s)(alpha | (beta << 8));
                hv[n++] = (u16s)(zDELTA | (altit << 8));
                if (n >= 28) n = 28;
            }
        }
    }
    gfx_clear(C_BLACK);
    for (i = 0; i < n; i++) { screen[2 * i] = (u8)hv[i]; screen[2 * i + 1] = (u8)(hv[i] >> 8); }
    screen[200] = (u8)n;
    plat_present();
    return 0;
}
