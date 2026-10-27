/* Dump of the universe with universe.c, in the same format as the oracle (TXTELITE.C 1.5). */
#include <stdio.h>
#include "universe.h"
u8 *screen;
void plat_init(void){} void plat_present(void){} void plat_shutdown(void){}
static char buf[4096]; static int len;
static void cap(int c) { if (len < 4095) buf[len++] = (char)c; buf[len] = 0; }
int main(void)
{
    static const unsigned fl[4] = { 0x00, 0x37, 0xAB, 0xFF };
    int g, i, j, k;
    uni_market m;
    uni_putc = cap;
    for (g = 1; g <= 8; g++) {
        uni_build_galaxy(g);
        for (i = 0; i < UNI_NSYS; i++) {
            const uni_system *p = &uni_galaxy[i];
            printf("S %d %d %s %u %u %u %u %u %u %u %u %u\n", g, i, p->name, p->x, p->y, p->economy, p->govtype, p->techlev, p->population, (unsigned)p->productivity, (unsigned)p->radius, 0u);
            len = 0; buf[0] = 0;
            uni_describe(p);
            printf("D %d %d %s\n", g, i, buf);
            for (k = 0; k < 4; k++) {
                uni_gen_market(fl[k], p, &m);
                printf("M %d %d %u", g, i, fl[k]);
                for (j = 0; j < UNI_NGOODS; j++) printf(" %u/%u", m.quantity[j], m.price[j]);
                printf("\n");
            }
        }
    }
    for (g = 1; g <= 8; g += 7) {
        uni_build_galaxy(g);
        for (i = 0; i < UNI_NSYS; i++) for (j = 0; j < UNI_NSYS; j++)
            printf("T %d %d %d %u\n", g, i, j, uni_distance(&uni_galaxy[i], &uni_galaxy[j]));
    }
    return 0;
}
