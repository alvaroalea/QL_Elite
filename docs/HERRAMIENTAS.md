# Herramientas y entorno

Movido desde `CLAUDE.md` (secciones 3.2–3.6 y 5.3).

## 3.2 Compilador de C para el QL (XTC68) — instalación si no esta disponible.
`tools/install_xtc68.sh` automatiza lo que sigue:
1. `git clone https://github.com/stronnag/xtc68` (migrado a Codeberg, pero el clon de GitHub sirve) y `git clone https://github.com/xXorAa/c68-support`.
2. `tools/install_xtc68.sh` instala XTC68 + runtime c68 en `/tmp/qlsdk` (ver 3.2). Luego `export PATH=/tmp/qlsdk/bin:$PATH QLINC=/tmp/qlsdk/share/qdos/include QLLIB=/tmp/qlsdk/share/qdos/lib` y
3. gcc moderno falla con `-Werror` en las fuentes antiguas (p. ej. «ignoring return value of write»): `sed -i "s/'-Werror', *//g" src/*/meson.build`.
4. `meson setup build --prefix=/tmp/qlsdk --strip && ninja -C build install` → `bin/{qcc,c68,as68,qld,qcpp,qdos-ar,...}`.
5. El runtime de c68 (cabeceras `INCLUDE_*` y librerías `LIB_*`, del c68 runtime 4.24 «424frun1») va en una carpeta **`support/424frun1`** del repositorio xtc68, y se instala con `./sdk-install.sh /tmp/qlsdk` ejecutado **desde esa carpeta** → `share/qdos/include` y `share/qdos/lib`.
6. Uso: `export PATH=/tmp/qlsdk/bin:$PATH QLINC=/tmp/qlsdk/share/qdos/include QLLIB=/tmp/qlsdk/share/qdos/lib`; `qcc -O -Isrc -o elite ...` (el `Makefile` ya lo hace con `make elite`). `-Isrc` es obligatorio porque qcc busca los `#include "..."` en el directorio actual. `qcc` imprime `elite: dataspace 18114 (46c2)` al terminar bien.
7. `as68` es de la familia Sozobon: directivas `.globl`/`.text`, comentarios con `;`, etiquetas con dos puntos, ficheros `.asm` (qcc no reconoce `.s`); no valen `xdef`, `section` ni comentarios con `*`.

## 3.3 Verificación de código de c68 (`make c68_check`)
`tools/c68_check/run.sh [sonda ...]` compila cada sonda de `tests/c68/` con gcc (PC) y con el `c68` real (`-int=16 -O -warn=0`, preprocesando con `gcc -E -P -U__GNUC__ -nostdinc`), convierte la salida de c68 (sintaxis ACK) a GNU as con `conv.py`, la enlaza con un mini runtime (`rt.s`: `.Xlmul`, `.Xldiv`, ..., salida de pantalla) y la ejecuta bajo `qemu-m68k`. Cada sonda deja hashes en la «pantalla» y se comparan byte a byte. `XTC68=<dir con c68>` (por defecto `/tmp/qlsdk/bin` si existe, si no `/tmp/xtc68/build`). Con `-U__GNUC__` se compilan las rutas «de QL» (por ejemplo las palabras largas de `gfx_fill_lines`). Sondas: `gfxfast, emath, keys, ships, game, charts, dash, flight, session`. **No sustituye al QL real** (no cubre el runtime ni el hardware), pero detecta fallos del compilador (encontró el de `-O`).

## 3.4 Herramientas Python (`tools/`)
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
| `inventory.py`, `ported.txt` | generan `INVENTARIO.md` |
| `m68k_mini.py`, `test_lineasm.py`, `test_ellasm.py`, `test_ll9asm.py`, `test_putsasm.py`, `gen_lltab.py` | intérprete mínimo de 68000 y pruebas del ensamblador propio (`ql_line`, `ql_ell`, `ql_ll9`, `ql_puts`) contra el C; `gen_lltab.py` genera `src/ql_lltab.asm` |
| `install_xtc68.sh`, `c68_check/` | cadena de qcc y verificación de c68 |
| `tmp/qltools_src/` (se crea sola) | clon y compilación de **qltools** de N. Dunbar (`make qltools`); formatea y escribe la imagen `elite.img` (3.5). No se incluye en el tar |

## 3.5 Imagen de disco `elite.img` (qltools)
`make elite.img` (alias `make disk`) hace todo de una vez y es repetible:
- **`elite`** (objetivo real; el alias `ql_game` se eliminó el 2026-10-24): compila con qcc si cambió algún `src/*.c`, `src/*.h` o `.asm`. qcc deja al final del fichero el trailer `XTcc` + dataspace (big-endian; hoy 16.840 bytes); qltools lo detecta al escribir y marca el fichero como ejecutable con ese dataspace (`E` en el directorio). Hace falta el `-W` de qltools, no copiar el fichero a mano.
- **`$(QLTOOLS)`** si no existe qltools en local se puede instalar y compilar en `tmp/qltools_src/` con  `git clone --depth 1 https://github.com/NormanDunbar/qltools tmp/qltools_src` y `make -C tmp/qltools_src/Unix` (lo hace `make qltools`). 
- Todo lo generado va a `build/` (`build/elite`, `build/elite.img`). `rm -f build/elite.img`; `qltools build/elite.img -fdd ELITE` (formatea DS/DD: 2 caras, 80 cilindros, 9 sectores por pista = 1.440 sectores = 737.280 bytes; ID `QL5A`); `qltools build/elite.img -W build/elite data/boot data/elite_scr data/elite_dash` (`-W` sobrescribe sin preguntar; los nombres en el disco son los del fichero sin el directorio); `qltools build/elite.img -d` lista el resultado.
- **Notas de qltools:** traduce `.` en `_` en los nombres (sin `-t`); ningún nombre del disco lleva punto. Muestra el año de la fecha de los ficheros como 126 (error solo de presentación). Los argumentos `-fdd` y `-W` van **antes** de la lista de ficheros; para comprobar un fichero con nombre sin traducir se usa `-t -n nombre`.
- En un emulador (sQLux, QPC2, uQLx): montar `build/elite.img` como `flp1_` y arrancar con `LRUN flp1_boot` (o dejar que lo ejecute el arranque del disco).

## 3.6 Imagen de microdrive `elite.mdv` (mdvtool)
- **`$(MDVTOOL)`** si no existe mdvtool en local se puede instalar y compilar en `tmp/` desde el repositorio https://github.com/xxoraa/mdvtool (`make mdvtool`; usa el del `PATH` si lo hay; necesita libzip).
- `make elite.mdv` genera `build/elite.mdv` (formato QLAY, etiqueta `ELITE`) con `boot`, `elite`, `elite_scr` y `elite_dash`. Copia los ficheros a `build/mdv/` e importa desde allí (mdvtool guarda el nombre **tal como se le pasa**, con directorio incluido). El `boot` del microdrive es el del disco con `dev$="mdv1_"` (sed). mdvtool detecta el trailer `XTcc` y marca `elite` como EXEC con su dataspace.
- **Capacidad:** 255 sectores de 512 B; quitando el mapa y el directorio quedan **253** para ficheros, y cada fichero lleva 64 B de cabecera (`MDV_SECTORS`). Con la imagen llena mdvtool termina con violación de segmento, así que la regla comprueba antes los sectores y falla con un mensaje. **Hoy el ejecutable (~147 KB, 146 KB sin `DEBUG`) no cabe** (hacen falta 369 sectores): por eso `elite.mdv` no está en el objetivo por defecto. Un cartucho real suele dar menos sectores que la imagen QLAY.




## 5.3 Cómo verificar una rutina nueva contra el original
`tools/orig.py` monta una imagen del 6502 con las rutinas originales que se pidan y stubs para las que dibujan; `tools/gen_laser.py` y `tools/gen_damage.py` son los ejemplos más cómodos; usan `tools/frag6502.py` (su `build(lineas, None, presets)` convierte en stubs que registran (nombre, A, X, Y) todo lo que no se resuelve, y pone un RTS local cuando una rama no llega al stub; ensambla tramos sueltos del bucle principal por etiqueta: `.MA68`..`.MA3`, `.MA47`..`.MA8`...; `presets` fija direcciones de tablas como `log`; si una rama no llega a una etiqueta de fuera, hay que ponerle un `RTS` local delante, p. ej. `['.out', 'RTS'] + OUCH`). Cuidado: un tramo que «cae» en la rutina siguiente (como `Ze` en `DORND2`) necesita esa rutina justo detrás en la lista; `tools/gen_trade.py` (preprocesa `IF/ELIF`, sustituye pantalla por stubs, ejecuta `tnpr` y los tramos de `TT219`/`TT208`). Pasos: volcar el estado inicial aleatorio en el emulador y en C, ejecutar, comparar INWK/variables/líneas dibujadas, repetir con valores límite, mutar una instrucción del C y comprobar que el test falla.

## Cronometraje del texto (`make ql_bench`)

`src/bench_text.c` imprime 2000 caracteres (papel opaco, transparente, `txt_puts` y números) y muestra los cuadros de 20 ms y los µs por carácter. Ejecutar `build/bench_ql` en el QL. Medido antes de `ql_text.asm` (C puro, tabla `fm`): opaco 73 t (730 µs/car.); antes de la tabla, 189 t. `ASM_TEXT` (en el `Makefile`, por defecto 1) usa `ql_puts` (`src/ql_text.asm`: tira de celdas con papel opaco, desenrollada) en `txt_puts` y `txt_putc`; el papel transparente sigue en C. Sin `ASM_TEXT` se usa la misma fórmula en C.
