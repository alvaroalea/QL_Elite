#include "platform.h"
#include <math.h>
int main(void)
{
    plat_init(); gfx_clear(C_BLACK);
    for (int c = 0; c < 8; c++) gfx_rect(8 + c * 30, 4, 28, 20, c);          /* paleta */
    for (int a = 0; a < 360; a += 15) {                                       /* abanico de lineas */
        double r = a * 3.14159265 / 180;
        gfx_line(128, 100, 128 + (int)(70 * cos(r)), 100 + (int)(60 * sin(r)), (a / 15) % 7 + 1, 0);
    }
    gfx_line(10, 180, 245, 230, C_WHITE, 0);
    gfx_line(10, 180, 245, 230, C_WHITE, 1);                                  /* XOR borra la anterior */
    gfx_line(10, 230, 245, 180, C_GREEN, 1);
    plat_present(); return 0;
}
