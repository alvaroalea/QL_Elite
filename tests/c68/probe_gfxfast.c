/* Relleno y copia rapidos (palabras largas en c68): hashes de la pantalla tras rellenos con recorte y copias con direcciones pares e impares. */
#include "platform.h"
#include "text.h"
typedef unsigned short u16s;
static u16s hv[40];
static u16s rs = 4242;
static unsigned rnd(void) { rs = (u16s)(rs * 25173u + 13849u); return (unsigned)(rs >> 4); }
static u8 src[7300], dst[7300];
int main(void)
{
    int n = 0, i, k, col, y0, nn;
    long j;
    u16s h;
    plat_init(); txt_init();
    for (i = 0; i < 7300; i++) src[i] = (u8)(i * 7 + 3);
    for (k = 0; k < 24; k++) {
        col = (int)(rnd() & 7); y0 = (int)(rnd() % 300) - 20; nn = (int)(rnd() % 120);
        gfx_clear((u8)(rnd() & 7));
        gfx_fill_lines(y0, nn, (u8)col);
        gfx_rect(0, (int)(rnd() % 200), 256, (int)(rnd() % 60), (u8)(rnd() & 7));
        txt_clear_rows((int)(rnd() % 32), (int)(rnd() % 8), (u8)(rnd() & 7));
        h = 0;
        for (j = 0; j < 32768L; j++) h = (u16s)(((h << 5) | (h >> 11)) ^ screen[j]);
        hv[n++] = h;
    }
    for (k = 0; k < 4; k++) {
        for (i = 0; i < 7300; i++) dst[i] = 0;
        gfx_copy(dst + k, src + (3 - k), 7168L);
        h = 0;
        for (j = 0; j < 7300L; j++) h = (u16s)(((h << 5) | (h >> 11)) ^ dst[j]);
        hv[n++] = h;
    }
    gfx_copy(screen + 128L * 136L, src + 2, 7168L);          /* a la pantalla, como el panel */
    gfx_copy(screen, src, 7168L);
    h = 0;
    for (j = 0; j < 32768L; j++) h = (u16s)(((h << 5) | (h >> 11)) ^ screen[j]);
    hv[n++] = h;
    gfx_clear(C_BLACK);
    for (i = 0; i < n; i++) { screen[2 * i] = (u8)hv[i]; screen[2 * i + 1] = (u8)(hv[i] >> 8); }
    screen[200] = (u8)n;
    plat_present();
    return 0;
}
