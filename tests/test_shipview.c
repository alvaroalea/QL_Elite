/* Redrawing ships with LL9 (XOR): animating a ship must leave the screen the same as drawing it from scratch. */
#include <stdio.h>
#include <string.h>
#include "shipview.h"
#include "state.h"

static u8 heap[SHIPVIEW_HEAP];
static u8 snap[SCR_BYTES];
static int fails, total;

int main(void)
{
    static const u8 types[6] = { SHIPT_COBRA_MK_3, SHIPT_PYTHON, SHIPT_VIPER, SHIPT_CORIOLIS, SHIPT_ASP_MK_2, SHIPT_THARGOID };
    int t, k, ay, bad;
    long i;
    plat_init();
    for (t = 0; t < 6; t++) {
        for (bad = 0, k = 0; k < 3; k++) {
            int steps = 5 + k * 20;
            gfx_clear(C_BLACK); state_clear();
            shipview_draw(types[t], 0, 0, 4 + t, 0, 4, heap, state_slot(0), 1);
            for (ay = 1; ay <= steps; ay++)
                shipview_draw(types[t], 0, 0, 4 + t, ay & 63, 4, heap, state_slot(0), 0);
            memcpy(snap, screen, SCR_BYTES);
            gfx_clear(C_BLACK); state_clear();
            shipview_draw(types[t], 0, 0, 4 + t, steps & 63, 4, heap, state_slot(0), 1);
            for (i = 0; i < SCR_BYTES; i++) if (snap[i] != screen[i]) { bad = 1; break; }
        }
        total++;
        if (bad) { fails++; printf("FALLO: tipo %d: la animacion deja restos\n", types[t]); }
    }
    printf("%d naves, %d fallos\n", total, fails);
    return fails ? 1 : 0;
}
