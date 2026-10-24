# Elite (Apple II) → Sinclair QL — memoria del proyecto

Este documento es la **memoria completa del proyecto**: contiene todo lo necesario para continuar el trabajo desde cero, en otra sesión, otra cuenta u otro LLM. Se actualiza al final de cada paso, junto con `PENDIENTES.md` e `INVENTARIO.md`, y si no se esta usando claude.code entrega con el código (`elite_ql_base.tar.gz`).

La LLM actua como experto en el ordenador Sinclair QL, el ordenador apple II, programacion en C y Ensamblador de los motorola 6502 y 68000. 

Índice: 1 Resumen · 2 Cómo trabajar · 3 Entorno y herramientas · 4 Estructura del repositorio · 5 Pruebas y verificación · 6 Arquitectura técnica · 7 Hardware QL · 8 Trampas, reglas y hallazgos · 9 Decisiones y desviaciones del original

---

## 1. Resumen

- **Objetivo:**
   -Port de **Elite, versión Apple II (Firebird, 1985)** con aritmética de enteros de 8 bits.
   -Destino el **Sinclair QL** (68008 a 7,5 MHz, bus de 8 bits, sin FPU, hasta 640 KB de RAM, **modo 8: 256×256, 8 colores**), en **C89** con **qcc (XTC68/c68)** y optimizaciones en ensamblador `as68`. Versión de disco. 

- **Fuentes de referencia:** solo para verificar no son parte del proyecto: 
   - Carpeta `ref/moxon-appleII`, El código documentado de Mark Moxon, obtenido de https://github.com/markmoxon/elite-source-code-apple-ii . Ficheros: `1-source-files/main-sources/elite-source.asm` (~47.000 líneas, 528 rutinas con cabecera `;       Name: X`) y `elite-data.asm`. **No tiene licencia**: se usa solo como referencia; el código del port es propio y no copia sus comentarios. El juego original es de Ian Bell y David Braben.
   - Carpeta 'ref/moxon-bbc-a', El código documentado de Mark Moxon, obtenido del github https://github.com/markmoxon/elite-a-source-code-bbc-micro
   - Archivo `ref/TXTELITE.C` Version 1.5 del motor solo texto de Ian Bell.
   - Emuladores del QL para probar: QPC2, uQLx, sQLux. Manual de SuperBASIC para la matriz de `KEYROW`.

- **Dependencias:**
   -`python3` (con `Pillow` para `tools/ql2png.py`), `gcc`, `make`, `git`, `meson`, `ninja`; para `make c68_check`: `qemu-user` (`qemu-m68k`) y `gcc-m68k-linux-gnu` (`m68k-linux-gnu-as`, `-ld`).
   - XTC68 (compilador cruzado de C68): disponible en local, de no ser asi las fuentes estan en https://github.com/stronnag/xtc68 (migrado a https://codeberg.org/stronnag/xtc68); necesita elruntime c68: https://github.com/xXorAa/c68-support (cabeceras `INCLUDE_*` y librerías `LIB_*`; en la carpeta `support/424frun1` del repositorio xtc68). 
   
- **Estrategia** 
   - Se identifican las rutinas que no pueden tener equivalencia en el QL, por ser dependientes del hardware y se crea un "HAL" con rutinas que no se comprobaran contra el 6502. 
   - Se porta **todas las demas rutinas** del juego a C **sin** verificar cada rutina contra el original mientras tanto; cada función nueva se anota en `PENDIENTES.md` (rutina de origen y cómo verificarla) y en `tools/ported.txt`.
   - Cuando el juego arranque en el QL real, se verifica la lista contra el emulador de 6502 y el feedback del QL real reordena las prioridades. Lo ya verificado se mantiene.
   - Se realiza un profiling y se determina que rutinas deben pasarse a ensamblador para mejorar el rendimiento, en este paso tambien se modifica el comportamiento y se añaden nuevas funcionalidades.

## 2. Cómo trabajar

- las herramientas (`tools/elitesrc.py`, `inventory.py`, `gen_dashboard.py`, `gen_move.py`) usan las rutas por defecto de las referencias, así que no hace falta exportar nada; si la fuente está en otro sitio, `export ELITE_SRC=<ruta>/1-source-files/main-sources`. Sin la fuente funcionan todos los tests salvo los que comparan con el 6502 original (`make test_orig`, `test_trade`, `test_laser`, `test_missions`...) y `tools/inventory.py`. Sin la fuente funcionan todos los tests salvo `make test_orig`, `make test_trade` y `tools/inventory.py`.
- **Línea base:** `make test_all` (y `make c68_check` si hay toolchain, `make test_orig`, `make test_trade` y `make test_laser`, `make test_damage`, `make test_planet`, `make test_equip`, `make test_tactics` y `make test_missions` si está la fuente) debe dar 0 fallos antes de tocar nada.
- **Compilador del QL:**    `make elite` (el `Makefile` ya pone solo esas variables si existe `/tmp/qlsdk`). **Para probar en el QL o en un emulador: `make elite.img`** (genera `build/elite.img`: disco DS/DD de 720 KB con `elite`, `boot`, `elite_scr` y `elite_dash`).
- **Flujo de trabajo por rutina**, **entrega al final de cada paso**: `elite_ql_base.tar.gz` (código + tests + docs, sin binarios), `CLAUDE.md`, `PENDIENTES.md`, `IN  VENTARIO.md` (y la carpeta data/). Verificar el tar desempaquetándolo en un directorio limpio y pasando los tests.

### 2.1 Flujo de trabajo para portar una rutina

1. Leer la rutina en `elite-source.asm` (buscar `;       Name: X`; las rutinas multiparte se llaman `X (Part n of m)`). Extraer el código sin comentarios: `sed -n A,Bp elite-source.asm | sed 's/;.*//' | grep -v '^\s*$'`.
2. Escribir el C **transcribiendo el flujo** (con `goto` si hace falta, incluidos los saltos al medio de rutinas y las caídas de una rutina en otra) y **modelando los acarreos** que usa el 6502 (`em_carry`, `em_rand_c`, variables `c`/`cc` locales). Si una instrucción depende del acarreo con que vuelve una subrutina, hay que calcular ese acarreo (ejemplo: el de `MLTU2` es el bit bajo del producto).
3. Si usa `int d` con comparaciones: **calcular el signo/acarreo ANTES de truncar a `u8`** (bug de c68 `-O`, sección 10).
4. Añadir la línea a `tools/ported.txt` (`NOMBRE funcion_C[,otra] estado nota`; estados: `verificada | transcripcion | sin-verificar | propia | parcial | no-necesaria`) y la fila a `PENDIENTES.md`; ejecutar `python3 tools/inventory.py`.
5. Pruebas propias en `tests/` (probar que el C hace lo que se quiere) y, si procede, una **sonda c68** en `tests/c68/probe_*.c` (añadirla a la lista de `tools/c68_check/run.sh`) para comprobar que c68 compila lo mismo que gcc.
6. Verificar contra el 6502 cuando haya ocasión (sección 5): ejecutar la rutina original en el emulador con entradas aleatorias y valores límite, comparar, probar con mutaciones de una instrucción, y pasar la fila a `verificada`.

## 3. Entorno y herramientas

### 3.2 Compilador de C para el QL (XTC68) — instalación si no esta disponible.
`tools/install_xtc68.sh` automatiza lo que sigue:
1. `git clone https://github.com/stronnag/xtc68` (migrado a Codeberg, pero el clon de GitHub sirve) y `git clone https://github.com/xXorAa/c68-support`.
2. `tools/install_xtc68.sh` instala XTC68 + runtime c68 en `/tmp/qlsdk` (ver 3.2). Luego `export PATH=/tmp/qlsdk/bin:$PATH QLINC=/tmp/qlsdk/share/qdos/include QLLIB=/tmp/qlsdk/share/qdos/lib` y
3. gcc moderno falla con `-Werror` en las fuentes antiguas (p. ej. «ignoring return value of write»): `sed -i "s/'-Werror', *//g" src/*/meson.build`.
4. `meson setup build --prefix=/tmp/qlsdk --strip && ninja -C build install` → `bin/{qcc,c68,as68,qld,qcpp,qdos-ar,...}`.
5. El runtime de c68 (cabeceras `INCLUDE_*` y librerías `LIB_*`, del c68 runtime 4.24 «424frun1») va en una carpeta **`support/424frun1`** del repositorio xtc68, y se instala con `./sdk-install.sh /tmp/qlsdk` ejecutado **desde esa carpeta** → `share/qdos/include` y `share/qdos/lib`.
6. Uso: `export PATH=/tmp/qlsdk/bin:$PATH QLINC=/tmp/qlsdk/share/qdos/include QLLIB=/tmp/qlsdk/share/qdos/lib`; `qcc -O -Isrc -o elite ...` (el `Makefile` ya lo hace con `make elite`). `-Isrc` es obligatorio porque qcc busca los `#include "..."` en el directorio actual. `qcc` imprime `elite: dataspace 18114 (46c2)` al terminar bien.
7. `as68` es de la familia Sozobon: directivas `.globl`/`.text`, comentarios con `;`, etiquetas con dos puntos, ficheros `.asm` (qcc no reconoce `.s`); no valen `xdef`, `section` ni comentarios con `*`.

### 3.3 Verificación de código de c68 (`make c68_check`)
`tools/c68_check/run.sh [sonda ...]` compila cada sonda de `tests/c68/` con gcc (PC) y con el `c68` real (`-int=16 -O -warn=0`, preprocesando con `gcc -E -P -U__GNUC__ -nostdinc`), convierte la salida de c68 (sintaxis ACK) a GNU as con `conv.py`, la enlaza con un mini runtime (`rt.s`: `.Xlmul`, `.Xldiv`, ..., salida de pantalla) y la ejecuta bajo `qemu-m68k`. Cada sonda deja hashes en la «pantalla» y se comparan byte a byte. `XTC68=<dir con c68>` (por defecto `/tmp/qlsdk/bin` si existe, si no `/tmp/xtc68/build`). Con `-U__GNUC__` se compilan las rutas «de QL» (por ejemplo las palabras largas de `gfx_fill_lines`). Sondas: `gfxfast, emath, keys, ships, game, charts, dash, flight, session`. **No sustituye al QL real** (no cubre el runtime ni el hardware), pero detecta fallos del compilador (encontró el de `-O`).

### 3.4 Herramientas Python (`tools/`)
| Fichero | Para qué |
|---|---|
| `cpu6502.py`, `asm6502.py` | emulador de 6502 y ensamblador mínimo de la sintaxis de la fuente (etiquetas, EQUB/EQUW, `P%`, `LO()/HI()`, `%bin`, `$hex`, `IF/ELIF`) |
| `orig.py`, `elitesrc.py` | monta una imagen con rutinas originales (LL9, SHPPT, LL51, PROJ, PLS6, DVID3B2, FMLTU, DORND; LOIN/DOEXP/PLANET son stubs que registran la llamada) y lee la fuente (símbolos) |
| `frag6502.py`, `gen_laser.py`, `gen_damage.py`, `gen_planet.py`, `gen_equip.py` | ensamblan tramos sueltos de la fuente (por etiqueta) con stubs que registran, y generan `tests/vectors_{laser,damage,planet,equip}.txt` ejecutando el original (`gen_planet.py` ejecuta la parte de `LOIN` que importa —hasta `LI3`/`LI15`— y registra la línea ya ordenada) |
| `gen_ll9.py` | genera `tests/vectors_ll9.txt` ejecutando el original con entradas aleatorias |
| `gen_geom.py`, `gen_lines.py`, `gen_draw.py` | (16.ª ronda) Maths Geometry con las entradas de `test_state`; `LOIN`/`HLOIN`/`VLOIN` en la pantalla hires del emulador decodificada a coordenadas del juego; `TT15`, `SIGHT`, `NLIN2/4`, `BOMBEFF2`, `DOEXP`, `scacol` con primitivas que registran → `tests/vectors_{geom,lines,draw}.txt` |
| `ref6502.py`, `ref6502b.py`, `ref6502_state.py` | **transcripciones literales** del 6502 en Python (con acarreos explícitos) de la aritmética y de las rutinas con estado |
| `gen_vectors*.py`, `gen_trade.py` | generan los vectores de `make test`, `test_state`, `test_save` y `test_trade` |
| `gen_shipdata.py` | extrae los 32 planos de naves de `elite-data.asm` → `src/shipdata.c/.h` |
| `gen_dashboard.py` | `A.SCREEN.bin` (volcado hires del Apple II) → `data/elite_dash` |
| `png2dash.py` | cabina editable: PNG de 8 colores (cualquier múltiplo de 256 × 56) → `data/elite_dash`, y `--export` del panel a PNG (por defecto 1024 × 112, píxeles de 4 × 2 con la proporción del QL); `make dash_png`, `make dash_from_png` (con `asset/cabina.png`), `make test_dashpng` (ida y vuelta) |
| `gen_font.py`, `gen_tables.py`, `gen_sne.py`, `gen_universe_text.py`, `make_oracle.py` | fuente 8×8, tablas log/seno, tabla de seno de naves, textos del universo, oráculo de Text Elite |
| `ql2png.py` | convierte una captura de pantalla QL (32 KB) a PNG |
| `inventory.py`, `ported.txt` | generan `INVENTARIO.md` (ver 11) |
| `m68k_mini.py`, `test_lineasm.py`, `test_ellasm.py`, `test_ll9asm.py`, `gen_lltab.py` | intérprete mínimo de 68000 y pruebas del ensamblador propio (`ql_line`, `ql_ell`, `ql_ll9`) contra el C; `gen_lltab.py` genera `src/ql_lltab.asm` |
| `install_xtc68.sh`, `c68_check/` | cadena de qcc y verificación de c68 |
| `tmp/qltools_src/` (se crea sola) | clon y compilación de **qltools** de N. Dunbar (`make qltools`); formatea y escribe la imagen `elite.img` (3.5). No se incluye en el tar |

### 3.5 Imagen de disco `elite.img` (qltools)
`make elite.img` (alias `make disk`) hace todo de una vez y es repetible:
- **`elite`** (objetivo real; el alias `ql_game` se eliminó el 2026-10-24): compila con qcc si cambió algún `src/*.c`, `src/*.h` o `.asm`. qcc deja al final del fichero el trailer `XTcc` + dataspace (big-endian; hoy 16.840 bytes); qltools lo detecta al escribir y marca el fichero como ejecutable con ese dataspace (`E` en el directorio). Hace falta el `-W` de qltools, no copiar el fichero a mano.
- **`$(QLTOOLS)`** si no existe qltools en local se puede instalar y compilar en `tmp/qltools_src/` con  `git clone --depth 1 https://github.com/NormanDunbar/qltools tmp/qltools_src` y `make -C tmp/qltools_src/Unix` (lo hace `make qltools`). 
- Todo lo generado va a `build/` (`build/elite`, `build/elite.img`). `rm -f build/elite.img`; `qltools build/elite.img -fdd ELITE` (formatea DS/DD: 2 caras, 80 cilindros, 9 sectores por pista = 1.440 sectores = 737.280 bytes; ID `QL5A`); `qltools build/elite.img -W build/elite data/boot data/elite_scr data/elite_dash` (`-W` sobrescribe sin preguntar; los nombres en el disco son los del fichero sin el directorio); `qltools build/elite.img -d` lista el resultado.
- **Notas de qltools:** traduce `.` en `_` en los nombres (sin `-t`); ningún nombre del disco lleva punto. Muestra el año de la fecha de los ficheros como 126 (error solo de presentación). Los argumentos `-fdd` y `-W` van **antes** de la lista de ficheros; para comprobar un fichero con nombre sin traducir se usa `-t -n nombre`.
- En un emulador (sQLux, QPC2, uQLx): montar `build/elite.img` como `flp1_` y arrancar con `LRUN flp1_boot` (o dejar que lo ejecute el arranque del disco).

### 3.6 Imagen de microdrive `elite.mdv` (mdvtool)
- **`$(MDVTOOL)`** si no existe mdvtool en local se puede instalar y compilar en `tmp/` desde el repositorio https://github.com/xxoraa/mdvtool (`make mdvtool`; usa el del `PATH` si lo hay; necesita libzip).
- `make elite.mdv` genera `build/elite.mdv` (formato QLAY, etiqueta `ELITE`) con `boot`, `elite`, `elite_scr` y `elite_dash`. Copia los ficheros a `build/mdv/` e importa desde allí (mdvtool guarda el nombre **tal como se le pasa**, con directorio incluido). El `boot` del microdrive es el del disco con `dev$="mdv1_"` (sed). mdvtool detecta el trailer `XTcc` y marca `elite` como EXEC con su dataspace.
- **Capacidad:** 255 sectores de 512 B; quitando el mapa y el directorio quedan **253** para ficheros, y cada fichero lleva 64 B de cabecera (`MDV_SECTORS`). Con la imagen llena mdvtool termina con violación de segmento, así que la regla comprueba antes los sectores y falla con un mensaje. **Hoy el ejecutable (~147 KB, 146 KB sin `DEBUG`) no cabe** (hacen falta 369 sectores): por eso `elite.mdv` no está en el objetivo por defecto. Un cartucho real suele dar menos sectores que la imagen QLAY.



## 4. Estructura del repositorio

```
Makefile              objetivos de compilación y pruebas (sección 5)
PLAN.md               documentos donde se almacena el historico de cambios
PENDIENTES.md         documentos donde se almacena el avance del port y las verificaciones contra el original.
asset/                contiene archivos externos que se usan para generar datos del programa.
data/                 contiene archivos que forman parte de la distribucion final del juego, en la imagen de disco:
                      elite_dash (fondo del panel, 56 líneas × 128 B, modo 8), elite_scr (pantalla de arranque, 32 KB), boot (arranque SuperBASIC) y README.txt (este no forma parte de la distribucion)
build/                contiene los ejecutables y archivos de codigo intermedios generados 
src/                  código del juego (≈8.300 líneas)
tests/                pruebas (test_*.c), c68/ (sondas), oracle/ (adaptacion de Text Elite), shots/ (capturas), vectors*.txt
tools/                herramientas Python y c68_check
ref/                  codigos fuente para usar como referencias
tmp/                  archivo temporal con compilaciones de utilidades de terceros, qltools, xtc68 y otros iran aqui si no 
                      estan disponibles en local
```

## 5. Pruebas y verificación

### 5.3 Cómo verificar una rutina nueva contra el original
`tools/orig.py` monta una imagen del 6502 con las rutinas originales que se pidan y stubs para las que dibujan; `tools/gen_laser.py` y `tools/gen_damage.py` son los ejemplos más cómodos; usan `tools/frag6502.py` (su `build(lineas, None, presets)` convierte en stubs que registran (nombre, A, X, Y) todo lo que no se resuelve, y pone un RTS local cuando una rama no llega al stub; ensambla tramos sueltos del bucle principal por etiqueta: `.MA68`..`.MA3`, `.MA47`..`.MA8`...; `presets` fija direcciones de tablas como `log`; si una rama no llega a una etiqueta de fuera, hay que ponerle un `RTS` local delante, p. ej. `['.out', 'RTS'] + OUCH`). Cuidado: un tramo que «cae» en la rutina siguiente (como `Ze` en `DORND2`) necesita esa rutina justo detrás en la lista; `tools/gen_trade.py` (preprocesa `IF/ELIF`, sustituye pantalla por stubs, ejecuta `tnpr` y los tramos de `TT219`/`TT208`). Pasos: volcar el estado inicial aleatorio en el emulador y en C, ejecutar, comparar INWK/variables/líneas dibujadas, repetir con valores límite, mutar una instrucción del C y comprobar que el test falla.

## 6. Arquitectura técnica

### 6.1 Convenciones del código generales
- **C89 puro** (sin `stdint.h`, sin declaraciones mezcladas ni en `for`); `u8 = unsigned char`; **`int`: el `qcc` del `Makefile` (sin `-int=16`) lo compila de 32 BITS (comprobado con `qcc -S`: `.data4`); `make c68_check` usa `-int=16`**: no depender del tamano (usar `long` para valores > 32767 y para todo lo que lea el ensamblador) para valores > 32767 (`SCR_BYTES = 32768L`) y no confiar en promociones.
- `qcc` busca los `#include "..."` en el directorio actual: siempre `-Isrc`.
- Sin `float`; sin `stdint.h`; ficheros de ensamblador `.asm`.


## 7. Hardware QL

- **Memoria:** pantalla en `$20000` (32 KB); objetivo ≤ 640 KB.
- **Pantalla (modo 8):** palabra = 4 píxeles; para el píxel k (0 a 3, izquierda a derecha) con `s = 2*(3-k)`: byte par con verde en el bit `s+1` y flash en el bit `s`; byte impar con rojo en el bit `s+1` y azul en el bit `s`. 128 bytes por línea. Colores: bit2 = verde, bit1 = rojo, bit0 = azul (`C_*` en `platform.h`).
- **Gráficos:** cada palabra son 4 píxeles; se escribe directamente en pantalla (`$20000` en el QL, memoria en el PC con el mismo formato; `tools/ql2png.py` la convierte a PNG). 
- **Consola (c68):** `plat_init` redefine la ventana a pantalla completa, borde negro y sin cursor (SD.WDEF, SD.CLEAR, SD.CURS) y registra `plat_shutdown` con `atexit`; la cola de teclado de la consola se vacía con IO.FBYTE (timeout 0) al arrancar, en cada `plat_present()` y al salir (KEYROW no consume pulsaciones y SuperBASIC las recibía). `plat_exit()` limpia la consola y elimina el trabajo con `MT.FRJOB` (`ql_trap1`) para evitar «Press a key to exit». `plat_wait_key()` (esperar pulsación).
- **Teclado:** `KEYROW` por IPC (comando 9), una llamada por fila (0–7); bloque `MT.IPCOM` de 8 bytes `9,1,0,0,0,0,<fila>,2`, respuesta en D1.B. **Cada fila es una llamada lenta**: `key_ctx_menu()` lee las filas 1 y 7 siempre y el resto rotando (3 lecturas por fotograma); `key_ctx_flight()` lee las filas 1, 3 y 4 siempre y 0, 2, 5, 6 y 7 rotando (4 lecturas). Una tecla de fila rotante se detecta en como mucho 5 (vuelo) o 6 (menús) fotogramas. `KEYROW` lee el teclado físico aunque otra ventana tenga el foco; tres teclas en un rectángulo de la matriz dan una fantasma. 
- **Joystick:** en el QL son teclas: CTL1 = cursores y espacio (fila 1), CTL2 = F1–F5 (fila 0). Disposición UK de los menús provisional; CTRL+Izq da el carácter 8.
- **Sonido:** `BEEP` por IPC (`MT.IPCOM`), orden 10 con 8 bytes de parámetros (`$0000AAAA`), bloque de 15 bytes con intervalo y duración **con el byte bajo primero** y **respuesta 1 (ninguna)**; `BEEP OFF` = orden 11 (también con respuesta 1). **Si se pide una respuesta que el 8049 no manda, el enlace se desincroniza y el QL se cuelga**. Un `BEEP` nuevo cancela el anterior; no sonar mientras se carga o descarga un job. Pitch = 0,022457·ciclos − 9,6 (frecuencia = 22917 / (2·(pitch + 9,6)) Hz); duración e intervalo en pasos de 43,64 µs (el 8049 da 22.917 por segundo; los 72 µs del manual están mal). Todos los efectos del original están traducidos desde los bucles del 6502, con prioridades.
- **Trap #3:** `ql_trap3(d0,d1,d2,d3,a0,a1)` en `ql_ipc.asm` carga registros, ejecuta `trap #3` y devuelve D0 (0 ok, negativo error QDOS). Códigos: IO.FBYTE=1, SD.WDEF=$0D, SD.CURE=$0E, SD.CURS=$0F, SD.CLEAR=$20; de `qdos.h` solo se usan `fgetchid` y `mt_dmode`.
- **Disco:** imagen `elite.img` con `make elite.img` (3.5); arranque por el fichero `boot`. Ficheros QDOS desde C (`fopen`/`fwrite`): `flp1_`, `flp2_`, `mdv1_`, `mdv2_` (o directorio en el PC). Los datos van como `<unidad>elite_dash` (panel) y `<unidad>elite_<nombre>` (partidas). **Unidad del juego** (`find_drive` en main.c): el argumento opcional de `EXEC_W dev$&"elite";dev$` (sintaxis de Toolkit II/SMSQ/E: sin TK2 da `bad parameter`, por eso el `boot` no lo pasa; el runtime de c68 lo da en `argv[1]`; se le añade `_` si falta) o, sin argumento, la primera donde se abra `elite_dash`: `DATA_USE`, `PROG_USE` (`plat_default_dir`: `getcdd`/`getcpd` de c68), `flp1_`, `flp2_`, `win1_`, `mdv1_`, `mdv2_`; si no aparece, `flp1_`. Sin TK2, `getcdd`/`getcpd` devuelven basura de 2-3 bytes (c68 lee la cadena de la dirección 0 porque el puntero de `SV+$AC`/`$B0` es 0); `save_valid_device` la descarta. El juego silencia el sonido antes de tocar el disco.
- **Rendimiento:** el 68008 es lento y el bus es de 8 bits.

## 8. Trampas, reglas y hallazgos

### 8.1 Compilador (c68)
- **Fallo del optimizador `-O`:** 
   - En `x = (u8)d; cc = (d >= 0);` c68 usa los flags del `move.b` para el `sge` en lugar de los de la palabra `d`; da error cuando el byte sale positivo con `d` negativo (en el QL: naves 3 veces más anchas o irreconocibles). **Regla: calcular siempre el acarreo/signo ANTES de guardar el byte** (`cc = (d >= 0); x = (u8)d;`). Aplicado en 25 sitios de `ship3d.c` y `ship_math.c`; en el código nuevo (`move.c`, `ships.c`) también.
   - En `dt_bprnt` (detok.c), `d = k[x] - tens[x] - borrow; x15[x] = (u8)d; cy = (d >= 0);` daba otro acarreo con c68 y los numeros de la pantalla de datos salian mal en el QL. Arreglado calculando `cy` antes de truncar. **Regla: en todo codigo nuevo con resta y acarreo, el signo antes del `(u8)`**,
   - Anadir una sonda de c68 por cada modulo nuevo (las sondas de las rondas 13-15 estan en `tests/c68/probe_missions.c`). 
- `int`: con `-int=16` (`make c68_check`) es de 16 bits y la tabla de senos de `shipview.c` es `long` (el 32768 se desbordaba); con el `qcc` normal del `Makefile` es de 32 bits (2026-10-22). **El ensamblador no debe leer por simbolo variables `int`** (solo `u8` y punteros): los valores `int` se pasan como argumentos `long`.
- **Sondas de `c68_check`:** en el PC `unsigned long` es de 64 bits; un generador congruencial en una sonda (`r = r * 1103515245UL + 12345UL`) da otra secuencia que en c68 (32 bits) y la sonda sale DIFERENTE sin que c68 falle. Hay que enmascarar (`& 0xFFFFFFFFUL`).

### 8.2 Hallazgos del original que se reproducen
- `tnpr` suma en **8 bits**: con 193 o más unidades de un artículo en kg/g, comprar (256 − las que llevas) da la vuelta y se acepta. `GCASH` calcula el importe en **16 bits**: una venta de más de 6.553 Cr de una vez pierde los bits altos (p. ej. 200 kg de platino). Se reproducen con `TRADE_FAITHFUL 1` en `game.h`; con 0 se corrigen.
- **`PDESC`:** el original pone `NRU% = 0` y recorre 256 posiciones desde `RUPLA-1` (memoria ajena; puede colgar la pantalla de datos durante la misión 1); el port recorre las 26 entradas. Medido en el emulador: casi nunca cambia el resultado (1 de 200 casos).
- **`MT17` (encontrado al verificar, 2026-10-17):** `LDA BUF-1,X` con `BUF = $0100` se ensambla en página cero (`B5 FF` en el binario de referencia), así que `VOWEL` mira el byte `DTW5 - 1` de la página cero, no la última letra del nombre: casi nunca quita la vocal antes de «IAN». Se reproduce con `zp_peek` (RAND, INWK y K3; `CHPR` guarda cada carácter en `K3`).
- **`MCNT` baja** (`DEC MCNT` en `TT100`, tras `M%`); el port lo subía. Ahora baja como en el original y a 0 se ejecutan las partes 2 a 4; `WARP` pone `MCNT = 1` y `EV = 0`.
- **Acarreos de `DORND`:** en `DT6` entra con C = 0 (`CMP #129`); en `MT18` el primero con C = 0, el segundo con el de salida del primero y los demás con C = 0 (`DASC` vuelve con `CLC`); en las partes 2 a 4 cada `DORND` lleva el acarreo de la instrucción anterior (`encounters.c` los modela uno a uno; el de entrada sale de `M%` y se aproxima con el del último `DORND`).
- **Texto en modo texto del Apple:** `clss` (pasar de la fila 23) solo mueve el cursor a (0, 0) en modo gráfico; en modo texto el original se colgaría. El port lo mueve siempre.
- **`WPLS2`** necesita `LSY2[0] = $FF` como centinela; en el original lo pone `RES2` al despegar (en el port, `sun_forget` y el valor inicial).
- `LL52` negaba con `1-x` en vez de `1-x-1` en la primera transcripción (corregido; lo encontró la verificación).
- `CHECK`/`CHECK2` detectan solo el 57 % de los cambios de un bit.
- **Láser (encontrado al verificar, 2026-10-17):** `SHD` no es solo «+1 sin pasar de 255»: **cae en `DENGY`**, así que cargar un escudo gasta 1 de energía. En `SPIN` el `TYA` tapa el número aleatorio: la cantidad de restos es `tipo AND byte 0 del plano AND 15` (0 o 4 placas; 0, 1, 4 o 5 cargas), solo con un 50 % de probabilidad. En `SFS1` la velocidad de los restos es el `X` que deja `DORND` (aleatoria 0–15), no el tipo. Los `DORND` de `LASLI`, `SPIN` y `SFS1` reciben acarreos que salen de `ADC`/`CMP` del propio código (`em_rand_c` se pone a mano; tras `NWSHP` es 1 si hubo sitio).
- **Láser (comportamiento del original que se reproduce):** con el militar contra un Constrictor el `LSR LAS` deja la potencia a 1/4 el resto del fotograma; la estación no recibe daño pero se enfada (`AN2`: `NEWB` bit 2 de la ranura 1); **no se cobra la recompensa si hay un mensaje en pantalla** (`DLY` ≠ 0) o en el hiperespacio (`MJ`); la nave destruida deja en su byte 35 lo que dejara `A` el sonido (aquí 0).
- **Daños (encontrado al verificar, 2026-10-17):** al recoger una carga el `DORND` entra con C = 1 (lo deja el `CPX #OIL`); la altitud es `SQRT(R Q)` con el `Q` que hubiera de antes (puede variar en 1–2 unidades); al chocar contra el planeta `ALTIT` queda en 255 (el `JMP DEATH` sale antes). El `SBC` de `OOPS` no lleva `SEC`: el acarreo de entrada viene de quien llama (en un choque, el bit 0 de la energía de la otra nave).
- **Daños (comportamiento del original que se reproduce):** chocar con una nave la destruye y da recompensa (pero no suma al `TALLY`); con los escudos agotados, cada golpe puede destruir mercancía o equipo, también el recogedor de combustible o la unidad de energía; los restos de nuestra nave en `DEATH` nunca chocan con nosotros (la z lleva el bit 6). En el original `SOEXPL` (el ruido de las explosiones) llama a `DORND` y gasta números aleatorios si el sonido está activado; desde la 16.ª ronda el port también (antes no).
- **Planetas en el borde (fallo visto en el QL, corregido y verificado, 2026-10-17):** `WPLS2` (borrar el planeta) decide si copia (X2, Y2) en (X1, Y1) mirando `SWAP`, que en el original deja `LOIN` (0, o $FF si intercambió los extremos). El port dibujaba con `gfx_line`, que no toca `SWAP`, y quedaba el que había puesto `LL145` al recortar en el borde: con `SWAP` ≠ 0, `WPLS2` trazaba todas las líneas desde el mismo punto. Lección: **una primitiva propia que sustituya a una rutina del original debe reproducir sus efectos en las variables que lee alguien después**.
- **SUN:** el `DORND` del borde irregular recibe el acarreo con que vuelve `LL5` (bit 7 del resto); `em_ll5` es ahora una transcripción literal que lo devuelve en `em_carry`. `HLOIN` redondea X1 y X2 a pares y, si X2 < X1, deja X2 = X1. **`RES2` pone `LSX2` y `LSY2` a $FF**: el centinela de `LSY2[0]` sí lo inicializa el original (al despegar), no es un fallo suyo.
- **DOCKIT** mira `K3+10` (= `XX2+10`, visibilidad de la cara 10 de la última nave dibujada) para decidir si una nave ha atracado; se reproduce tal cual (`zk3[10]`). En `DOKEY`, un alabeo fuerte del ordenador de atraque pone `JSTX = 64` y suelta la tecla de alabeo aunque esté pulsada.
- **La bomba:** la versión de la fuente vuelve a lanzar la bomba (sin borrar el rayo anterior) si se pulsa `B` mientras explota; la publicada (IB disk) lo impide y es la que se sigue.
- **TACTICS (al portar y verificar, 2026-10-17):** `BIT M32+1` usa como máscara el operando de la instrucción `LDY #32` (= $20: «explotando»); `EXNO2` tras un impacto de misil recibe la **ranura** del blanco, no su tipo (y `TA352` usa `LDX #PLT`); `FRMIS` cae en `ANGRY` con la A que deja `SOHISS`; `ANGRY` mira `TYPE` (la nave en curso), no el tipo de la enfadada, para decidir si la vuelve hostil; el ECM de cualquier nave usa el mismo `ECMA` y cambia nuestra bombilla E; cada `DORND` de TACTICS recibe el acarreo de la comparación anterior (modelado en `em_rand_c`). En la prueba, el sonido está desactivado (`DNOIZ` = $FF) para que `SOEXPL`/`SOHISS` no gasten números aleatorios (el port los gasta desde la 16.ª ronda solo con el sonido activo; en las pruebas no se pone `snd_dornd_hook` o no se llega a esos sonidos).
- **`DVIDT` (encontrado al verificar Maths Geometry, 2026-10-18):** tras `ASL P; ROL P+1`, el `ASL Q; LSR Q` deja C = 0, así que **el bit 7 de `P+1` se pierde** y el primer `ROL A` mete un 0. La transcripción en C y la de Python (`ref6502b.dvidt`) lo metían; con ello `TIS3` daba otro valor y `TIDY` (ortonormalizar la orientación de las naves) salía distinto en el 44 % de los casos. Corregido en los dos.
- **`FMLTU` con A = 0 o Q = 0 no toca el acarreo** (`BEQ MU3`/`MU3again` antes del `CLC`): sale con el de entrada. En `EXS1` la rama `EX5` resta luego con C = 1 (sin préstamo). `em_fmltu` ya no pone `em_carry = 0` en esos casos (el que llama pone el de entrada).
- **`LOIN` del Apple II (al verificar punto a punto, 2026-10-18):** la pendiente sale de `log`/`logL`/`alogh` (cociente con logaritmos, 255 si no hay préstamo), el error empieza en S = 128, un punto por cada x (o y) y **no dibuja uno de los extremos**: en las líneas más horizontales que suben, el (X2, Y2) de la llamada; en las que bajan o son planas y en las más verticales, el (X1, Y1). Los acarreos que entran en `S + Q` son siempre 0. El Bresenham que usaba el port (`gfx_line`) no coincidía en ningún caso; ahora `loin_draw` (lines.c) es la transcripción y la usan naves, planetas, sol, láser y bomba. `HLOIN` y `VLOIN` (con `CPIX`, 2 puntos de ancho) ya coincidían en cobertura.
- **Start and end (21.ª ronda):** `RES2` hace mucho más que los giros: `NOSTM = NOST`, `LSX2`/`LSY2` = $FF, `MSTG` = $FF, `JSTY` = 128 (**no** `JSTX`), giros y `DELTA`/`ALPHA`/`ALP1` = 3, `dontclip` = 0, `Yx2M1` = 135, `SPBLB` si hay estación (`SSPR`), `ECMOF` si suena el ECM, `WPSHPS`, `ZERO`, `SLSP = LS%` y cae en `ZINF`. `RESET` = `ZERO`, `BETA..ALP2` (7 bytes de página cero: giros, `QQ22`, `ECMA`, `ALP1`) a 0, `JSTGY` = $FF, `QQ12` = $FF (atracados), `FSH`/`ASH`/`ENERGY` = $FF y `RES2`. En `DFAULT`, `CHK2`, `CHK3` y `CHK` son variables fijas **fuera** del bloque copiado (84 bytes: nombre y `TP`..byte 75); si `CHK` o `CHK3` no cuadran el original repite `CHECK` **para siempre** (antimanipulación) y el port devuelve error; los bytes 20-21 y 47-50 del bloque no se usan. `TT170`, `TITLE`, `BR1`, `DEATH2` y `DEATH` son flujo de pantallas propio: `no-verificable`.
- **Moving y Equipment (23.ª ronda):** el acarreo con que sale `MLTU2` (16 vueltas de `ROR`/`ADC`) es el último bit que sale de `ROR P` y es **siempre 0** (comprobado con 100.000 pares); la fórmula anterior (`(~P) AND Q AND 1`) daba 1 a veces y descuadraba `INWK+3` de las naves en la parte 3 de `MVEIT`. `MVEIT` con un planeta o sol que tenga los bits 7 o 5 de `INWK+31` lee el plano en `XX0` (memoria ajena): no ocurre en el juego y la prueba no lo genera. En `EQSHP` el orden es: `eq` cobra (si no hay dinero, mensaje y fin, aunque ya se tenga), luego `pres` devuelve el importe si ya se tiene (también el misil número 5), y los láseres reembolsan el antiguo con `refund` (POW→4, POW+128→5, Armlas→12, cualquier otro→13).
- **Dashboard y Flight (25.ª ronda):** `DIS7` llama dos veces a `MSBARS` con el truco `JSR P%+3` (cada una dibuja Y1+1 e Y1+2), es decir 4 `HLOIN` por barra; `DIS1+2` deja un solo `LSR` (el combustible es `QQ14 >> 1`); `CPIX` empieza con `STA Y1`, por eso el `VLOIN` del palo del radar usa la y de la cabeza; `cntr` mira `auto` antes que `DAMP` (con el ordenador de atraque centra siempre). La bombilla E está en x = 56 y la S en x = 192, y = 176..183, bit 0 a la izquierda; `ECBT`/`SPBT` son `7F 7F 07 7F 7F 07 7F 7F` y `7F 7F 07 7F 7F 70 7F 7F`. `MJP` en la variante IB repite `GTHG` hasta tener 3 Thargoides y deja `NOSTM = 2` (en la variante `_SOURCE_DISK_CODE_FILES` es otra condición). En `TT110` la estación es del tipo 2 con el plano del Dodo si el nivel tecnológico es ≥ 10. En `TT18` el `DORND` del salto fallido entra con el acarreo de `CMP #9` (C = 1 si hay una tecla ≥ 9 pulsada; el port supone sin tecla).
- **Sonidos que gastan números aleatorios:** `SOEXPL` (explosiones, muerte), `SOHISS` y `SOBOMB` llaman a `DORND` una vez por clic para el retardo (Y veces, 256 si Y = 0; 25 en `SOBOMB`) **si el sonido está activo** (`DNOIZ` bit 7 = 0). Ahora el port también (`snd_dornd_hook`, que pone `ships_hooks`). En `BOMBEFF` el acarreo con que entra `BOMBON` es 1 en la vista espacial y, fuera de ella, el que deja `SOBOMB`.
- **Colores del radar:** en el Apple II `CYAN` y `MAG` valen `WHITE`: las naves «cian» y «magenta» del radar son blancas. El port las muestra en cian y magenta (decisión de presentación, sección 13).
- **`DOEXP` con 6 en el byte 2 del montón** (ninguna nave: `4 * vértices + 6`) haría que `EXL1` diera la vuelta a Y (256 copias); no ocurre con los planos del juego y la prueba no lo genera.
- **Modo texto del Apple:** las vistas de texto (todas menos el espacio, el título y las cartas) usan la página de texto de 40 × 24; `NLIN4` pone entonces guiones (`NLI4`) bajo los caracteres de la fila 1. El QL no tiene modo texto: `nlin4` dibuja siempre la línea de `y = 19` del modo gráfico.
- **Polvo estelar (17.ª ronda):** los `DORND` de `KILL1`/`KILL6`/`KILL2`/`ST5` reciben el acarreo de la comparación que los manda allí (`CMP #120; BCS` → 1, `CMP #16; BCC` → 0, `BEQ` → 1); `PIXEL` vuelve con C = 1 si ZZ ≥ 80 o si la fila no es múltiplo de 8 (si no, 0) y `PIXEL2` con C = 1 si la partícula está fuera; el primer `DORND` de `nWq` lleva el acarreo de entrada, y `nWq` cae en `WPSHPS`. El port no modelaba esos acarreos (el 22 % de los casos daba otra nube).
- **Cartas (17.ª ronda):** los puntos de la galáctica son `PIXEL` con ZZ = 255, es decir **dos puntos** (el port ponía uno); la carta local también tiene la línea de `NLIN4` en y = 19; `TT16`/`TT123` mueven la mira sin pegarla a ningún sistema y no la mueven si daría la vuelta (antes el port la pegaba al sistema más cercano y la limitaba a la parte visible de la local: ver 13.1); en `TT23`, con la fila Y = 1 ocupada, `Y - 2` = 255 lee `INWK - 1` = `SC+1` (nunca 0, el port lo trata como ocupada); `HME2` compara el nombre con el que imprime `cpl` en el búfer justificado: con `QQ17 = $C0` (lo normal) funciona, con `$80` no encuentra el sistema 0 (sale en mayúscula) y con 0 o `$40` ninguno.
- **Túneles (17.ª ronda):** en `HFL2`, cada `SOHISS` entra con C = 0 (`CLC; ADC #8` o `CMP #160; BCC`); `TT110` hace `LAUN` (dos `SOHISS` de 256 y el túnel) **antes** de `RES2`/`SOS1`/`NWSPS` y luego `HFS1` vuelve a dibujar los mismos anillos (con sus sonidos) para borrarlos; el port lo hace ahora en ese orden (`tunnel_laun`, `space_launch`, `tunnel_draw`). En `LL164` el primer `SBC #10` lleva el acarreo de entrada (con C = 0 los tonos bajan de 11 en 11 la primera vez).
- **Pantalla (17.ª ronda):** `TTX66` deja `LSP = 0`, imprime en la vista del espacio su nombre («Front View» en Sentence Case, columna 11, fila 1) y, si hay cuenta atrás de hiperespacio, el número en (1, 1); las vistas gráficas llevan `BOX` (línea azul en y = 0, `HLOIN` de 0 a 255 más las esquinas: el punto 254 también); los bordes azules que pinta `clearrow` (bytes 0 y 37 de cada fila) caen fuera de los 256 puntos del juego; `CLYNS` (ficha de salto 21) no hacía nada en el port: ahora borra las filas 15 y 16 (y = 120..135) en las vistas gráficas o escribe 64 espacios desde (1, 21) en las de texto.
- **Herramientas (17.ª ronda):** `PIX1` (Maths) cae en `PIXEL2`, `TT22` en `TT15`, `TT16` en `TT103`, `TT20` en `TT54`, `nWq` en `WPSHPS`, `CLYNS` en `clearrow` y `clearrow` en un `RTS` (`SCR1`): el bloque siguiente tiene que ir detrás. `BOX` no tiene cabecera propia (está en el bloque de `TTX66K`), `DTW2` es un byte del código (sin definirlo, `STA DTW2` pisaba un stub) y `PIXEL` real necesita `TWOS3`, `SCTBL/H/H2` y `SCTBX1/2`. Un gancho puesto en la dirección de una rutina real registra y la deja ejecutarse.
- **Herramientas (16.ª ronda):** cuando una rama no llega a una etiqueta de fuera, `frag6502.build` añade un `RTS` local, **que no imita lo que haya allí**: `PL21` es `SEC; RTS` y con el RTS local `PLS6` devolvía C = 0 en el desbordamiento (había que incluir la rutina real). Si la rama está lejos del final hay que poner el `RTS` local justo delante del bloque (`TT48` en `DOEXP`). `SKIP n` se cambia por `n` × `EQUB 0` (`BOMBTBX`); `EQUD %...` de `MASKT` se expande; `DNOIZ` es una etiqueta del código (sin ella quedaba como stub, cuyo byte $60 tiene el bit 7 a 0 = sonido activo). `tests/test_state` acepta el fichero de vectores como argumento y ahora imprime el valor del port en los fallos.
- **Herramientas:** `frag6502.build` convierte en stubs los `EQUW` a rutinas de fuera (tabla `JMTB`); el bloque de `TT26` necesita también el de `BELL` (el `EQUB $2C` de `DA7` salta a su `LDA #7`). `elitesrc.py` no evaluaba `INT(...)` (`Armlas` quedaba sin definir y el ensamblador lo convertía en un stub); arreglado.

### 8.3 Reglas de oro
- **Dibujo en XOR:** borrar = volver a dibujar lo mismo (mismos puntos en el mismo orden). Si se limpia una zona sin avisar al montón de líneas (naves, sol, círculo), hay que **olvidar** esas líneas.
- **Una `u8` truncada no se compara:** calcular el signo antes (10.1).
- **Acarreos como parámetros:** modelarlos explícitamente; ante la duda, mirar qué instrucción los consume (`ADC`/`SBC`/`ROL`/`BCS`).
- **Datos de naves** se extraen con script, no a mano.
- **Entregar siempre** el tar + los tres `.md` al final de cada paso, y comprobar el tar en un directorio limpio.

## 9. Decisiones y desviaciones del original

### 9.1 Criterio de «rutina no necesaria»:
- *Anticopia y arranque del Apple II:* `ENTRY`, `S%`, `DEEOR`, `DEEORS`, `G%`, `F%` (suma de comprobación y descifrado del código), `NMIpissoff`, `COLD`, `BEGIN`, `MUTILATE`/`UNMUTILATE` (cifrado antimanipulación del fichero de comandante; `save.c` tiene su propio cifrado), `SWAPPZERO`.
- *Lo que da el compilador/runtime de C:* categoría **Utility routines** (rellenos de memoria, copias de páginas, retardos por bucles, manejador de errores `BRK`), **Workspaces** (`ZP`, `XX3`, `K%`, `WP`, `Disk`, `UP`, `Option`: son variables de C) y **Tube** (`newosrdch`).
- *Hardware del Apple II/BBC/C64:* tablas de la pantalla hires (`SCTB*`, `cellocl`, `TWFL/TWFR`, `MASKT`), cambio de modo (`HGR`, `TEXT`), impresión en modo texto (`CHPR2`, `R5`, `RR5`), altavoz (`CLICK`), registrador de teclas, sprites de los Trumbles (VIC).
- *DOS 3.3 / RWTS:* catálogo, sectores, pistas, nibbles, brazo del disco (QDOS gestiona el disco).
- *Sin uso en el original* (`MUT3`, `LASNOISE2`, `beamcol`, `DKSANYKEY`, `KTRAN`).

### 9.2 Decisiones
- **Port fiel con partes propias donde el hardware lo exige:** `LOIN` es la transcripción (`loin_draw`, desde la 16.ª ronda; antes era Bresenham); `HLOIN` = `gfx_hloin` y `VLOIN` = `gfx_vloin` (misma cobertura que el original, color liso en lugar de los patrones de bits del Apple); los colores del radar cian y magenta (blancos en el Apple II) se mantienen; las cartas siguen siendo pantallas propias (`menu.c`: título en una barra azul, información del objetivo abajo, pasos de la mira de 1 o 2 por fotograma), pero lo que se dibuja es el original; el panel usa el bitmap original convertido a modo 8; el fichero de partida es propio (ELQD, 94 bytes).
- **Montones de líneas** por ranura (256 bytes) en lugar del reparto con `SLSP`; `K%` con 9 ranuras.
- **Estación y planeta:** las posiciones del original (`TT110` al despegar, `SOLAR` al llegar, parte 14 y `KS4`); `SOLAR` y `NWSPS` completos.
- **Teclas de prueba** `N`, `K` y `C` sin ordenador: solo en las pruebas (`flight_testkeys`).
- **`JSTGY`** empieza en $FF (eje Y normal), como el disco IB; la versión de la fuente lo deja a 0. **`K` (`JSTK`)** activa el joystick CTL1 del QL (cursores y espacio, que ya eran los controles). El título muestra los autores solo con `PATG` (tecla `X` en pausa), como el original.
- **Universo y mercado** vienen de Text Elite (verificado contra su oráculo, no contra el 6502); `uni_distance` sigue el 6502.
- **Textos de los menús como cadenas C** (salvo la pantalla de datos del sistema, que es la `TT25` del original); las pantallas de las misiones y las descripciones especiales usan el **motor de fichas original** (`detok.c`, con las tablas de la fuente). La descripción normal sigue con Text Elite.
- **Sonido** traducido desde los bucles del 6502 a `BEEP` del QL: aproximado.
- **`TRADE_FAITHFUL 1`**: se reproducen dos peculiaridades aritméticas del original.
- **Teclas** repartidas para el teclado del QL (sección 8), distintas de las del Apple II.

### 9.3 Mejoras propias que se han quitado al verificar contra el original (por si interesa recuperarlas)

Cada una se puede volver a poner como opcion de compilacion; aqui queda que hacia y donde estaba.

| Mejora del port | Que hace el original (lo que hay ahora) | Donde estaba / como recuperarla | Ronda |
|---|---|---|---|
| **Fuente de dominio publico** `font8x8_basic.h` (D. Hepper), `tools/gen_font.py` generaba `src/font.h` desde `font8x8_basic.h` (invertia bits: bit 0 -> bit 7); bastaria otro generador con ese fichero | 15 |
| **Sin limite de naves por el monton de lineas**: cabian siempre 8 naves (NOSH) | El monton `SLSP` (de `LS%` = $0B5F hacia abajo, cada nave el byte 5 de su plano) hace fallar `NWSHP` antes: p. ej. solo 4 Cobras | `ship_new()` en ships.c: quitar la comprobacion de `orig_slsp` (el port no necesita el monton: cada ranura tiene el suyo, `heaps[n]`) | 15 |
| **Pantalla de datos propia** con etiquetas y valores en columnas, en colores (blanco/verde/cian), y la descripcion de Text Elite (`uni_describe`) reflejada a lo ancho | `TT25`: el texto del original, en blanco, justificado a 30 columnas, con la especie y la descripcion de la ficha 5 | `scr_data()` antigua en menu.c (con `lbl`, `val`, `put_wrapped`); `uni_describe` sigue en universe.c. Ademas tenia un fallo: la poblacion salia como `pop >> 3 . pop & 7` (formula de Text Elite) en vez de `pop / 10` | 15 |
| **Planetas de la carta corta con `SUN`** transformado (relleno por pares de puntos, deformado) y **texto de la pantalla de datos justificado** (`TT25` original) | `sun_xf_disc` (elipse de 1 punto) y `dt_nojustify` | `tt23` de chart.c (rama `chart_xf_mode() == 1`) y `tt25()` de missions.c: quitar esas dos lineas | 27 |
| **Vocal final quitada bien** en el adjetivo del sistema ("Zaonce" -> "Zaoncian") | `MT17` mira un byte de la pagina cero (fallo de `LDA BUF-1,X`): casi nunca la quita ("Zaonceian") | `detok.c`, `case 17`: `if (vowel(dt_buf[dtw5 - 1])) dtw5--;` | 13 |
| **Teclas de prueba `N` (nave hostil) y `K` (explotar la mas cercana)** | no existen | `flight.c` (`flight_testkeys`) | 14 |
| **Autores en el titulo siempre** | solo con `PATG` (tecla `X` en pausa) | `scr_title()` en menu.c | 14 |
| **Estacion al despegar mas lejos** (detras y algo por debajo, z = -1536) y regla propia para quitarla (z_hi > $30) | estacion a z = -256 y se quita al pasar de 224 * 256 (`FAROF`), con el sol en su ranura (`KS4`) | `ships_launch()` y `ships_step()` en ships.c | 14 |
| **Lineas con Bresenham** (`gfx_line`): dibujaban los dos extremos y una pendiente exacta | `LOIN`: pendiente con logaritmos y sin uno de los extremos (10.3) | `gfx_line` sigue en gfx.c; bastaria volver a llamarla desde `default_loin` (ship3d.c), `loin()` (sun.c), `las_pair` (combat.c) y `bomb_off` (equip.c) | 16 |
| **Brazo vertical de las cruces de 1 punto** (TT15, mira) y linea bajo el titulo de las misiones de 0 a 255 | `VLOIN` = `CPIX` de 2 puntos de ancho; `NLIN4` es `HLOIN` de 0 a 255, que cubre 0..253 | `vloin()` de chart.c era `gfx_line`; `nlin4` de missions.c era `gfx_hline(0, 255, 19)` | 16 |
| **Los sonidos no gastaban numeros aleatorios** (la partida no dependia de tener el sonido activo) | `SOEXPL`, `SOHISS` y `SOBOMB` llaman a `DORND` en cada clic si el sonido esta activo | quitar `dornds()` de sound.c (o no poner `snd_dornd_hook`) | 16 |
| **Mira de las cartas pegada al sistema mas cercano** y limitada a la parte visible de la carta local | `TT16`/`TT123`: la mira va libre (QQ9, QQ10) y solo `D` (`hm`) o el salto la llevan al sistema (`TT111`) | `scr_chart` de menu.c: `cx`/`cy` con `clampi` y `game_set_target(uni_nearest(...))` en cada paso | 17 |
| **Vista del espacio sin titulo ni linea superior** (solo el pie de pantalla propio con la vista) | `TTX66`: «Front View» (o Rear, Left, Right) arriba y `BOX` (linea azul en y = 0) | `clear_view()` en lugar de `space_tt66()` en flight.c | 17 |
| **Puntos de la carta galactica de un solo pixel** | `PIXEL` con ZZ = 255: dos puntos | `gfx_pixel` en lugar de `gfx_dust(x, y, 255)` en `tt22` | 17 |
| **Sin limite en `PDESC`** (se mantiene): el port recorre solo las 26 entradas de `RUPLA` | `NRU% = 0`: recorre 256 posiciones de memoria ajena y puede colgarse | sigue asi en `missions.c` (no se ha quitado: es una correccion) | 13 |

