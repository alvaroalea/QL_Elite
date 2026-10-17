# Elite Apple II → Sinclair QL: 

## 1. Objetivo del port

- **Juego:** Elite, versión Apple II (Firebird 1985). 
- **Fuente:** el código documentado de Mark Moxon, esta en el archivo elite-source-code-apple-ii-copy.zip es una copia del github. Ficheros: `1-source-files/main-sources/elite-source.asm`, de unas 47.000 líneas con unas 590 rutinas (cabeceras `;       Name: X`), y `elite-data.asm`.
- **Licencia:** ese repo no tiene licencia. Se usa como referencia, sin copiar sus comentarios; hay que escribir código propio. El original es de Bell y Braben.
- **Meta en QL:** versión de disco, QL de hasta 640 KB de RAM, modo 8 (256x256, 8 colores). El Apple II usa un viewport de 256x136 centrado en (128,68); las filas extra del QL van al panel inferior.
- **Fidelidad:** port fiel al Apple II, con aritmética de enteros de 8 bits. No se usa `newkind`: es de la versión BBC con extras de Pinder y usa coma flotante, inviable en un 68008 sin FPU.

- **Estrategia de trabajo (desde 2026-10-16):** se portan todas las rutinas del juego a C **sin** verificarlas contra simulaciones del original. Cada función nueva se anota en `PENDIENTES.md` (con la rutina de origen y cómo verificarla) y en `tools/ported.txt`; `INVENTARIO.md` (generado con `python3 tools/inventory.py`) lista las 528 rutinas de la fuente y cuáles están portadas. Cuando haya una versión que compile y arranque en el QL, se verifica la lista de pendientes contra el emulador de 6502, y el feedback del QL real reordena las prioridades. Lo ya verificado antes (secciones siguientes) se mantiene.

## 2. Arquitectura y estado técnico

- **Lenguaje:** C89 con `qcc` (XTC68) y un ensamblador `as68` para lo mínimo. Las rutinas críticas pasarán a ensamblador 68000 tras perfilar.
- **Proyecto:** `/home/claude/elite-ql`, entregado como `/mnt/user-data/outputs/elite_ql_base.tar.gz`. El sandbox puede perderse, así que hay que desempaquetar el tar.
- **Metodología de verificación:** se transcribe literalmente el 6502 en Python (`tools/ref6502*.py`, con acarreos explícitos) y se genera un conjunto de vectores. Se implementa en C (formas cerradas cuando son exactas) y se compara con `make test`.
- **Resultado** tras cada paso debes proporcionar el archivo elite_ql_base.tar.gz con el ultimo codigo fuente y Proyecto.md actualizado con los pasos y notas.

**Hecho y probado en PC**

- **Gráficos** (`gfx.c`, `platform.h`): píxel, píxel XOR, línea normal y XOR, `hline`, `rect`, `clear`.
- **Texto** (`text.c`): celdas 8x8 (32x32), tinta, papel y papel transparente, cursor XOR parpadeante. Fuente `font8x8` de dominio público, generada con `tools/gen_font.py`.
- **Aritmética** (`emath.c`): `DORND`, `MULTU`, `MU11`, `SQUA`/`SQUA2`, `MULT1`, `MULT12`, `MAD`, `ADD`, `MULTS`, `FMLTU`, `FMLTU2`, `MLTU2`, `MLU1`/`MLU2`, `MULT3`, `TIS1`, `TIS2`, `DVID96`, `LL5`, `LL28`, `LL61`, `LL38`, `DVID4`, `DV41`, `DV42`, `DVIDT`, `DVID3B`. Las variables de página cero son globales (`zP`, `zQ`, `zR`, `zS`, `zT`, `zU`, `zK[4]`, `em_x`, `em_carry`). Unas 896.000 comprobaciones sin fallos con `make test`.
- **Teclado y joystick** (`keys.c`): 27 pruebas con `make test_keys`.
- **Sonido** (`sound.c`): 26 pruebas con `make test_sound`.
- **Estado del juego** (`state.h`, `state.c`): bloque de nave INWK de 37 bytes (incluye XX19 y NEWB) con desplazamientos `I_*`; K% como `ks` con NOSH+1 ranuras de NI bytes (una de resguardo, porque UNIV tiene NOSH+1 entradas); `univ[]`, `frin[]` (NOSH+1), `many[]` (`sspr` = `many[SHIP_SST]`), junk/auto/ecmp/mj; zona de trabajo de página cero (`zP2`, K3 y K4 contiguos en `zk3[16]` con `zk4`, `zx15`, `zx12`, `zxx16`); variables QQ* y del comandante con los tamaños del original (CASH en big-endian). `state_clear()` lo pone a cero e inicializa UNIV. K, P, P+1, Q, R, S, T y U siguen en `emath`.
- **Aritmética dependiente del estado** (`ship_math.c`): MVT3, MAS1 a MAS4, TAS1, VCSUB/VCSU1, TAS2, TAS3, TAS4, TAS6, NORM, TIS3, TIDY, SPS1 a SPS4, PLS6, PROJ, LCASH/MCASH/GCASH y SCALEX/SCALEY/SCALEY2. MVT3, PLS6 y SCALEY2 no estaban en la lista, pero son dependencias directas. Respeta las caídas del 6502 (TAS2→NORM, SPS1/SPS4→TAS2, TIS3→DVIDT, TAS3/TAS4→MAD, SCALEY→SCALEY2, LCASH→MCASH) y los acarreos (SQUA2 sale siempre con C=0). TAS5 no existe en esta versión. `make test_state` (incluido en `make test`): `tools/ref6502_state.py` transcribe el 6502 literalmente, `tools/gen_vectors3.py` genera 81.000 casos (27 rutinas × 3.000) y `tests/test_state.c` los compara; 0 fallos, y se comprobó con mutaciones que la prueba detecta cambios de acarreo, signo y ramas.
- **Datos de naves** (`shipdata.c/.h`, generados por `tools/gen_shipdata.py` desde `elite-data.asm`): los 32 planos (cabecera de 20 bytes, vértices de 6, aristas de 4 y caras de 4) en un único bloque `ship_blob` de 7.154 bytes, porque algunos planos comparten aristas o caras con otro mediante desplazamientos relativos de 16 bits (negativos en algunos casos). `SHIP_BP(tipo)` da el plano y `SHIPT_*` los números de tipo (nombres de la fuente).
- **Proyección 3D** (`ship3d.c/.h`): `ship_ll9` (LL9 completo: visibilidad de caras, rotación y proyección de vértices, aristas visibles, explosión y modo "punto" lejano), `ship_ll51`, `ship_shppt`, recorte de líneas `ship_clip`/`ship_clip2` (con LL118, LL120, LL123 y LL129), y el montón de líneas por nave `ship_lsput`/`ship_lsclr` (dibuja la nueva y borra la vieja con XOR). Sigue el flujo del 6502 con `goto`, incluidos los saltos al medio de rutinas. Las líneas salen por `ship_loin_hook` (por defecto `gfx_line` en XOR sobre el área de 256x136); `ship_doexp_hook` y `ship_planet_hook` son los ganchos para la explosión y para planetas y soles, aún sin portar. `ship_c_in` es el acarreo con que se entra a LL9 (lo usa el primer `DORND` de una explosión). `sm_proj` devuelve además A (`sm_a`), que SHPPT usa aunque haya desbordado. `make host_demo_ship` dibuja seis naves en `demo_ship.png`.
- **Verificación contra el código original** (`tools/cpu6502.py`, `asm6502.py`, `orig.py`, `gen_ll9.py`, `tests/test_ll9.c`): un emulador de 6502 y un ensamblador mínimo ejecutan las rutinas originales de la fuente de referencia (no incluida en el proyecto; ver `tools/README_ref.txt`) con entradas aleatorias, y se comparan con el C: INWK, montón de líneas, vértices, caras visibles, semilla aleatoria y la lista exacta de líneas dibujadas. `make test_orig`: 15.000 comprobaciones (LL9, CLIP, CLIP2, LL51 y PROJ), 0 fallos; también 75.000 con otra semilla. Encontró un fallo real en la primera ejecución (la negación de LL52 usaba `1-x` en lugar de `1-x-1`; aparecía en 7 de 3.000 casos). Con mutaciones de una instrucción la prueba detecta los cambios, salvo dos que son equivalentes. LL28 y LL61 se compararon además de forma exhaustiva con el original (65.280 y 32.640 pares), con 0 diferencias.
- **Consola del runtime de c68** (`plat_ql.c`, `ql_ipc.asm`): `plat_init` redefine la ventana de consola a pantalla completa, borde negro y sin cursor (SD.WDEF, SD.CLEAR, SD.CURS), y registra `plat_shutdown` con `atexit`. La cola de teclado de la consola se vacía con IO.FBYTE (timeout 0) al arrancar, en cada `plat_present()` y al salir, porque KEYROW no consume las pulsaciones y SuperBASIC las recibía al salir. `ql_trap3` es un stub genérico de `trap #3`. **Sin probar en QL**.
- **Universo** (`universe.c`): galaxias, sistemas, mercado, distancia, salto galáctico y descripciones. Port de `TXTELITE.C` 1.5 de Ian Bell, que está en `tests/oracle/`. `make test_universe` da 12.288 registros idénticos al oráculo (8 galaxias) y 131.072 distancias correctas.


**Hecho en PC en la sesión del 2026-10-16 (sin verificar contra el original; ver `PENDIENTES.md`)**

- **Comandante** (`cmdr.c/.h`): bloque de 77 bytes (#0..#76) con los desplazamientos `CB_*`, `cmdr_pack`/`cmdr_unpack`, `cmdr_default` (JAMESON, Lave), `CHECK`/`CHECK2` y el sellado como `SVE` (`CHK3`, `CHK`, `CHK2 = CHK ^ $A9`). El comandante de fábrica reproduce las constantes de la fuente (`$03`, `$27`, `$AA`). `make test_save`: 6.244 comprobaciones contra 3.003 vectores de una transcripción literal del 6502 (`tools/gen_vectors_chk.py`), ida y vuelta por fichero y corrupción. **Aviso:** `CHECK`/`CHECK2` del original solo detectan el 57 % de los cambios de un solo bit (cada byte se suma en un paso y se combina por XOR en el siguiente, y a veces se cancelan); no es un fallo del port.
- **Guardar y cargar** (`save.c/.h`): fichero propio de 94 bytes (`ELQL`, versión, semilla, nombre de 8, bloque de 77 cifrado con XOR), nombre `<dispositivo>elite_<nombre>` (`flp1_`, `flp2_`, `mdv1_`, `mdv2_`, o directorio en el PC). `load_commander` valida `CHK` y `CHK3` y marca `cok` bit 7 si falla solo `CHK2`, como `DFAULT`. El juego silencia el sonido (`snd_off`) antes de tocar el disco. **Sin probar en QL:** `fopen`/`fwrite` con nombres QDOS desde c68.
- **Lógica de juego** (`game.c/.h`): `game_new`, `game_after_load`, `game_enter_system`, comercio (`trade_*`: reglas de `tnpr`, existencias que no se reponen al vender, dinero en 32 bits), equipo (`equip_*`: lista y precios de `PRXS`, combustible `(70-QQ14)*2`, laser antiguo reembolsado después de cobrar), rangos (`rating_name`) y estado legal, hiperespacio (`jump_*`, salto galáctico con la semilla `qq21` de la galaxia). `uni_galaxy_seed` en `universe.c`.
- **Entrada** (`input.c/.h`): `in_poll`/`in_wait` con caracteres, cursores y SHIFT+ESC; gancho `in_script` para sesiones guiadas en el PC.
- **Pantallas** (`menu.c/.h`): título con la Cobra Mk III girando (`shipview.c`, LL9 con monton reutilizado), estado, inventario, mercado, compra y venta (lista con cursor, cantidad numérica, `M` = máximo), equipo (con elección de vista para lasers), datos del sistema (con descripción), cartas galáctica y local (mira con cursores, sistema más cercano automático, círculo de combustible), menú de disco (cargar, guardar, cambiar de unidad, reiniciar, salir). Teclas de pantalla como el original: 0 despegar, 1 comprar, 2 vender, 3 equipo, 4 carta galáctica, 5 carta local, 6 datos, 7 mercado, 8 estado, 9 inventario, I disco.
- **Bucle principal** (`app.c`, `main.c`): título → atracado (máquina de pantallas) ↔ vuelo. `make host_game` / `make ql_game`. `make test_game`: 58 comprobaciones de lógica y una sesión guiada de 77 teclas con 14 capturas en `tests/shots/` (se ven con `tools/ql2png.py`).
- **Vuelo (`flight.c`): PROVISIONAL.** Polvo estelar sustituto, panel con velocidad, combustible, dinero y sistemas, cambio de vista (0-3), hiperespacio (`H`, cuenta atrás de 15, requiere sistema objetivo en alcance), salto galáctico (`G`) y atraque (`C`, sin exigir llegar a la estación). No es un port: se sustituirá al portar `STARS`, `MVEIT`, `DIALS`, `SCAN`, `TACTICS`, etc.
- **Demos del QL:** `plat_wait_key()` espera una tecla antes de volver a QDOS (el runtime de c68 borra la pantalla al salir). Lo usan `demo_ship`, `demo_universe` y `test_text`. Sin probar en QL (usa el mismo `KEYROW` de `keys_ql.c`).
- **Correcciones tras probar en el QL (2026-10-16):** el título animado dejaba un caos de píxeles porque `shipview_draw` ponía a 0 el estado de la nave (`INWK+31`, bit 3 = «hay líneas viejas en el montón») en cada fotograma, y LL9 solo borra las líneas viejas con ese bit puesto. El estado ahora va por nave (`slot[I_STATE]`); `make test_shipview` comprueba que animar deja la pantalla igual que dibujar desde cero (falla sin el arreglo). Pendiente de confirmar en el QL. `plat_wait_key` confirmado en el QL por el usuario.
- **Comercio verificado contra el original (2026-10-16):** `tools/gen_trade.py` ejecuta en el emulador el código original de `tnpr`, el tramo de compra de `TT219`, el de venta de `TT208`, `GCASH`, `LCASH` y `MCASH` (con las rutinas de pantalla sustituidas por stubs y un preprocesador de `IF`/`ELIF`); `make test_trade` (20.000 casos por rutina) lo compara con `trade_fits`/`trade_buy`/`trade_sell`: 0 discrepancias en el dominio alcanzable y 4 mutaciones detectadas. **Hallazgos del original que se reproducen** (`TRADE_FAITHFUL 1` en `game.h`, por defecto; con 0 se corrigen): `tnpr` suma en 8 bits, así que en kg/g con 193 o más unidades de un artículo en bodega, comprar 256 menos las que llevas (hasta 63) da la vuelta y se acepta (la cantidad de bodega también da la vuelta); y `GCASH` calcula el importe en 16 bits, de modo que una venta de más de 6.553 Cr de una vez pierde los bits altos (p. ej. 200 kg de platino). La diferencia que había declarado como intencionada ya no lo es.
- **Refactor:** `shipview.c` (orientación y dibujo de una nave a partir de ángulos) sale de `demo_ship.c`; la captura del demo es idéntica byte a byte a la anterior.

**Decisiones de diseño**

- `uni_distance` sigue el algoritmo del 6502 (`dx²+(dy>>1)²`, `LL5`, ×4, con saturación), no el `sqrt` flotante de Text Elite, que solo coincide en el 14 % de los pares.
- El salto galáctico va al sistema más cercano a (0x60,0x60).

**Memoria**

- Pantalla en `$20000` (32 KB). No hay mapa de memoria del juego todavía. `uni_galaxy[256]` ocupa unos 8 KB. K% (`ks`) ocupa 333 bytes (9 ranuras de 37).

**Sin portar**

- IA (`TACTICS`), combate, ataque con lasers y misiles, generación de naves y encuentros, atraque real, ECM, bombas, cápsula de escape, misiones, el original de `STARS`/`MVEIT`/`DIALS`/`SCAN` (hoy hay un sustituto), y el resto de las 528 rutinas (ver `INVENTARIO.md`). De la proyección 3D faltan la explosión (`DOEXP`), planetas y soles (`PLANET`, `SUN`, `CIRCLE`), el polvo estelar, el panel (`DIALS`), el radar (`SCAN`) y el movimiento de naves (`MVEIT`). Tampoco está el comandante por defecto, y `LOIN` es el `gfx_line` propio, no la transcripción del original.

## 3. Adaptación de hardware QL

**Gráficos (modo 8)**

- Cada palabra son 4 píxeles. Para el píxel k (0 a 3, izquierda a derecha), con `s=2*(3-k)`: byte par con verde en el bit `s+1` y flash en el bit `s`; byte impar con rojo en el bit `s+1` y azul en el bit `s`.
- Cada línea ocupa 128 bytes. Colores: bit2=Verde, bit1=Rojo, bit0=Azul.
- Se dibuja directamente en pantalla. El backend de PC usa el mismo formato (`tools/ql2png.py` lo convierte a PNG).

**Teclado**

- `KEYROW` por IPC (comando 9), una llamada por fila, filas 0 a 7. Matriz verificada con el manual de SuperBASIC.
- Bloque `MT.IPCOM` de 8 bytes `9,1,0,0,0,0,<fila>,2`, con la respuesta en D1.B. Layout: byte 0 comando, byte 1 número de parámetros, bytes 2 a 5 palabra de bits por parámetro (32 bits), después los parámetros y el byte de respuesta.
- **Joystick:** en el QL son teclas. CTL1 son los cursores y el espacio (fila 1); CTL2 son F1 a F5 (fila 0). Con CTL1 el espacio es disparo y acelerar pasa a ENTER.
- **Controles de Elite:** `/` frenar, espacio acelerar, `,` y `.` alabeo, `X` y `S` cabeceo, `A` láser, `B` bomba, `ESC` cápsula, `T`/`U`/`M` misil, `E` ECM, `J` salto, `C` y `P` atraque. Las 16 teclas están en `ctl_read()`.
- **Pendiente de comprobar:** la disposición UK para menús, que es provisional. CTRL+Izq da el carácter 8.

**Sonido**

- `BEEP` por IPC, comando 10 con 10 parámetros, bloque de 17 bytes con bits-por-parámetro `$00000AAA`. `BEEP OFF` es el comando 11.
- **Fórmulas:** pitch ≈ 0,022434·ciclos − 10,6 y duración en unidades de \~72 µs. Una nota del manual de sQLux apunta a \~43,6 µs reales; `snd_unit_us` no está implementada todavía.
- **Efectos:** todos los del original están traducidos desde los bucles del 6502, con prioridades. `snd_enable(0)` equivale a DNOIZ.
- **Sin oír todavía:** el ruido y los barridos son aproximaciones. `SND_NOISE_FUZZ` y `SND_NOISE_RND` se ajustan a oído, y el signo de los barridos hacia arriba es hipótesis.

## 4. Restricciones y reglas clave

- **Hardware:** 68008 a 7,5 MHz con bus de 8 bits y sin FPU, así que nada de `float`/`double` en el juego. RAM objetivo de hasta 640 KB; el mínimo se decidirá cuando haya memoria real.
- **C89 puro:** sin `stdint.h`, sin declaraciones mezcladas ni dentro de `for`. Usar `u8` (`typedef unsigned char`) y `u16`. `int` puede ser de 16 bits en c68, así que se usa `long` para valores mayores de 32767 (por ejemplo `SCR_BYTES = 32768L`).
- **Compilación:** `qcc -O -Isrc -I/usr/local/share/qdos/include ...`. `-Isrc` es obligatorio porque `qcc` busca los `#include "..."` en el directorio actual.
- **as68:** familia Sozobon. Directivas `.globl` y `.text`, comentarios con `;`, etiquetas con dos puntos. Los ficheros deben llamarse `.asm`; `qcc` no reconoce `.s`. No valen `xdef`, `section` ni los comentarios con `*`. Compilado y verificado localmente. Se puede reconstruir `as68` clonando `github.com/stronnag/xtc68` y compilando `src/as68` con `-DXTC68 -DMINIX -DLABELDIFF -DUSELINE -DHOST_LITTLE_ENDIAN`.
- **Trap #3:** `ql_trap3(d0,d1,d2,d3,a0,a1)` en `ql_ipc.asm` carga los registros, ejecuta `trap #3` y devuelve D0 (0 = ok, negativo = error QDOS). Códigos usados: IO.FBYTE=1, SD.WDEF=$0D, SD.CURE=$0E, SD.CURS=$0F, SD.CLEAR=$20. De `qdos.h` solo se usan `fgetchid` y `mt_dmode`.
- **IPC:** un `BEEP` nuevo cancela el anterior, y no hay que sonar mientras se carga o descarga un job. La lectura de teclado por IPC es lenta, así que conviene leer solo las filas necesarias con `key_set_rows`. `KEYROW` lee el teclado físico aunque otra ventana tenga el foco, y tres teclas en un rectángulo de la matriz generan una fantasma.
- **Datos de naves:** extraerlos con un script desde `elite-data.asm`, no a mano.

## 5. Próximos pasos (backlog)

1. ~~**Estado del juego** y rutinas aritméticas dependientes~~ **Hecho** (ver sección 2). No hay `ARCTAN` ni `TAS5` en esta versión. Queda el comandante por defecto (los datos de la fuente dependen de la opción de build, así que habrá que extraerlos con script) y la tabla de precios `QQ23`.
2. **Proyección 3D:** ~~`LL9`, recorte de líneas, dibujo de naves~~ **Hecho** (ver sección 2). Pendiente: `DOEXP` (explosiones), planetas y soles (`PLANET`, `SUN`, `CIRCLE`), polvo estelar (`STARS`), panel (`DIALS`), radar (`SCAN`) y `MVEIT`.
3. **Bucle principal y resto del juego:** ~~bucle principal, menús, cartas, guardado y carga por ficheros QDOS~~ **Hecho en el PC (sin verificar)**, ver sección 2. Falta portar (sin verificar, anotando en `PENDIENTES.md`): `STARS` (+`STARS1/2/6`, `NWSTARS`, `MLS1/2`, `MUT2`, `PIX1`, `PIXEL2`), `MVEIT` (9 partes) y movimiento, `DIALS`, `SCAN`, `TACTICS` (7 partes), `DOEXP`, `PLANET`/`SUN`/`CIRCLE`, generación de naves (`NWSHP`...), láseres y misiles, atraque, ECM y el resto del inventario. `snd_tick` ya se llama desde `in_poll` (20 ms por fotograma, provisional).
4. **Rendimiento:** perfilar y pasar a ensamblador `LOIN`/`LL9` si hace falta. LL9 en C solo se ha medido en PC; en el 68008 a 7,5 MHz no hay medida todavía.
5. **Validar en QL real o emulador** (prioridad: compilar `make ql_game` con qcc y arrancarlo; los módulos nuevos `cmdr.c`, `save.c`, `game.c`, `menu.c`, `flight.c`, `input.c`, `app.c`, `shipview.c` no se han compilado con c68 todavía) (nuevo: que `fgetchid(stdin)` devuelva el canal correcto, que no aparezca el marco rojo ni queden pulsaciones al salir, y que `ql_trap3` funcione) (QPC2, uQLx, sQLux), porque todo lo de QL está probado solo en PC: `plat_ql.c` (`mt_dmode`, escritura directa en `$20000`, posible modo supervisor), `keys_ql.c`, `sound_ql.c` y `ql_ipc.asm`. Demos: `make ql_keys`, `make ql_sound`, `make ql_universe`, `make ql_ship`. Confirmado por el usuario en QL: gráficos y texto, teclado, sonido y el demo de universo (muestra la pantalla con la información del planeta y sale). Sin confirmar todavía: el arreglo de la consola (`plat_ql.c`), `ql_trap3` y `make ql_ship` (que además compila por primera vez los módulos nuevos con c68: `state.c`, `ship_math.c`, `ship3d.c`, `shipdata.c`).
6. **Pruebas contra el original:** la lista viva está en `PENDIENTES.md` (se hace cuando el juego compile en el QL, según la estrategia de la sección 1). Ya hecho para la proyección 3D con el emulador de 6502 (sección 2). Pendiente aplicarlo a lo anterior: `ship_math.c` (TAS2, NORM, TIDY, MAS1 a MAS4, SPS1 a SPS4, LCASH...) y `emath.c`, cuyas pruebas siguen apoyándose en transcripciones literales propias (LL28 y LL61 ya están comprobadas contra el original).
7. **Ajustar a oído:** los parámetros de sonido y la disposición del teclado UK.
