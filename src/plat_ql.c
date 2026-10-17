/* Backend QL (XTC68 / qcc). SIN PROBAR: esqueleto para validar en QL real o emulador. */
#include "platform.h"
#include <qdos.h>

u8 *screen = (u8 *)0x20000L;               /* pantalla 1 del QL estandar */

void plat_init(void)
{
    short mode = 8, tv = -1;               /* modo 8, sin cambiar TV */
    mt_dmode(&mode, &tv);
    /* NOTA: escribir directo en $20000 puede requerir modo supervisor (trap #0) o SD.EXTOP */
}
void plat_present(void) {}
void plat_shutdown(void) {}
