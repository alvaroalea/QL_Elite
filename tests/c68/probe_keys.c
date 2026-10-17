/* Teclado: contextos de filas, flancos, caracteres y controles con patrones pseudoaleatorios de la matriz. */
#include "platform.h"
#include "keys.h"
typedef unsigned short u16s;
extern u8 host_matrix[8];
extern int host_reads;
static u16s hv[16];
static int nh = 0;
static u16s hh;
static void hb(u8 b) { hh = (u16s)(((hh << 5) | (hh >> 11)) ^ b); }
int main(void)
{
    int ctx, step, i, ch;
    unsigned seed = 12345;
    struct controls c;
    plat_init();
    for (ctx = 0; ctx < 4; ctx++) {
        key_init();
        ctl_joystick = (ctx == 3) ? 2 : (ctx == 2 ? 1 : 0);
        if (ctx == 0) key_ctx_menu(); else key_ctx_flight();
        hh = 0;
        for (i = 0; i < 8; i++) host_matrix[i] = 0;
        for (step = 0; step < 300; step++) {
            seed = (unsigned)(seed * 25173u + 13849u);
            if ((seed & 7) == 0) { i = (int)((seed >> 4) & 7); host_matrix[i] = (u8)(host_matrix[i] ^ (1 << ((seed >> 8) & 7))); }
            host_reads = 0;
            key_scan();
            hb((u8)host_reads);
            for (i = 0; i < 8; i++) hb((u8)key_raw(i));
            while ((ch = key_getchar()) >= 0) hb((u8)ch);
            ctl_read(&c);
            hb((u8)(c.slow_down | (c.speed_up << 1) | (c.roll_left << 2) | (c.roll_right << 3) | (c.pitch_up << 4) | (c.pitch_down << 5) | (c.fire << 6)));
            hb((u8)(c.bomb | (c.escape_pod << 1) | (c.target << 2) | (c.unarm << 3) | (c.missile << 4) | (c.ecm << 5) | (c.jump << 6) | (c.dock << 7)));
            hb((u8)(key_pressed(K_A) | (key_released(K_A) << 1) | (joy_read(1) << 2) | (key_any() << 7)));
        }
        hv[nh++] = hh;
    }
    gfx_clear(C_BLACK);
    for (i = 0; i < nh; i++) { screen[2 * i] = (u8)hv[i]; screen[2 * i + 1] = (u8)(hv[i] >> 8); }
    screen[200] = (u8)nh;
    plat_present();
    return 0;
}
