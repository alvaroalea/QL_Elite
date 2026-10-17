# Elite (Apple II) → Sinclair QL — memoria del proyecto

Este documento es la **memoria completa del proyecto**: contiene todo lo necesario para continuar el trabajo desde cero, en otra sesión, otra cuenta u otro LLM. Estado a **2026-10-17**. Se actualiza al final de cada paso, junto con `PENDIENTES.md` e `INVENTARIO.md`, y se entrega con el código (`elite_ql_base.tar.gz`).

Índice: 1 Resumen · 2 Cómo continuar (checklist de arranque) · 3 Entorno y herramientas · 4 Estructura del repositorio · 5 Pruebas y verificación · 6 Arquitectura técnica · 7 Hardware QL · 8 Controles · 9 Formatos de datos · 10 Trampas, reglas y hallazgos · 11 Inventario de rutinas · 12 Qué falta (plan) · 13 Decisiones y desviaciones del original · 14 Historial · 15 Referencias.

---

## 1. Resumen

- **Objetivo:** port de **Elite, versión Apple II (Firebird, 1985)** al **Sinclair QL** (68008 a 7,5 MHz, bus de 8 bits, sin FPU, hasta 640 KB de RAM, **modo 8: 256×256, 8 colores**), en **C89** con **qcc (XTC68/c68)** y un poco de ensamblador `as68`. Versión de disco. Port **fiel** al Apple II con aritmética de enteros de 8 bits (no se usa `newkind` ni coma flotante).
- **Fuente de referencia:** el código documentado de Mark Moxon, `elite-source-code-apple-ii` (GitHub; el usuario aportó `elite-source-code-apple-ii-copy.zip`). Ficheros: `1-source-files/main-sources/elite-source.asm` (~47.000 líneas, 528 rutinas con cabecera `;       Name: X`) y `elite-data.asm`. **No tiene licencia**: se usa solo como referencia; el código del port es propio y no copia sus comentarios. El juego original es de Ian Bell y David Braben.
- **Estrategia (desde 2026-10-16):** se porta **todo** el juego a C **sin** verificar cada rutina contra el original mientras tanto; cada función nueva se anota en `PENDIENTES.md` (rutina de origen y cómo verificarla) y en `tools/ported.txt`. Cuando el juego arranque en el QL real, se verifica la lista contra el emulador de 6502 y el feedback del QL real reordena las prioridades. Lo ya verificado se mantiene (sección 5).
- **Estado:** el juego completo **compila y enlaza con qcc** (`make ql_game` → ejecutable QDOS `elite`, ~96 KB, dataspace 18.114 bytes) y funciona en PC con el backend de pantalla en memoria. **Título → atracado ↔ vuelo**: menús (estado, inventario, mercado, compra/venta, equipo, datos del sistema, cartas galáctica y local, disco), guardar/cargar comandante, hiperespacio y salto galáctico con túnel, despegue/atraque, panel de instrumentos, polvo estelar, radar, mira, **naves que se mueven (`MVEIT`) y se dibujan (`LL9`) en las 4 vistas, planeta, sol, estación y explosiones (`DOEXP`)**. **Falta** lo que hace que sea un juego: IA (`TACTICS`), combate, encuentros, atraque real, misiones, muerte (sección 12).
- **Inventario (528 rutinas):** 334 portadas (25 verificadas contra el 6502, 309 pendientes de verificar), 140 no necesarias, **54 sin portar**. Detalle por área en `INVENTARIO.md`.
- **Probado en el QL real por el usuario:** gráficos y texto, teclado, sonido, demo de universo, `plat_wait_key`, y varias rondas del juego hasta el túnel/polvo/radar/mira. **Sin probar en el QL** todo lo posterior a la sexta ronda (naves, planeta, explosiones, borrado rápido, teclas nuevas) y el ejecutable `elite` recién enlazado.

## 2. Cómo continuar (checklist de arranque)

1. **Desempaquetar** `elite_ql_base.tar.gz` (el sandbox es efímero; el tar es la copia buena). Leer este documento, `PENDIENTES.md` (lista viva de verificación) e `INVENTARIO.md` (estado por rutina). `PLAN.md` es el plan original (histórico).
2. **Dependencias:** `python3` (con `Pillow` para `tools/ql2png.py`), `gcc`, `make`, `git`, `meson`, `ninja`; para `make c68_check`: `qemu-user` (`qemu-m68k`) y `gcc-m68k-linux-gnu` (`m68k-linux-gnu-as`, `-ld`).
3. **Fuente de referencia** (solo para verificar y para leer el original; no se incluye en el proyecto):
   `git clone https://github.com/markmoxon/elite-source-code-apple-ii /tmp/moxon` y `export ELITE_SRC=/tmp/moxon/1-source-files/main-sources`. (En la sesión actual estaba descomprimido en `/tmp/moxon/elite-source-code-apple-ii-main/1-source-files/main-sources`, que es el valor por defecto de `tools/inventory.py`.) Sin la fuente funcionan todos los tests salvo `make test_orig`, `make test_trade` y `tools/inventory.py`.
4. **Línea base:** `make test_all` (y `make c68_check` si hay toolchain) debe dar 0 fallos antes de tocar nada.
5. **Compilador del QL:** `tools/install_xtc68.sh` instala XTC68 + runtime c68 en `/tmp/qlsdk` (ver 3.2). Luego `export PATH=/tmp/qlsdk/bin:$PATH QLINC=/tmp/qlsdk/share/qdos/include QLLIB=/tmp/qlsdk/share/qdos/lib` y `make ql_game`.
6. **Flujo de trabajo por rutina** (ver 2.1) y **entrega al final de cada paso**: `elite_ql_base.tar.gz` (código + tests + docs, sin binarios), `Proyecto.md`, `PENDIENTES.md`, `INVENTARIO.md` (y `elite_dash` si cambia). Verificar el tar desempaquetándolo en un directorio limpio y pasando los tests.
7. **Siguiente paso recomendado:** sección 12 (paso 3: combate e IA). Antes, si el usuario ha probado en el QL, atender primero su feedback.

### 2.1 Flujo de trabajo para portar una rutina

1. Leer la rutina en `elite-source.asm` (buscar `;       Name: X`; las rutinas multiparte se llaman `X (Part n of m)`). Extraer el código sin comentarios: `sed -n A,Bp elite-source.asm | sed 's/;.*//' | grep -v '^\s*$'`.
2. Escribir el C **transcribiendo el flujo** (con `goto` si hace falta, incluidos los saltos al medio de rutinas y las caídas de una rutina en otra) y **modelando los acarreos** que usa el 6502 (`em_carry`, `em_rand_c`, variables `c`/`cc` locales). Si una instrucción depende del acarreo con que vuelve una subrutina, hay que calcular ese acarreo (ejemplo: el de `MLTU2` es el bit bajo del producto).
3. Si usa `int d` con comparaciones: **calcular el signo/acarreo ANTES de truncar a `u8`** (bug de c68 `-O`, sección 10).
4. Añadir la línea a `tools/ported.txt` (`NOMBRE funcion_C[,otra] estado nota`; estados: `verificada | transcripcion | sin-verificar | propia | parcial | no-necesaria`) y la fila a `PENDIENTES.md`; ejecutar `python3 tools/inventory.py`.
5. Pruebas propias en `tests/` (probar que el C hace lo que se quiere) y, si procede, una **sonda c68** en `tests/c68/probe_*.c` (añadirla a la lista de `tools/c68_check/run.sh`) para comprobar que c68 compila lo mismo que gcc.
6. Verificar contra el 6502 cuando haya ocasión (sección 5): ejecutar la rutina original en el emulador con entradas aleatorias y valores límite, comparar, probar con mutaciones de una instrucción, y pasar la fila a `verificada`.

## 3. Entorno y herramientas

### 3.1 Qué hay en el sandbox y qué se pierde
El sandbox (`/home/claude`) puede perderse entre sesiones. Todo lo importante está en el tar. Lo que hay que reinstalar cada vez: la fuente de referencia (`/tmp/moxon`), la cadena de qcc (`/tmp/qlsdk`, con `tools/install_xtc68.sh`), `qemu-user` y `gcc-m68k-linux-gnu` si faltan (`apt`/`pip` según permita la red; en la sesión actual la red permite github.com, pypi y los repositorios de Ubuntu).

### 3.2 Compilador de C para el QL (XTC68) — instalación
`tools/install_xtc68.sh` automatiza lo que sigue (probado el 2026-10-17):
1. `git clone https://github.com/stronnag/xtc68` (migrado a Codeberg, pero el clon de GitHub sirve) y `git clone https://github.com/xXorAa/c68-support`.
2. gcc moderno falla con `-Werror` en las fuentes antiguas (p. ej. «ignoring return value of write»): `sed -i "s/'-Werror', *//g" src/*/meson.build`.
3. `meson setup build --prefix=/tmp/qlsdk --strip && ninja -C build install` → `bin/{qcc,c68,as68,qld,qcpp,qdos-ar,...}`.
4. El runtime de c68 (cabeceras `INCLUDE_*` y librerías `LIB_*`, del c68 runtime 4.24 «424frun1») va en una carpeta **`support/424frun1`** del repositorio xtc68, y se instala con `./sdk-install.sh /tmp/qlsdk` ejecutado **desde esa carpeta** → `share/qdos/include` y `share/qdos/lib`.
5. Uso: `export PATH=/tmp/qlsdk/bin:$PATH QLINC=/tmp/qlsdk/share/qdos/include QLLIB=/tmp/qlsdk/share/qdos/lib`; `qcc -O -Isrc -o elite ...` (el `Makefile` ya lo hace con `make ql_game`). `-Isrc` es obligatorio porque qcc busca los `#include "..."` en el directorio actual. `qcc` imprime `elite: dataspace 18114 (46c2)` al terminar bien.
6. `as68` es de la familia Sozobon: directivas `.globl`/`.text`, comentarios con `;`, etiquetas con dos puntos, ficheros `.asm` (qcc no reconoce `.s`); no valen `xdef`, `section` ni comentarios con `*`.

### 3.3 Verificación de código de c68 (`make c68_check`)
`tools/c68_check/run.sh [sonda ...]` compila cada sonda de `tests/c68/` con gcc (PC) y con el `c68` real (`-int=16 -O -warn=0`, preprocesando con `gcc -E -P -U__GNUC__ -nostdinc`), convierte la salida de c68 (sintaxis ACK) a GNU as con `conv.py`, la enlaza con un mini runtime (`rt.s`: `.Xlmul`, `.Xldiv`, ..., salida de pantalla) y la ejecuta bajo `qemu-m68k`. Cada sonda deja hashes en la «pantalla» y se comparan byte a byte. `XTC68=<dir con c68>` (por defecto `/tmp/qlsdk/bin` si existe, si no `/tmp/xtc68/build`). Con `-U__GNUC__` se compilan las rutas «de QL» (por ejemplo las palabras largas de `gfx_fill_lines`). Sondas: `gfxfast, emath, keys, ships, game, charts, dash, flight, session`. **No sustituye al QL real** (no cubre el runtime ni el hardware), pero detecta fallos del compilador (encontró el de `-O`).

### 3.4 Herramientas Python (`tools/`)
| Fichero | Para qué |
|---|---|
| `cpu6502.py`, `asm6502.py` | emulador de 6502 y ensamblador mínimo de la sintaxis de la fuente (etiquetas, EQUB/EQUW, `P%`, `LO()/HI()`, `%bin`, `$hex`, `IF/ELIF`) |
| `orig.py`, `elitesrc.py` | monta una imagen con rutinas originales (LL9, SHPPT, LL51, PROJ, PLS6, DVID3B2, FMLTU, DORND; LOIN/DOEXP/PLANET son stubs que registran la llamada) y lee la fuente (símbolos) |
| `gen_ll9.py` | genera `tests/vectors_ll9.txt` ejecutando el original con entradas aleatorias |
| `ref6502.py`, `ref6502b.py`, `ref6502_state.py` | **transcripciones literales** del 6502 en Python (con acarreos explícitos) de la aritmética y de las rutinas con estado |
| `gen_vectors*.py`, `gen_trade.py` | generan los vectores de `make test`, `test_state`, `test_save` y `test_trade` |
| `gen_shipdata.py` | extrae los 32 planos de naves de `elite-data.asm` → `src/shipdata.c/.h` |
| `gen_dashboard.py` | `A.SCREEN.bin` (volcado hires del Apple II) → `data/elite_dash` |
| `gen_font.py`, `gen_tables.py`, `gen_sne.py`, `gen_universe_text.py`, `make_oracle.py` | fuente 8×8, tablas log/seno, tabla de seno de naves, textos del universo, oráculo de Text Elite |
| `ql2png.py` | convierte una captura de pantalla QL (32 KB) a PNG |
| `inventory.py`, `ported.txt` | generan `INVENTARIO.md` (ver 11) |
| `install_xtc68.sh`, `c68_check/` | cadena de qcc y verificación de c68 |

## 4. Estructura del repositorio

```
Makefile              objetivos de compilación y pruebas (sección 5)
PLAN.md               plan original (histórico)   Proyecto.md / PENDIENTES.md / INVENTARIO.md   documentación viva
data/                 elite_dash (fondo del panel, 56 líneas × 128 B, modo 8) y README.txt
src/                  código del juego (≈8.300 líneas)
tests/                pruebas (test_*.c), c68/ (sondas), oracle/ (Text Elite), shots/ (capturas), vectors*.txt
tools/                herramientas Python y c68_check
```
**Módulos de `src/`:**
| Módulo | Contenido |
|---|---|
| `platform.h`, `plat_ql.c`, `plat_host.c`, `ql_ipc.asm` | capa de plataforma: pantalla `$20000` (QL) o memoria (PC), `plat_present/exit/wait_key`, IPC, `trap #1/#3` |
| `gfx.c` | píxel, XOR, línea, `hline`, `hloin`, `vloin`, `cpix`, `dust`, `rect`, `gfx_fill_lines`/`gfx_copy` (rápidos), `clear` |
| `text.c/.h`, `font.h` | texto 8×8 en celdas 32×32, cursor, `txt_clear_rows` |
| `emath.c/.h`, `math_tables.h` | aritmética del 6502 (verificada): DORND, MULTU, MU11, SQUA, MULT1/12, MAD, ADD, MULTS, FMLTU/2, MLTU2, MLU1/2, MULT3, TIS1/2, DVID96, LL5, LL28, LL61, LL38, DVID4, DV41/42, DVIDT, DVID3B |
| `state.c/.h` | INWK (37 bytes), K% (`ks`), `frin`, `many`, `univ`, página cero (`zk3`, `zx15`...), variables QQ* y del comandante |
| `ship_math.c/.h` | rutinas con estado: MVT3, MAS1-4, TAS1-4/6, NORM, TIS3, TIDY, SPS1-4, PLS6, PROJ, LCASH/MCASH/GCASH, SCALEX/Y/Y2 |
| `ship3d.c/.h`, `shipdata.c/.h`, `shipview.c` | LL9 completo (verificado), recorte (LL118/120/123/129/145), montón de líneas, SHPPT; planos de las 32 naves; orientación y dibujo de una nave (título) |
| `universe.c/.h` | galaxias, sistemas, mercado, distancia, salto galáctico, descripciones (port de Text Elite 1.5; verificado contra su oráculo) |
| `cmdr.c`, `save.c`, `game.c`, `input.c` | comandante (bloque de 77 bytes), guardar/cargar, lógica de juego (comercio, equipo, hiperespacio), entrada |
| `keys.c`, `keys_ql.c`, `keys_host.c`, `sound.c`, `sound_ql.c`, `sound_host.c` | teclado por KEYROW/IPC y sonido por BEEP/IPC, con backends QL y PC |
| `menu.c`, `chart.c`, `sun.c` | pantallas de texto y navegación; cartas galáctica y local, mira; SUN, CIRCLE, BLINE, WPLS2, túnel |
| `dash.c`, `space.c`, `stars.c` | panel (DIALS, radar, brújula, misiles, bombillas); giros, velocidad, altitud, radar (lógica), `space_step`; polvo estelar |
| `move.c`, `ships.c` | **MVEIT** completo, MV40/MV45/MVS4/MVS5/MVT1/MVT6/PLUT; lista de naves, ZINF/NWSHP/KILLSHP/SOLAR/NWSPS, bucle por nave, **DOEXP**, **PLANET** |
| `flight.c`, `app.c`, `main.c` | bucle de vuelo; bucle principal título → atracado ↔ vuelo; punto de entrada |
| `demo_*.c`, `test_text.c` | demos para el QL (`make ql_keys`, `ql_sound`, `ql_universe`, `ql_ship`) |

## 5. Pruebas y verificación

### 5.1 Objetivos de `make` (resultados a 2026-10-17, todos con 0 fallos)
| Objetivo | Qué prueba | Resultado |
|---|---|---|
| `make test` | aritmética (`emath`) + estado (`ship_math`) contra transcripciones literales del 6502 | 81.000 comprobaciones (≈896.000 en total en la suite de aritmética) |
| `make test_orig` | LL9, CLIP, CLIP2, LL51, PROJ contra el **6502 original en el emulador** (necesita `ELITE_SRC`) | 15.000 |
| `make test_keys` / `test_sound` | teclado (contextos de filas, CTL1/CTL2) y sonido | 41 / 26 |
| `make test_universe` | 8 galaxias contra el oráculo de Text Elite y 131.072 distancias | OK |
| `make test_save` | comandante y fichero: 3.003 vectores de CHECK/CHECK2, ida y vuelta, corrupción | 6.244 |
| `make test_game` | lógica (58) + sesión guiada de 311 teclas con 28 capturas en `tests/shots/` (62) | 120 |
| `make test_shipview` | animar el título = dibujar desde cero (6 naves) | 6 |
| `make test_space` | radar, polvo en las 4 vistas, túnel, mira, misiles, bombillas, panel, borrado/copia rápidos, MVEIT, PLUT, altas y bajas, planeta y sol, explosión sin restos, salida de la estación | 694 |
| `make test_trade` | `tnpr`, `TT219`, `TT208` (+ GCASH/LCASH/MCASH) contra el 6502 original (necesita `ELITE_SRC`) | 20.000 × 2 casos; 4 mutaciones detectadas |
| `make c68_check` | 9 sondas: código de c68 bajo qemu = PC | 9 IGUAL |
| `make test_all` | agrupa los anteriores (sin `test_orig`, `test_trade` ni `c68_check`) | |
| `make host_game` / `ql_game` | ejecutable de PC / ejecutable QDOS con qcc | compila |
| `make dash_file` | regenera `data/elite_dash` desde `A.SCREEN.bin` | |

### 5.2 Qué está verificado contra el original y qué no
- **Verificado contra el 6502 original** (emulador, entradas aleatorias): LL9 completo y recorte (`LL118`, `LL120`, `LL123`, `LL129`, `LL145` = CLIP/CLIP2), LL51, PROJ, PLS6/DVID3B2 (a través de PROJ), LL28 y LL61 (exhaustivos: 65.280 y 32.640 pares), `tnpr`, `TT219`, `TT208`. Encontró un fallo real (la negación de LL52 usaba `1-x` en lugar de `1-x-1`). Las pruebas detectan mutaciones de una instrucción (salvo dos equivalentes).
- **Verificado contra transcripciones literales propias del 6502** (`tools/ref6502*.py`; falta el emulador sobre el binario original): `emath.c` (≈896.000 comprobaciones), `ship_math.c` (81.000), `CHECK`/`CHECK2` (3.003).
- **Verificado contra Text Elite** (no es el 6502): `universe.c`.
- **Sin verificar** (portado, pruebas propias que solo comprueban que el C hace lo que se quiere): todo lo demás — la lista está en `PENDIENTES.md` (filas 3–39) con la rutina de origen y cómo verificarla. Prioridad A de verificación: `MVEIT`/`MV45`/`MVS4`/`MVS5`/`PLUT`, `DOEXP`/`PTCLS`/`EXS1`, `PLANET`, polvo estelar, radar, DIALS/COMPAS, cartas, controles.
- **c68 vs PC:** las 9 sondas coinciden (no prueba el original, solo que c68 compila igual que gcc).

### 5.3 Cómo verificar una rutina nueva contra el original
`tools/orig.py` monta una imagen del 6502 con las rutinas originales que se pidan y stubs para las que dibujan; `tools/gen_trade.py` es el ejemplo más completo (preprocesa `IF/ELIF`, sustituye pantalla por stubs, ejecuta `tnpr` y los tramos de `TT219`/`TT208`). Pasos: volcar el estado inicial aleatorio en el emulador y en C, ejecutar, comparar INWK/variables/líneas dibujadas, repetir con valores límite, mutar una instrucción del C y comprobar que el test falla.

## 6. Arquitectura técnica

### 6.1 Convenciones del código
- **C89 puro** (sin `stdint.h`, sin declaraciones mezcladas ni en `for`); `u8 = unsigned char`; **`int` es de 16 bits en c68**: usar `long` para valores > 32767 (`SCR_BYTES = 32768L`) y no confiar en promociones.
- Las variables de página cero son **globales de C** (`zP`, `zP1`, `zQ`, `zR`, `zS`, `zT`, `zU`, `zK[4]`, `em_x`, `em_carry`, `em_rand[4]`, `em_rand_c`, `zk3`, `zx15`...). Los nombres del original van en minúsculas; tamaños y orden de bytes son los del 6502 porque las rutinas indexan con desplazamientos (`INWK+10,X`).
- **INWK** (nave actual) tiene **NI = 37 bytes** (incluye XX19 y NEWB); desplazamientos `I_*` en `state.h`: pos x/y/z (3 bytes cada una: lo, hi, signo) 0–8; morro 9–14, techo 15–20, lateral 21–26 (vectores de 6 bytes: x lo/hi, y lo/hi, z lo/hi); velocidad 27, aceleración 28, contador de alabeo 29, de cabeceo 30, estado 31, IA 32, puntero del montón 33–34 (no se usa en C), energía 35, NEWB 36. **K%** (`ks`) tiene `NOSH+1 = 9` ranuras de NI bytes (la última de resguardo); `state_slot(n)` da la ranura; `frin[n]` es el tipo (0 = libre; `frin[NOSH]` hace de terminador); `many[tipo]`; `sspr = many[SHIP_SST]`.
- **Bits de `INWK+31` (estado):** bit 7 = nave destruida o a punto de explotar, bit 6 = hay nube de explosión dibujada, bit 5 = explotando, bit 4 = punto dibujado en el radar, bit 3 = hay líneas dibujadas en el montón, bits 0–2 = indicador de líneas del plano.
- **Rutinas con acarreo:** el 6502 usa el acarreo como parámetro y resultado; el C lo modela con variables (`em_carry`, `em_rand_c`, `c`). Las rutinas multiparte con saltos al medio se transcriben con `goto`.
- **Ganchos de LL9** (`ship3d.h`): `ship_loin_hook` (dibujo de líneas; por defecto `gfx_line` en XOR), `ship_doexp_hook` (= `doexp`), `ship_planet_hook` (= `planet`); `ships_hooks()` los pone. Variables de entrada de LL9: `ztype` (TYPE), `zxx0` (plano, `SHIP_BP`), `zxx19` (montón de líneas de la nave), `zinf` (bloque K% de la nave).

### 6.2 Bucle de vuelo (`flight.c`, `space.c`, `ships.c`)
Cada fotograma de `flight_run`: leer teclas (`key_ctx_flight` + `ctl_read`), `space_controls` (giros con `cntr`/`BUMP2`/`REDU2`, `ALPHA/BETA/ALP1/BET1`, velocidad `DELTA`), `space_step` (= `mcnt++`, **`ships_step(view)`**, altitud, eventos), polvo (`stars_update`), panel (`dash_update`), mira, `plat_present`. **`ships_step`** (bucle por nave del bucle principal reducido): para cada ranura `n` con `frin[n] != 0`: carga INWK desde K%, fija `ztype/zinf/zxx19/zxx0`, **`mveit()`**, copia INWK a K% (MAL3), **`plut(view)`**, **`ship_ll9()`** (dibuja o borra con XOR), guarda energía (byte 35); luego quita la nave si terminó de explotar (`INWK+31` bits 7 y 5), si está lejos (`FAROF`: alguna coordenada alta > 224) o, para la estación, si se aleja (regla propia: `z_hi > 0x30`), y en ese último caso vuelve el sol. `ship_kill` desplaza las ranuras y sus montones.
- **Montones de líneas:** un montón de 256 bytes por ranura (`heaps[NOSH]` en `ships.c`) en lugar del reparto con `SLSP` del original. Borrar una nave sin pasar por LL9: `zxx19=heap; zlsnum=1; zlsnum2=heap[0]; ship_lsclr();`. Al borrar el área de vista (`clear_view`) hay que **olvidar** las líneas (`ships_forget_lines()`, `sun_forget()`), porque el dibujo es XOR: si el montón cree que hay líneas y no están, la siguiente pasada dibuja basura.
- **Planeta, sol y estación:** `ships_launch()` (planeta en la ranura 0 y estación en la 1; colocación de la estación propia), `ships_arrive()` (planeta y sol; el sol está detrás: z negativa). `planet()` = `PLANET`: si `z_top >= 48` o `z == 0` o `PROJ` falla → quita (`WPLS2` o `WPLS`); si no, `DVID3B2` da el radio, y dibuja `SUN` (tipo impar) o `WPLS2`+`CIRCLE` (tipo par). **`circ_lsy2[0]` debe ser `$FF`** (centinela del montón de la bola).
- **`MVEIT`** (`move.c`): parte 1 (TIDY cada 16 fotogramas; `TACTICS` si `INWK+32` bit 7, vía `move_tactics_hook`, hoy vacío); `SCAN` (radar); partes 2–5 (avance por el morro `INWK+27`, aceleración y velocidad máxima del plano, giro de la posición por nuestro alabeo/cabeceo con `MLTU2`/`MVT6`); `MV45` (z −= DELTA, giro de los vectores por nuestros giros con `MVS4`, contadores de alabeo y cabeceo de la nave con `MVS5`, y radar). Los planetas y soles (tipo ≥ 128) van por `MV40`. **Ojo con los acarreos:** el de `MLTU2` sale del bit bajo del producto (`((zP^$FF)&zQ)&1`).
- **Explosión:** LL9 marca (`INWK+31` bit 7 → bit 5, `zxx19[1..6]` inicializados) y llama a `DOEXP` cada fotograma: borra la nube anterior (`PTCLS` en XOR), la hace crecer, la dibuja; al terminar `EX2` pone los bits 7 y 5 y `ships_step` quita la nave. Los colores del original son 4 del Apple II (azul, rojo, «fuzzy», otro): aquí cian, rojo, amarillo y blanco.
- **Contactos de prueba (provisional):** `N` (`ships_spawn`) crea una nave aleatoria delante (z_hi 2–5, velocidad = máximo/8) y `K` (`ships_explode_nearest`) pone el bit 7 de la más cercana. Se quitarán con los encuentros.

### 6.3 Gráficos y texto
- **Gráficos:** cada palabra son 4 píxeles; se escribe directamente en pantalla (`$20000` en el QL, memoria en el PC con el mismo formato; `tools/ql2png.py` la convierte a PNG). Vista espacial 256×136 centrada en (128, 68) como en el Apple II; el panel va en las líneas 136–191 (bitmap del original) y las filas extra del QL quedan para mensajes. XOR en todo lo animado (borrar = redibujar).
- **Rápido:** `gfx_fill_lines` (relleno de líneas completas con palabras largas desenrolladas) y `gfx_copy` (copia con palabras largas si las direcciones son pares). En el PC usan bytes (sin depender de reglas de alias); `c68_check` comprueba que la ruta de palabras largas de c68 da lo mismo.
- **Texto:** celdas 8×8 (32×32), tinta/papel/transparente, cursor XOR parpadeante; fuente 8×8 de dominio público (`gen_font.py`). El motor de textos comprimidos del original **no se porta**: los textos son cadenas C (`menu.c`, `universe.c`).

### 6.4 Resto de módulos (resumen técnico)
- **Universo** (`universe.c`): `uni_distance` sigue el algoritmo del 6502 (`dx²+(dy>>1)²`, `LL5`, ×4, con saturación), no el `sqrt` flotante de Text Elite (que solo coincide en el 14 % de los pares). El salto galáctico va al sistema más cercano a (0x60, 0x60). Radio del planeta de la carta local: `(w2hi AND 1) + 2 + C` con C el acarreo de `cpl`.
- **Cartas** (`chart.c`, `sun.c`): galáctica de 192 × 96 px con marco (x 32..223, y 19..115); local con 3 px/unidad en x y 1,5 en y centrada en (105, 75), solo sistemas a menos de 7,0 AL; círculos con `SUN`/`CIRCLE`; `HLOIN` = `gfx_hloin` (extremos a pares, cubre X1..X2−1).
- **Comercio** (`game.c`): reglas de `tnpr`; con `TRADE_FAITHFUL 1` (por defecto) se reproducen dos peculiaridades del original (ver 10.3).
- **Panel** (`dash.c`): `DIALS` completo con la caché de lo último dibujado (`dials`, `dialc`), `COMPAS`, misiles (`msbar`), bombillas S y E. **Energía y escudos fijos a 255**, temperaturas constantes (no hay combate ni sol que los muevan).
- **Polvo estelar** (`stars.c`): `STARS1/6/2` (frontal, trasera, laterales con `ST2`), `FLIP`, `NWSTARS`, `PIXEL2`.

## 7. Hardware QL

- **Pantalla (modo 8):** palabra = 4 píxeles; para el píxel k (0 a 3, izquierda a derecha) con `s = 2*(3-k)`: byte par con verde en el bit `s+1` y flash en el bit `s`; byte impar con rojo en el bit `s+1` y azul en el bit `s`. 128 bytes por línea. Colores: bit2 = verde, bit1 = rojo, bit0 = azul (`C_*` en `platform.h`).
- **Consola (c68):** `plat_init` redefine la ventana a pantalla completa, borde negro y sin cursor (SD.WDEF, SD.CLEAR, SD.CURS) y registra `plat_shutdown` con `atexit`; la cola de teclado de la consola se vacía con IO.FBYTE (timeout 0) al arrancar, en cada `plat_present()` y al salir (KEYROW no consume pulsaciones y SuperBASIC las recibía). `plat_exit()` limpia la consola y elimina el trabajo con `MT.FRJOB` (`ql_trap1`) para evitar «Press a key to exit». `plat_wait_key()` (esperar pulsación) **confirmado en el QL real**.
- **Teclado:** `KEYROW` por IPC (comando 9), una llamada por fila (0–7); bloque `MT.IPCOM` de 8 bytes `9,1,0,0,0,0,<fila>,2`, respuesta en D1.B. **Cada fila es una llamada lenta**: `key_ctx_menu()` lee las filas 1 y 7 siempre y el resto rotando (3 lecturas por fotograma); `key_ctx_flight()` lee las filas 1, 3 y 4 siempre y 0, 2, 5, 6 y 7 rotando (4 lecturas). Una tecla de fila rotante se detecta en como mucho 5 (vuelo) o 6 (menús) fotogramas. `KEYROW` lee el teclado físico aunque otra ventana tenga el foco; tres teclas en un rectángulo de la matriz dan una fantasma. **Joystick:** en el QL son teclas: CTL1 = cursores y espacio (fila 1), CTL2 = F1–F5 (fila 0). Disposición UK de los menús provisional; CTRL+Izq da el carácter 8.
- **Sonido:** `BEEP` por IPC, comando 10 con 10 parámetros, bloque de 17 bytes (bits por parámetro `$00000AAA`); `BEEP OFF` = comando 11; un `BEEP` nuevo cancela el anterior; no sonar mientras se carga o descarga un job. Pitch ≈ 0,022434·ciclos − 10,6; duración en unidades de ≈72 µs (otra nota apunta a ≈43,6 µs). Todos los efectos del original están traducidos desde los bucles del 6502, con prioridades; `snd_enable(0)` = DNOIZ. **Sin oír del todo:** ruido y barridos son aproximaciones (`SND_NOISE_FUZZ`, `SND_NOISE_RND` a oído; el signo de los barridos hacia arriba es hipótesis).
- **Trap #3:** `ql_trap3(d0,d1,d2,d3,a0,a1)` en `ql_ipc.asm` carga registros, ejecuta `trap #3` y devuelve D0 (0 ok, negativo error QDOS). Códigos: IO.FBYTE=1, SD.WDEF=$0D, SD.CURE=$0E, SD.CURS=$0F, SD.CLEAR=$20; de `qdos.h` solo se usan `fgetchid` y `mt_dmode`.
- **Disco:** ficheros QDOS desde C (`fopen`/`fwrite`): `flp1_`, `flp2_`, `mdv1_`, `mdv2_` (o directorio en el PC). Los datos van como `<unidad>elite_dash` (panel) y `<unidad>elite_<nombre>` (partidas). El juego silencia el sonido antes de tocar el disco.
- **Memoria:** pantalla en `$20000` (32 KB); `uni_galaxy[256]` ≈ 8 KB; K% 333 bytes; montones de naves 2 KB; dataspace total 18.114 bytes. Sin mapa de memoria del juego todavía; objetivo ≤ 640 KB.
- **Rendimiento:** el 68008 es lento y el bus es de 8 bits. LL9 en C solo se ha medido en PC. El borrado de la vista pasó de ≈35.000 llamadas a `gfx_pixel` a 544 iteraciones de 8 escrituras largas. Si hace falta más: rutina de ensamblador con `movem.l` para el relleno, y `LOIN`/`LL9` en ensamblador tras perfilar.

## 8. Controles

- **Vuelo:** cursores = alabeo (izquierda/derecha) y cabeceo (arriba = picar, abajo = subir; también el joystick CTL1); **`F` acelera, `D` frena**; `A` o espacio láser; `B` bomba, `ESC` cápsula, `T`/`U`/`M` misil, `E` ECM, `J` salto, `C` y `P` atraque (los de equipo aún sin función); **`1`–`4` vistas** (frontal, trasera, izquierda, derecha); **`5`–`9` pantallas** (carta local, datos del sistema, mercado, estado, inventario) y `0` carta galáctica; `H` hiperespacio (requiere sistema objetivo en alcance), `G` salto galáctico, `C` atraque (sin exigir llegar a la estación); `N` nave de prueba y `K` explotar la más cercana (provisionales).
- **Atracado:** **`U` despega (undock)**, `0` carta galáctica, `1` comprar, `2` vender, `3` equipo, `4` carta galáctica, `5`–`9` como en vuelo, `I` disco. `/`, `,`, `.`, `S` y `X` ya no se usan.
- **Cartas:** cursores mueven la mira, `F` busca sistema (solo atracado, `HME2`), `D` = `hm`, `O` = `ping`.
- Implementado en `ctl_read()` (keys.c) y `ui_nav()` (menu.c).

## 9. Formatos de datos

- **Comandante** (`cmdr.c`): bloque de 77 bytes (#0..#76), desplazamientos `CB_*`; `cmdr_pack/unpack`, `cmdr_default` (JAMESON, Lave; constantes de fábrica `$03`, `$27`, `$AA`), sellado como `SVE` (`CHK3`, `CHK`, `CHK2 = CHK ^ $A9`). `CHECK`/`CHECK2` del original solo detectan el 57 % de los cambios de un bit (no es un fallo del port). Dinero en big-endian (décimas de crédito).
- **Fichero de partida** (`save.c`): propio, **94 bytes**: `ELQD`, versión, semilla, nombre de 8, bloque de 77 cifrado con XOR. Nombre `<unidad>elite_<nombre>`. `load_commander` valida `CHK` y `CHK3` y marca `cok` bit 7 si falla solo `CHK2`, como `DFAULT`. (El cifrado `MUTILATE` del original no se porta: es antimanipulación.)
- **`elite_dash`:** cabecera `ELQD` + 56 líneas de 128 bytes en modo 8 (violeta → magenta, naranja → rojo), generado desde `A.SCREEN.bin`; se lee **una sola vez** y se guarda en `dash_bg[]`. Sin el fichero se dibuja un panel mínimo.
- **Naves** (`shipdata.c`): 32 planos (cabecera de 20 bytes, vértices de 6, aristas de 4, caras de 4) en un bloque `ship_blob` de 7.154 bytes (algunos planos comparten aristas/caras por desplazamientos de 16 bits); `SHIP_BP(tipo)`; `SHIPT_*` con los nombres de la fuente (`SHIPT_CORIOLIS=2`, `SHIPT_DODO=32`...). Bytes de la cabecera usados: 5 (tamaño del montón), 7 (explosión), 13 (distancia de dibujo completo), 14 (energía), 15 (velocidad máxima), 19 (banderas de línea).

## 10. Trampas, reglas y hallazgos

### 10.1 Compilador (c68)
- **Fallo del optimizador `-O` (2026-10-16):** en `x = (u8)d; cc = (d >= 0);` c68 usa los flags del `move.b` para el `sge` en lugar de los de la palabra `d`; da error cuando el byte sale positivo con `d` negativo (en el QL: naves 3 veces más anchas o irreconocibles). **Regla: calcular siempre el acarreo/signo ANTES de guardar el byte** (`cc = (d >= 0); x = (u8)d;`). Aplicado en 25 sitios de `ship3d.c` y `ship_math.c`; en el código nuevo (`move.c`, `ships.c`) también.
- `int` de 16 bits: la tabla de senos de `shipview.c` es `long` (el 32768 se desbordaba).
- `qcc` busca los `#include "..."` en el directorio actual: siempre `-Isrc`.
- Sin `float`; sin `stdint.h`; ficheros de ensamblador `.asm`.

### 10.2 Entorno del QL
- IPC: lectura de filas lenta (leer solo las necesarias); un `BEEP` nuevo cancela el anterior; no sonar durante cargas de jobs.
- La consola de c68 borra la pantalla al salir y escribe «Press a key to exit» (se evita con `MT.FRJOB`, **sin comprobar en el QL**).

### 10.3 Hallazgos del original que se reproducen
- `tnpr` suma en **8 bits**: con 193 o más unidades de un artículo en kg/g, comprar (256 − las que llevas) da la vuelta y se acepta. `GCASH` calcula el importe en **16 bits**: una venta de más de 6.553 Cr de una vez pierde los bits altos (p. ej. 200 kg de platino). Se reproducen con `TRADE_FAITHFUL 1` en `game.h`; con 0 se corrigen.
- El original tiene un bug de tabla (`RUGAL`) que cuelga la pantalla de datos del sistema en algunos sistemas durante la misión del Constrictor; no se reproduce.
- **`WPLS2`** necesita `LSY2[0] = $FF` como centinela (el original no lo inicializa a la vista); sin él dibuja una línea basura al borrar el planeta.
- `LL52` negaba con `1-x` en vez de `1-x-1` en la primera transcripción (corregido; lo encontró la verificación).
- `CHECK`/`CHECK2` detectan solo el 57 % de los cambios de un bit.

### 10.4 Reglas de oro
- **Dibujo en XOR:** borrar = volver a dibujar lo mismo (mismos puntos en el mismo orden). Si se limpia una zona sin avisar al montón de líneas (naves, sol, círculo), hay que **olvidar** esas líneas.
- **Una `u8` truncada no se compara:** calcular el signo antes (10.1).
- **Acarreos como parámetros:** modelarlos explícitamente; ante la duda, mirar qué instrucción los consume (`ADC`/`SBC`/`ROL`/`BCS`).
- **Datos de naves** se extraen con script, no a mano.
- **Entregar siempre** el tar + los tres `.md` al final de cada paso, y comprobar el tar en un directorio limpio.

## 11. Inventario de rutinas

`tools/ported.txt` + `tools/inventory.py` generan `INVENTARIO.md` (528 rutinas de la fuente, agrupadas por categoría de la propia fuente). **Estados:** `verificada` (comparada con el 6502), `transcripcion` (con una transcripción literal propia), `sin-verificar` (portada, en `PENDIENTES.md`), `propia` (equivalente propio con otra forma), `parcial` (versión reducida; cuenta como pendiente), `no-necesaria`, vacío = sin portar.

**Resumen a 2026-10-17:** 528 rutinas = **334 portadas** (25 verificadas + 309 pendientes de verificar) + **140 no necesarias** + **54 sin portar** (tabla por áreas al principio de `INVENTARIO.md`).

**Criterio de «no necesaria»:**
- *Anticopia y arranque del Apple II:* `ENTRY`, `S%`, `DEEOR`, `DEEORS`, `G%`, `F%` (suma de comprobación y descifrado del código), `NMIpissoff`, `COLD`, `BEGIN`, `MUTILATE`/`UNMUTILATE` (cifrado antimanipulación del fichero de comandante; `save.c` tiene su propio cifrado), `SWAPPZERO`.
- *Lo que da el compilador/runtime de C:* categoría **Utility routines** (rellenos de memoria, copias de páginas, retardos por bucles, manejador de errores `BRK`), **Workspaces** (`ZP`, `XX3`, `K%`, `WP`, `Disk`, `UP`, `Option`: son variables de C) y **Tube** (`newosrdch`).
- *Hardware del Apple II/BBC/C64:* tablas de la pantalla hires (`SCTB*`, `cellocl`, `TWFL/TWFR`, `MASKT`), cambio de modo (`HGR`, `TEXT`), impresión en modo texto (`CHPR2`, `R5`, `RR5`), altavoz (`CLICK`), registrador de teclas, sprites de los Trumbles (VIC).
- *DOS 3.3 / RWTS:* catálogo, sectores, pistas, nibbles, brazo del disco (QDOS gestiona el disco).
- *Motor de textos comprimidos* (`DETOK*`, `MT*`, `TKN2`, `QQ16`, `DTW*`, `TT27`, ...): el port usa cadenas C; los textos de las misiones se escribirán como cadenas.
- *Sin uso en el original* (`MUT3`, `LASNOISE2`, `beamcol`, `DKSANYKEY`, `KTRAN`).

## 12. Qué falta (plan)

**Paso 3 — combate, IA y encuentros** (los 54 sin portar; en orden recomendado). Rutinas del original entre paréntesis:
1. **Bucle principal completo** (`Main`, parcial): las partes 1–6 del bucle principal que faltan: colisiones nave-nave y con la estación, escarbado de cargas, láser (`LASLI`, `HITCH`, energía del láser `LAS`, calor), misil propio (`FRMIS`, `FR1`), muerte de naves con puntos (`EXNO2`, tabla `KWL%`/`KWH%` → `TALLY`, mensajes), generación aleatoria de encuentros (policía, piratas, comerciantes, cazarrecompensas, asteroides; usa `NWSHP` ya portado), `GTHG` (Thargoides), `SPIN`/`SFS1` (cargas al destruir), `Ze` (nave agresiva), `MJP` (misjump).
2. **IA** (`TACTICS`, 7 partes; `ANGRY`, `FRS1`, `SFRMIS`, `KS2`, `HITCH`): conectar `move_tactics_hook`. Es el bloque más grande: misiles enemigos, estación, cápsula, Thargon solitario, policía, huida, ataque; usa MAS1–4, TAS1–6, SPS1–4 (ya verificados).
3. **Daños y energía** (`OOPS`, `SHD`, `DENGY`, `OUCH`, `ou2`, `ou3`, `mes9`): escudos, energía, pérdida de equipo; `dash.c` ya pinta los bancos, hoy fijos a 255; temperaturas del láser y de cabina.
4. **Atraque real** (`DOENTRY` parcial, `DOCKIT`, `DCS1`, `dockEd`): ordenador de atraque, orientación de la estación (`SPS`), entrada por la ranura; sustituye al atraque con `C`. Colocar la estación como `SOLAR`/`NWSPS` (hoy la posición es propia).
5. **Equipo y emergencias:** ECM (`ecmp`, bombilla ya hecha), bomba de energía (`BOMBON/OFF/EFF2`, `BOMBPOS/TBX/TBY`), cápsula de escape (`ESCAPE`, `SESCP`), salto local `WARP` (tecla `J`), interruptores de configuración (`TGINT`, `DKS3`: sonido, etc.).
6. **Muerte y reinicio** (`DEATH`, `DEATH2`, `DET1`).
7. **Misiones** (`BRIEF`, `BRIEF2`, `BRIEF3`, `DEBRIEF`, `DEBRIEF2`, `TBRIEF`, `BRIS`, `BRP`, `PAUSE`, `PAS1`, `THERE`, `MT27`, `MT28`, texto de `RUTOK`/`TKN1` como cadenas C): Constrictor, Thargoides, Trumbles; directiva de la misión 1 en `PDESC`.
8. **Quitar** `N` y `K` de prueba cuando haya encuentros.

**Verificación y puesta a punto:**
- **En el QL real (prioridad máxima cuando el usuario pueda):** arrancar `elite` (qcc ya lo compila), comprobar naves, planeta, sol, estación, explosiones, borrado rápido (cambio de vista, fin del túnel, hiperespacio), teclas `U`/`F`/`D`, `plat_exit`, `ql_trap3`, y rendimiento por fotograma con varias naves.
- **Contra el 6502 (`PENDIENTES.md`, prioridad A):** `MVEIT` y compañía, `DOEXP`/`PTCLS`/`EXS1`, `PLANET`, `SOLAR`, polvo estelar, radar, `DIALS`/`COMPAS`, cartas, controles. Sirve el patrón de `tools/gen_trade.py`.
- **Rendimiento:** perfilar y pasar a ensamblador lo que haga falta (relleno de pantalla con `movem.l`, `LOIN`, `LL9`).
- **Ajustar a oído:** parámetros de sonido (ruido y barridos) y disposición del teclado UK.
- **Colores:** los 4 del polvo de la explosión (hoy cian, rojo, amarillo, blanco) y los de la mira y el túnel se pueden ajustar.

## 13. Decisiones y desviaciones del original

- **Port fiel con partes propias donde el hardware lo exige:** `LOIN` es `gfx_line` (no la transcripción); `HLOIN` = `gfx_hloin`; el panel usa el bitmap original convertido a modo 8; el fichero de partida es propio (ELQD, 94 bytes).
- **Montones de líneas** por ranura (256 bytes) en lugar del reparto con `SLSP`; `K%` con 9 ranuras.
- **Estación al despegar:** posición propia (detrás y algo por debajo); **regla de salida** propia (z_hi > 0x30 la quita y devuelve el sol); `SOLAR` y `NWSPS` parciales.
- **Hiperespacio sin encuentros, atraque sin llegar a la estación, contactos de prueba con `N`/`K`**: provisionales hasta el paso 3.
- **Universo y mercado** vienen de Text Elite (verificado contra su oráculo, no contra el 6502); `uni_distance` sigue el 6502.
- **Textos como cadenas C**, sin el motor de tokens.
- **Sonido** traducido desde los bucles del 6502 a `BEEP` del QL: aproximado.
- **`TRADE_FAITHFUL 1`**: se reproducen dos peculiaridades aritméticas del original.
- **Teclas** repartidas para el teclado del QL (sección 8), distintas de las del Apple II.

## 14. Historial

- **2026-10-16 (primeras rondas):** capa gráfica, texto, aritmética verificada, teclado y sonido; estado y aritmética con estado; datos de naves; LL9 y recorte verificados contra el 6502 (`make test_orig`); universo (Text Elite); consola del runtime de c68; demos para el QL; **estrategia de portar todo sin verificar**; comandante, guardar/cargar, lógica de juego, entrada, pantallas, bucle título → atracado ↔ vuelo.
- **2026-10-16 (tras el QL):** título animado (el estado de la nave por ranura, bit 3 de `INWK+31`); comercio verificado contra el 6502 (hallazgos `tnpr`/`GCASH`); **bug de c68 `-O`** (naves 3, 4 y título; corregido con `c68_check`); `plat_exit`; `SUN`/`CIRCLE`/cartas (`TT22`, `TT23`...); marco de la galáctica; cursores y lectura de teclado por contextos; `hm`, `ping`, `HME2`.
- **2026-10-16/17 (panel y vuelo):** panel con el bitmap original (`elite_dash` cargado una vez), `DIALS`, `COMPAS`, misiles y bombillas; giros reales (`ALPHA`/`BETA`, `MV40`); **quinta ronda:** polvo estelar real (`STARS1/6/2`), radar (`SCAN`), mira (`SIGHT`), túnel (`HFS2`/`HFS1`), `VLOIN`/`CPIX`.
- **2026-10-17 sexta ronda:** `F` acelera y `D` frena; atracado `U` despega y `5`–`9` = vuelo; **borrado rápido de pantalla** (`gfx_fill_lines`, `gfx_copy`, sondas `gfxfast`; `-U__GNUC__` en `c68_check`).
- **2026-10-17 séptima ronda — paso 2 «Proyección 3D» terminado (sin verificar):** `MVEIT` completo, `MV40`/`MV45`/`MVS4`/`MVS5`/`MVT1`/`MVT6`/`PLUT` (`move.c`); `ships.c` (lista de naves, `ZINF`, `NWSHP`, `KILLSHP`, `SOLAR`, `NWSPS`, bucle por nave); **`DOEXP`/`PTCLS`/`EXS1`/`EX2`**; **`PLANET`** (`PL9`, `PL2`, `WP1`); el vuelo dibuja naves, planeta, sol, estación y explosiones en las 4 vistas; hallazgo de `LSY2[0] = $FF`; `make test_space` 694 comprobaciones; sondas `flight` y `session` ampliadas.
- **2026-10-17 octava ronda:** **cadena de qcc instalada en el sandbox** (`tools/install_xtc68.sh`: XTC68 + runtime c68 en `support/424frun1`) y **`make ql_game` compila y enlaza** (`elite`, ~96 KB, dataspace 18.114); **revisión completa del inventario** (528 rutinas: 334 portadas, 140 no necesarias, 54 sin portar; tabla por áreas; criterios de «no necesaria»: anticopia, Tube, Utility routines, Workspaces, RWTS, tablas de HGR, motor de textos); este `Proyecto.md` reescrito como memoria del proyecto.

## 15. Referencias

- Fuente de referencia: https://github.com/markmoxon/elite-source-code-apple-ii y la web https://elite.bbcelite.com/apple/ (código documentado de Mark Moxon).
- XTC68 (compilador cruzado de C68): https://github.com/stronnag/xtc68 (migrado a https://codeberg.org/stronnag/xtc68); runtime c68: https://github.com/xXorAa/c68-support (cabeceras `INCLUDE_*` y librerías `LIB_*`; en la carpeta `support/424frun1` del repositorio xtc68). Otros runtimes: https://dilwyn.qlforums.co.uk/c/.
- Emuladores del QL para probar: QPC2, uQLx, sQLux. Manual de SuperBASIC para la matriz de `KEYROW`.
- Oráculo del universo: `tests/oracle/` (`TXTELITE.C` 1.5, Ian Bell).
- Conversación original con las sesiones completas: transcripción `/mnt/transcripts/2026-10-17-08-06-48-elite-ql-port.txt` (solo accesible dentro de la sesión que la generó; este documento la sustituye).
