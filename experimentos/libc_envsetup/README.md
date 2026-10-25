# libc.a de c68 4.24f con `_envsetup` corregido

Experimento separado del juego. El juego no lo usa: lleva su propio `_envsetup` en `src/plat_ql.c`.

## El fallo

`Cinit` (el arranque de c68) llama a `_envsetup(env)` antes de `main`. Esa rutina copia en `PROG_USE`, `DATA_USE` y `SPL_USE` los directorios por defecto de Toolkit II, que son punteros a cadenas QDOS en `SV+$AC`, `$B0` y `$B4`. La copia la hace `qlstr_to_c` sobre un búfer de 50 bytes en la pila, sin comprobar el puntero ni la longitud.

Sin Toolkit II esos punteros valen 0, así que lee la cadena de la dirección 0 de la ROM. Su primera palabra hace de longitud:

| ROM | palabra en $0000 | efecto |
|---|---|---|
| JS | `$0003` | 3 bytes de basura (inofensivo) |
| Minerva 1.98 | `$4C02` | copia 19.458 bytes, destroza la pila y se cuelga antes de `main` |
| cualquiera + TK2 | puntero válido | funciona |

Hay un segundo fallo: `_setcd` arma `nombre=valor` en otro búfer de 50 bytes, así que un directorio de más de 40 caracteres también lo desborda.

## Ficheros

- `orig/libc.a`, `orig/_envsetup.o`: la biblioteca y el módulo originales, extraído con `slb -x`.
- `envsetup_orig.asm`: transcripción fiel del módulo, desensamblado con capstone del ejecutable enlazado y traducido a símbolos con el mapa de `qld -ms`. Enlazada en lugar de la original da un ejecutable **idéntico byte a byte**.
- `envsetup.asm`: la misma transcripción con la corrección en `e_set`:
  - el puntero debe ser no nulo, par y estar en la RAM (entre las variables del sistema y `SV_RAMT`);
  - la longitud debe ir de 1 a 40;
  - el búfer se pone a cero antes de `__getcd`.
- `prueba.c`: programa mínimo para enlazar y comparar.
- `Makefile`: `make` genera `lib/libc.a` (copia de `orig/libc.a` con el módulo sustituido; otra de partida con `make SYSLIB=/ruta/libc.a`); `make check` hace las comprobaciones.
- `lib/libc.a`: el resultado.

## Trampas de las herramientas

- `as68` pone como nombre del módulo SROFF el nombre del fichero de salida **tal cual**, con directorio y extensión. `slb -r` solo sustituye un módulo con el mismo nombre (`_envsetup`); si no coincide, lo añade al final (y el enlazador sigue cogiendo el original, que va antes) o no hace nada. Por eso se ensambla dentro del directorio de destino a un fichero llamado `_envsetup` y luego se copia a `.o`.
- `qdos-ar` es un guion que solo deja `-rc`/`-x`/`-t`; para lo demás hay que usar `slb` directamente.
- El desensamblador de `slb -A` se desalinea con el texto del principio del módulo (`*ENV*`) y da basura. `llvm-mc` para m68k se salta instrucciones. Capstone va bien.

## Pendiente

No está probado en un QL ni en un emulador. Para probarlo: quitar `_envsetup` y `tk2_default` de `src/plat_ql.c`, enlazar con `-L<esta carpeta>/lib/` y arrancar en Minerva sin TK2.
