/* Panel de instrumentos (dashboard) del Elite de Apple II, port sin verificar (ver PENDIENTES.md).
   El fondo es un bitmap que el original carga del disco (A.SCREEN): aqui se convierte con tools/gen_dashboard.py a
   data/elite_dash (tira de 256 x 56 pixeles en modo 8) y se carga de <unidad>elite_dash. Si falta el fichero se dibuja un panel
   minimo (marco, pistas y etiquetas) para que los instrumentos funcionen igual.
   Instrumentos (DIALS): velocidad, alabeo, cabeceo, 4 bancos de energia, escudos delantero y trasero, combustible, altitud,
   temperatura de cabina y de los lasers, y la brujula (COMPAS). Todo se dibuja en XOR: borrar = redibujar. */
#ifndef DASH_H
#define DASH_H
#include "platform.h"

#define DASH_Y0     136                 /* primera linea del panel en pantalla (el area de vista ocupa 0..135) */
#define DASH_LINES  56
#define DASH_BYTES  (DASH_LINES * 128)  /* 7168 bytes: 56 lineas de 128 bytes (modo 8) */

#define DASH_OK        0
#define DASH_E_OPEN   (-1)
#define DASH_E_FORMAT (-2)

int  dash_load(void);                   /* lee <unidad>elite_dash; DASH_OK, DASH_E_OPEN o DASH_E_FORMAT */
int  dash_loaded(void);                 /* 1 si el bitmap esta en memoria */
int  dash_loads(void);                  /* veces que se ha leido el fichero del disco: una vez cargado se queda en memoria */
void dash_show(void);                   /* dibuja el fondo (o el panel minimo) y reinicia los instrumentos */
void dash_reset(void);                  /* borra la memoria de lo ultimo dibujado (tras repintar el fondo) */
void dash_update(void);                 /* DIALS: actualiza los instrumentos y la brujula */
void dash_msbar(u8 x, u8 color);         /* MSBAR: indicador X (1..4) con el color del original (0 = apagado) */
void dash_msblob(void);                 /* msblob: indicadores de misil (NOMSL en verde, el resto apagados) */
void dash_spblb(void);                  /* SPBLB: enciende o apaga (XOR) la bombilla S de estacion cerca */
void dash_ecblb(void);                  /* ECBLB: enciende o apaga (XOR) la bombilla E del ECM */
#endif
