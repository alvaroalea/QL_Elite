/* PC test backend: the buffer has the same format as the QL screen. */
#include "platform.h"
#include <stdio.h>
#include <time.h>
static u8 buf[SCR_BYTES];
u8 *screen = buf;
void plat_init(void) {}
void plat_shutdown(void) {}
void plat_flush_input(void) {}
void plat_present(void)
{
    FILE *f = fopen("screen.bin", "wb");
    if (f) { fwrite(buf, 1, SCR_BYTES, f); fclose(f); }       /* no header: tools/ql2png.py <bin> <png> [4] (the 4 = mode 4) */
}
void plat_mode(int m4)
{
    if ((m4 ? 1 : 0) == (int)gfx_m4) return;
    gfx_set_mode(m4);
    gfx_clear(C_BLACK);
}
void plat_wait_key(void) {}
int plat_default_dir(int prog, char *buf, int max) { (void)prog; (void)max; buf[0] = 0; return 0; }
unsigned long plat_ticks(void) { return (unsigned long)(clock() * 50L / CLOCKS_PER_SEC); }
void plat_exit(void) {}
