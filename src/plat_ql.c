/* Backend QL (c68 / QDOS). SIN PROBAR: esqueleto para validar en hardware real o emulador (QPC2/uQLx). */
#include "platform.h"
#include <qdos.h>

uint8_t *screen = (uint8_t *)0x20000;      /* pantalla 1 del QL estandar */

void plat_init(void)
{
    short mode = 8, tv = -1;               /* modo 8, sin cambiar TV */
    mt_dmode(&mode, &tv);
    /* NOTA: escribir directo en $20000 requiere modo supervisor (trap #0) o SD.EXTOP en una ventana */
}
void plat_present(void) {}                 /* se dibuja directo en pantalla */
void plat_shutdown(void) {}
