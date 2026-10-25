# Elite (Apple II) → Sinclair QL — memoria del proyecto

Memoria del proyecto: lo necesario para continuar el trabajo en otra sesión, cuenta o LLM. Se actualiza al final de cada paso junto con `docs/INVENTARIO.md` (vía `tools/inventory.py`) y los demás ficheros de `docs/`.

La LLM actúa como experta en Sinclair QL, Apple II, C y ensamblador de 6502 y 68000.

**Detalle en `docs/`** (leer el que toque antes de trabajar en esa área):
- `docs/HERRAMIENTAS.md` — instalar XTC68, herramientas Python de `tools/`, `c68_check`, imágenes `elite.img` (qltools) y `elite.mdv` (mdvtool), verificar contra el 6502.
- `docs/HARDWARE_QL.md` — pantalla modos 4 y 8 (y cuándo se cambia), consola, `KEYROW`, joystick, `BEEP` por IPC, trap #3, disco y unidad del juego, `_envsetup`.
- `docs/HALLAZGOS.md` — comportamientos y fallos del original que se reproducen (acarreos de `DORND`, `LOIN`, `MT17`, láser, daños, TACTICS...). **Consultar antes de portar o tocar una rutina.**
- `docs/DECISIONES.md` — rutinas «no necesarias», decisiones de diseño y mejoras propias retiradas (y cómo recuperarlas).

## 1. Resumen

- **Objetivo:** port de **Elite, versión Apple II (Firebird, 1985)** al **Sinclair QL** (68008 a 7,5 MHz, bus de 8 bits, sin FPU, ≤ 640 KB; **modo 8: 256×256, 8 colores; opcionalmente (opciones de la pausa, tecla M) título y vuelo en modo 4: 512×256, negro/rojo/verde/blanco**), en **C89** con **qcc (XTC68/c68)** y ensamblador `as68`. Versión de disco.
- **Referencias** (solo para verificar, no son parte del proyecto): `ref/moxon-appleII` (Mark Moxon; `1-source-files/main-sources/elite-source.asm`, rutinas con cabecera `;       Name: X`, y `elite-data.asm`; **sin licencia**: el código del port es propio y no copia sus comentarios), `ref/moxon-bbc-a`, `ref/TXTELITE.C` (Text Elite 1.5). Emuladores: QPC2, uQLx, sQLux. Juego original de Ian Bell y David Braben.
- **Estrategia:** HAL para lo que depende del hardware (no se compara con el 6502); el resto se porta a C sin verificar cada rutina, anotándola en `tools/ported.txt` (de ahí sale `docs/INVENTARIO.md`); con el juego arrancando en el QL se verifica contra el emulador de 6502; después, profiling, paso a ensamblador y mejoras.

## 2. Cómo trabajar

- Las herramientas usan las rutas por defecto de `ref/`; si la fuente está en otro sitio, `export ELITE_SRC=<ruta>/1-source-files/main-sources`. Sin la fuente funcionan todos los tests salvo los que comparan con el 6502 (`test_orig`, `test_trade`, `test_laser`, `test_missions`...) y `tools/inventory.py`.
- **Línea base:** `make test_all` (y `make c68_check` si hay toolchain; con la fuente, `test_orig`, `test_trade`, `test_laser`, `test_damage`, `test_planet`, `test_equip`, `test_tactics`, `test_missions`) debe dar 0 fallos.
- **Compilar para el QL:** `make elite` (el `Makefile` pone `PATH`/`QLINC`/`QLLIB` si existe `/tmp/qlsdk`; si no, ver `docs/HERRAMIENTAS.md`). **Probar en el QL o emulador: `make elite.img`** → `build/elite.img` (montar como `flp1_`, `LRUN flp1_boot`). Todo lo generado va a `build/`.
- **Entrega al final de cada paso:** `elite_ql_base.tar.gz` (código + tests + docs, incluido `docs/`, sin binarios), `CLAUDE.md`, `docs/INVENTARIO.md` y `data/`. Verificar el tar desempaquetándolo en un directorio limpio y pasando los tests.

### 2.1 Flujo para portar una rutina

1. Leer la rutina en `elite-source.asm` (`;       Name: X`; multiparte: `X (Part n of m)`). Sin comentarios: `sed -n A,Bp elite-source.asm | sed 's/;.*//' | grep -v '^\s*$'`.
2. Escribir el C **transcribiendo el flujo** (con `goto` si hace falta, incluidos saltos al medio de rutinas y caídas de una en otra) y **modelando los acarreos** (`em_carry`, `em_rand_c`, `c`/`cc` locales). Si una instrucción usa el acarreo con que vuelve una subrutina, calcularlo (p. ej. el de `MLTU2` es siempre 0).
3. Con `int d` y comparaciones: **signo/acarreo ANTES de truncar a `u8`** (sección 4).
4. Añadir la línea a `tools/ported.txt` (`NOMBRE funcion_C[,otra] estado nota`; estados `verificada | transcripcion | sin-verificar | propia | parcial | no-necesaria`) y ejecutar `python3 tools/inventory.py` (regenera `docs/INVENTARIO.md`).
5. Pruebas en `tests/` y, si procede, una **sonda c68** `tests/c68/probe_*.c` (añadirla a `tools/c68_check/run.sh`).
6. Verificar contra el 6502 cuando se pueda (`docs/HERRAMIENTAS.md`, sección 5.3): entradas aleatorias y límite, comparar, mutar una instrucción del C y ver que falla; pasar la fila a `verificada`.

## 3. Estructura del repositorio

```
Makefile        compilación y pruebas
docs/           INVENTARIO.md (estado de las rutinas, generado por tools/inventory.py desde tools/ported.txt),
                MODULOS_PRUEBAS.md, PLAN.md y PENDIENTES.md (históricos, no se actualizan) y el detalle técnico (ver arriba)
asset/          ficheros externos para generar datos
data/           distribución en el disco: elite_dash, elite_scr, boot (README.txt no se distribuye)
build/          ejecutables e intermedios
src/            código del juego
tests/          test_*.c, c68/ (sondas), oracle/, shots/, vectors*.txt
tools/          herramientas Python y c68_check
ref/            fuentes de referencia
tmp/            utilidades de terceros compiladas aquí si no están en local
```

## 4. Convenciones y trampas que afectan a casi todo

- **C89 puro:** sin `stdint.h`, sin declaraciones mezcladas ni en `for`, sin `float`; `u8 = unsigned char`; ensamblador en ficheros `.asm` (sintaxis Sozobon: `.globl`, `.text`, comentarios `;`).
- **`int`:** el `qcc` del `Makefile` lo compila de 32 bits, `make c68_check` de 16 (`-int=16`). No depender del tamaño: `long` para valores > 32767 (`SCR_BYTES = 32768L`) y no confiar en promociones. **El ensamblador no lee por símbolo variables `int`** (solo `u8` y punteros): los `int` se pasan como argumentos `long`.
- `qcc` busca los `#include "..."` en el directorio actual: siempre `-Isrc`.
- **Fallo del optimizador `-O` de c68:** en `x = (u8)d; cc = (d >= 0);` usa los flags del `move.b` y no los de `d` (naves deformadas, números mal en `dt_bprnt`). **Regla: calcular siempre el signo/acarreo ANTES de guardar el byte** (`cc = (d >= 0); x = (u8)d;`). Una sonda de c68 por cada módulo nuevo.
- **Sondas c68:** en el PC `unsigned long` es de 64 bits; enmascarar los generadores congruenciales (`& 0xFFFFFFFFUL`) o la sonda sale distinta sin que c68 falle.
- **IPC del QL:** si se pide al 8049 una respuesta que no manda, el QL se cuelga (`BEEP` = respuesta 1). Ver `docs/HARDWARE_QL.md`.

### Reglas de oro
- **Dibujo en XOR:** borrar = volver a dibujar lo mismo en el mismo orden. Si se limpia una zona sin avisar al montón de líneas (naves, sol, círculo), hay que **olvidar** esas líneas.
- **Acarreos como parámetros:** modelarlos explícitamente; ante la duda, ver qué instrucción los consume (`ADC`/`SBC`/`ROL`/`BCS`).
- **Una primitiva propia que sustituya a una del original debe reproducir sus efectos en las variables que otro lee después** (p. ej. `SWAP` de `LOIN`, ver `docs/HALLAZGOS.md`).
- **Datos de naves** se extraen con script, no a mano.
- **Entregar siempre** el tar + los `.md` al final de cada paso y comprobar el tar en un directorio limpio.
