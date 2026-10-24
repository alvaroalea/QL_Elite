/* Guardar y cargar comandantes en fichero (QDOS: flp1_, mdv1_...; PC: directorio actual o el que se indique).
   Formato propio (94 bytes), con el bloque de 77 bytes del original y sus sumas de comprobacion:
     0..3   "ELQL"      4   version (1)      5..8  semilla del cifrado      9..16  nombre (8 bytes, relleno con 0)
     17..93 bloque del comandante (77 bytes)
   Desde el byte 9 todo va cifrado con XOR contra una secuencia generada con la semilla (el mismo estilo que MUTILATE:
   sirve para que no se pueda editar a ojo, no es seguridad). */
#ifndef SAVE_H
#define SAVE_H
#include "platform.h"

#define SAVE_FILE_SIZE 94

#define SAVE_OK        0
#define SAVE_E_OPEN   (-1)       /* no se pudo abrir (no existe, disco no listo, protegido...) */
#define SAVE_E_IO     (-2)       /* error de lectura o escritura (disco lleno...) */
#define SAVE_E_FORMAT (-3)       /* no es un fichero de Elite QL */
#define SAVE_E_CHECK  (-4)       /* las sumas de comprobacion no cuadran */
#define SAVE_E_NAME   (-5)       /* nombre vacio */

#define SAVE_DEV_MAX 31
extern char save_dev[SAVE_DEV_MAX + 1];        /* prefijo del dispositivo: "flp1_" en el QL, "" en el PC */
extern u8   save_seed;           /* semilla del cifrado de la proxima grabacion (el juego pone un byte aleatorio) */

void save_set_device(const char *dev);
int  save_valid_device(const char *dev);           /* 1 si es AAAn_[texto_]: 3 letras, cifra, _ y acaba en _ */
int  save_next_device(void);                       /* alterna flp1_ -> flp2_ -> mdv1_ -> mdv2_ ; devuelve el indice nuevo */
void save_filename(const char *name, char *out);   /* out de al menos 48 bytes */
int  save_encode(const u8 *blk, const char *name, u8 *file);          /* bloque (ya sellado) -> 94 bytes */
int  save_decode(const u8 *file, char *name, u8 *blk);                /* 94 bytes -> nombre + bloque; SAVE_E_FORMAT si no es de Elite */
int  save_commander(void);                         /* guarda el comandante actual: svc/2, sella y escribe */
int  load_commander(const char *name);             /* lee, valida (CHK, CHK3) y aplica */
const char *save_errstr(int err);
#endif
