/* Text under the flight panel: painting a new state over another (only the leftover rectangle is emptied, with gfx_clear_cells)
   leaves the screen the same as painting it from scratch. flight.c is included to reach its internal functions. */
#include <stdio.h>
#include <string.h>
#include "../src/flight.c"

static int fails, total;
static void check(int ok, const char *m) { total++; if (!ok) { fails++; printf("FALLO: %s\n", m); } }
static u8 snap[32768];

static void fresh(void)                                  /* empty screen and everything painted from scratch */
{
    memset(screen, 0, 32768);
    panel_static();
    panel_update();
}

int main(void)
{
    static const unsigned long cashes[] = { 0UL, 5UL, 99UL, 1234567UL, 12UL, 100000000UL, 3UL, 456UL };
    int a, b, k;
    plat_init(); txt_init(); in_init();
    state_clear();
    for (a = 0; a < 8; a++) for (b = 0; b < 8; b++) {
        int sa = (a * 37) % 200, sb = (b * 53) % 200, ta = (a * 11) % 200, tb = (b * 29) % 200;
        view = a & 3;
        cur_sys = sa; tgt_sys = ta; cash_set(cashes[a]);
        fresh();                                         /* state A */
        view = b & 3;
        cur_sys = sb; tgt_sys = tb; cash_set(cashes[b]);
        panel_update();                                  /* A -> B on top */
        memcpy(snap, screen, 32768);
        fresh();                                         /* B from scratch */
        for (k = 0; k < 32768 && snap[k] == screen[k]; k++) ;
        check(k == 32768, "estado nuevo sobre el viejo = estado nuevo desde cero");
    }
    /* collecting a reward only changes the credits line */
    cur_sys = 7; tgt_sys = 9; view = 0; cash_set(1000UL);
    fresh();
    memcpy(snap, screen, 32768);
    cash_set(1000UL + 50000UL);
    panel_cash();
    {
        int rowa = ROW_HERE * 8 * 128, rowb = (ROW_HERE + 1) * 8 * 128, diff = 0;
        for (k = 0; k < 32768; k++) if (snap[k] != screen[k] && (k < rowa || k >= rowb)) diff = 1;
        check(!diff, "panel_cash solo toca la fila de HERE");
    }
    cash_set(1000UL + 50000UL);
    memcpy(snap, screen, 32768);
    fresh();
    for (k = 0; k < 32768 && snap[k] == screen[k]; k++) ;
    check(k == 32768, "panel_cash = pintar los creditos desde cero");
    printf("%d comprobaciones, %d fallos\n", total, fails);
    return fails != 0;
}
