/* Backend de pruebas en PC: el buffer tiene el mismo formato que la pantalla del QL. */
#include "platform.h"
#include <stdio.h>
static u8 buf[SCR_BYTES];
u8 *screen = buf;
void plat_init(void) {}
void plat_shutdown(void) {}
void plat_flush_input(void) {}
void plat_present(void)
{
    FILE *f = fopen("screen.bin", "wb");
    if (f) { fwrite(buf, 1, SCR_BYTES, f); fclose(f); }
}
