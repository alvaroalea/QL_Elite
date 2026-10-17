/* Bloque del comandante (77 bytes, bytes #0..#76 del original) y sus tres sumas de comprobacion.
   Disposicion y algoritmos de CHECK / CHECK2 / SVE / DFAULT del Elite de Apple II; el codigo es propio. */
#ifndef CMDR_H
#define CMDR_H
#include "platform.h"

#define CMDR_SIZE    77            /* bytes #0..#76: TP ... CHK */
#define CMDR_NAMELEN 7             /* hasta 7 letras, como el limite de nombres de DFS */
#define CMDR_NT      76            /* NT% del original = SVC + 3 - TP */

/* desplazamientos dentro del bloque */
#define CB_TP     0
#define CB_QQ0    1
#define CB_QQ1    2
#define CB_SEED   3                /* 6 bytes */
#define CB_CASH   9                /* 4 bytes, big-endian */
#define CB_FUEL   13
#define CB_COK    14
#define CB_GCNT   15
#define CB_LASER  16               /* 4 bytes: frontal, trasero, izquierdo, derecho */
#define CB_CRGO   22
#define CB_HOLD   23               /* 17 bytes */
#define CB_ECM    40
#define CB_BST    41
#define CB_BOMB   42
#define CB_ENGY   43
#define CB_DKCMP  44
#define CB_GHYP   45
#define CB_ESCP   46
#define CB_NOMSL  51
#define CB_FIST   52
#define CB_AVL    53               /* 17 bytes */
#define CB_QQ26   70
#define CB_TALLY  71               /* 2 bytes, byte bajo primero */
#define CB_SVC    73
#define CB_CHK2   74
#define CB_CHK3   75
#define CB_CHK    76

/* valores de las armas (POW etc. del original) */
#define LASER_POW    15            /* laser de pulsos */
#define LASER_BEAM   (128 + 15)    /* laser de haz */
#define LASER_MIL    151           /* militar: INT(128.5 + 1.5 * POW) */
#define LASER_MINING 50

extern char cmdr_name[CMDR_NAMELEN + 1];     /* nombre actual, terminado en 0 */

u8   cmdr_check(const u8 *blk);              /* CHECK: suma de comprobacion principal (bytes #0..#73) */
u8   cmdr_check2(const u8 *blk);             /* CHECK2: tercera suma (Apple II / C64) */
void cmdr_default(void);                     /* JAMESON: el comandante por defecto, en Lave; llama a cmdr_unpack con el bloque de fabrica */
void cmdr_pack(u8 *blk);                     /* variables del juego -> bloque (sin sumas: ponerlas con cmdr_seal) */
void cmdr_seal(u8 *blk);                     /* calcula CHK3, CHK y CHK2 = CHK ^ $A9 (como SVE) */
/* bloque -> variables del juego. Devuelve 0 si es valido, -1 si CHK o CHK3 no cuadran (no toca nada).
   Si solo falla CHK2 se marca cok bit 7 (datos alterados), como DFAULT; cok bit 3 siempre se pone. */
int  cmdr_unpack(const u8 *blk);
int  cmdr_set_name(const char *s);           /* limpia y recorta el nombre (letras, cifras, '_' y '-'); devuelve la longitud, 0 si queda vacio */
#endif
