# Decisiones y desviaciones del original

Movido desde `CLAUDE.md` (sección 9).

## 9.1 Criterio de «rutina no necesaria»:
- *Anticopia y arranque del Apple II:* `ENTRY`, `S%`, `DEEOR`, `DEEORS`, `G%`, `F%` (suma de comprobación y descifrado del código), `NMIpissoff`, `COLD`, `BEGIN`, `MUTILATE`/`UNMUTILATE` (cifrado antimanipulación del fichero de comandante; `save.c` tiene su propio cifrado), `SWAPPZERO`.
- *Lo que da el compilador/runtime de C:* categoría **Utility routines** (rellenos de memoria, copias de páginas, retardos por bucles, manejador de errores `BRK`), **Workspaces** (`ZP`, `XX3`, `K%`, `WP`, `Disk`, `UP`, `Option`: son variables de C) y **Tube** (`newosrdch`).
- *Hardware del Apple II/BBC/C64:* tablas de la pantalla hires (`SCTB*`, `cellocl`, `TWFL/TWFR`, `MASKT`), cambio de modo (`HGR`, `TEXT`), impresión en modo texto (`CHPR2`, `R5`, `RR5`), altavoz (`CLICK`), registrador de teclas, sprites de los Trumbles (VIC).
- *DOS 3.3 / RWTS:* catálogo, sectores, pistas, nibbles, brazo del disco (QDOS gestiona el disco).
- *Sin uso en el original* (`MUT3`, `LASNOISE2`, `beamcol`, `DKSANYKEY`, `KTRAN`).

## 9.2 Decisiones
- **Port fiel con partes propias donde el hardware lo exige:** `LOIN` es la transcripción (`loin_draw`, desde la 16.ª ronda; antes era Bresenham); `HLOIN` = `gfx_hloin` y `VLOIN` = `gfx_vloin` (misma cobertura que el original, color liso en lugar de los patrones de bits del Apple); los colores del radar cian y magenta (blancos en el Apple II) se mantienen; las cartas siguen siendo pantallas propias (`menu.c`: título en una barra azul, información del objetivo abajo, pasos de la mira de 1 o 2 por fotograma), pero lo que se dibuja es el original; el panel usa el bitmap original convertido a modo 8; el fichero de partida es propio (ELQD, 94 bytes).
- **Montones de líneas** por ranura (256 bytes) en lugar del reparto con `SLSP`; `K%` con 9 ranuras.
- **Estación y planeta:** las posiciones del original (`TT110` al despegar, `SOLAR` al llegar, parte 14 y `KS4`); `SOLAR` y `NWSPS` completos.
- **Teclas de prueba** `N`, `K` y `C` sin ordenador: solo en las pruebas (`flight_testkeys`).
- **`JSTGY`** empieza en $FF (eje Y normal), como el disco IB; la versión de la fuente lo deja a 0. **`K` (`JSTK`)** activa el joystick CTL1 del QL (cursores y espacio, que ya eran los controles). El título muestra los autores solo con `PATG` (tecla `X` en pausa), como el original.
- **Universo y mercado** vienen de Text Elite (verificado contra su oráculo, no contra el 6502); `uni_distance` sigue el 6502.
- **Textos de los menús como cadenas C** (salvo la pantalla de datos del sistema, que es la `TT25` del original); las pantallas de las misiones y las descripciones especiales usan el **motor de fichas original** (`detok.c`, con las tablas de la fuente). La descripción normal sigue con Text Elite.
- **Sonido** traducido desde los bucles del 6502 a `BEEP` del QL: aproximado.
- **`TRADE_FAITHFUL 1`**: se reproducen dos peculiaridades aritméticas del original.
- **Paso fijo de vuelo (10 FPS como máximo):** toda la simulación cuenta por fotograma (naves, IA, `MCNT`, energía, láser, giro `ALPHA`/`BETA` y velocidad) y el original iba a pocos FPS, así que en máquinas rápidas (Super Gold Card ~20 FPS, PC ~50) el juego iba demasiado deprisa y el giro era brusco. `flight_loop` espera hasta `FRAME_TICKS` (5 cuadros de 50 Hz) desde el fotograma anterior (`flight.c`); sin reloj (`plat_ticks() = 0`) o con guion no espera. Con menos de 10 FPS no cambia nada. Se descartó escalar solo el mando (`jstx`/`jsty`, `alpha`/`beta` por tiempo): el mundo seguía yendo más rápido, y escalar toda la simulación rompe la fidelidad con el 6502. Para otro ritmo, cambiar `FRAME_TICKS` (2 = 25 FPS).
- **Túnel de lanzamiento e hiperespacio con ritmo propio:** los anillos (`hfl1`, `sun.c`) se dibujan seguidos; en el QL real se veían salir uno a uno por la lentitud del dibujo. `sun_pace_hook` (`ring_pace` en `flight.c`) espera `RING_TICKS` (2 cuadros) por anillo, y `wait_frames` espera por tiempo (`TUNNEL_FRAMES` × `TUNNEL_TICKS`) y no por vueltas de `in_poll`. Sin reloj o con guion no espera.
- **Teclas** repartidas para el teclado del QL (`docs/HARDWARE_QL.md`, Teclado), distintas de las del Apple II.

## 9.3 Mejoras propias que se han quitado al verificar contra el original (por si interesa recuperarlas)

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
| **Lineas con Bresenham** (`gfx_line`): dibujaban los dos extremos y una pendiente exacta | `LOIN`: pendiente con logaritmos y sin uno de los extremos (`docs/HALLAZGOS.md`, `LOIN`) | `gfx_line` sigue en gfx.c; bastaria volver a llamarla desde `default_loin` (ship3d.c), `loin()` (sun.c), `las_pair` (combat.c) y `bomb_off` (equip.c) | 16 |
| **Brazo vertical de las cruces de 1 punto** (TT15, mira) y linea bajo el titulo de las misiones de 0 a 255 | `VLOIN` = `CPIX` de 2 puntos de ancho; `NLIN4` es `HLOIN` de 0 a 255, que cubre 0..253 | `vloin()` de chart.c era `gfx_line`; `nlin4` de missions.c era `gfx_hline(0, 255, 19)` | 16 |
| **Los sonidos no gastaban numeros aleatorios** (la partida no dependia de tener el sonido activo) | `SOEXPL`, `SOHISS` y `SOBOMB` llaman a `DORND` en cada clic si el sonido esta activo | quitar `dornds()` de sound.c (o no poner `snd_dornd_hook`) | 16 |
| **Mira de las cartas pegada al sistema mas cercano** y limitada a la parte visible de la carta local | `TT16`/`TT123`: la mira va libre (QQ9, QQ10) y solo `D` (`hm`) o el salto la llevan al sistema (`TT111`) | `scr_chart` de menu.c: `cx`/`cy` con `clampi` y `game_set_target(uni_nearest(...))` en cada paso | 17 |
| **Vista del espacio sin titulo ni linea superior** (solo el pie de pantalla propio con la vista) | `TTX66`: «Front View» (o Rear, Left, Right) arriba y `BOX` (linea azul en y = 0) | `clear_view()` en lugar de `space_tt66()` en flight.c | 17 |
| **Puntos de la carta galactica de un solo pixel** | `PIXEL` con ZZ = 255: dos puntos | `gfx_pixel` en lugar de `gfx_dust(x, y, 255)` en `tt22` | 17 |
| **Sin limite en `PDESC`** (se mantiene): el port recorre solo las 26 entradas de `RUPLA` | `NRU% = 0`: recorre 256 posiciones de memoria ajena y puede colgarse | sigue asi en `missions.c` (no se ha quitado: es una correccion) | 13 |

