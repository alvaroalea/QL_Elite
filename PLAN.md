# Elite QL - plan de port (version disco, hasta 640 KB)

Referencia: https://elite.bbcelite.com/apple/ (Apple II, Firebird 1985). Codigo QL escrito desde cero, sin copiar comentarios.

## Decisiones
- Pantalla: modo 8 (256x256). Vista espacial 256x136 centrada en (128,68), como en el Apple II; el panel inferior aprovecha las filas extra.
- Se dibuja directo en el formato QL (gfx.c). Backend PC con el mismo formato para probar la logica sin hardware.
- Logica en C (c68); bucles criticos (lineas, proyeccion 3D) pasaran a ensamblador 68000 tras perfilar.
- Disco: guardado/carga de comandante en fichero QDOS (flp1_/mdv1_), sustituye a rwts.

## Capa de plataforma (hecha, probada en PC)
pixel, xor_pixel, get, line (normal/XOR), hline, rect, clear.

## Texto (hecho, probado en PC)
text.c: celdas 8x8 (32x32), tinta/papel/transparente, \n \r \b, ajuste de linea, borrado de filas, cursor de bloque XOR con parpadeo. Fuente de dominio publico (font8x8_basic, generada con tools/gen_font.py).

## Aritmetica (hecha salvo lo que depende del estado del juego; probada en PC)
emath.c: DORND, MULTU/MU11, SQUA, MULT1/MULT12, MAD, ADD, MULTS, FMLTU/FMLTU2 (tablas log y senos), MLTU2, MLU1/MLU2, MULT3, TIS1/TIS2/DVID96, LL5 (raiz cuadrada), LL28/LL61 (division por logaritmos), LL38, DVID4/DV41/DV42, DVIDT, DVID3B. `make test` compara ~895.000 casos con una transcripcion literal del 6502.
Las que dependen del estado del juego estan en ship_math.c (ver la seccion siguiente).

## Estado del juego y aritmetica dependiente (hecho en PC, probado)
- `state.h/state.c`: bloque de nave INWK (37 bytes; desplazamientos `I_*`), tabla K% (`ks`, NOSH+1 ranuras de NI bytes: una de resguardo porque UNIV tiene NOSH+1 entradas), `univ[]` (UNIV), `frin[]` (FRIN, NOSH+1), `many[]` (MANY; `sspr` = many[SHIP_SST]), junk/auto/ecmp/mj, espacio de trabajo de la pagina cero (P+2, K3/K4 contiguos en `zk3[16]`, XX15, XX12, XX16) y las variables QQ* y del comandante con los tamanos del original (CASH en big-endian). `state_clear()` pone todo a cero e inicializa UNIV. K, P, P+1, Q, R, S, T, U siguen en emath (zK, zP, zP1...).
- `ship_math.c`: MVT3, MAS1-MAS4, TAS1, VCSUB/VCSU1, TAS2, TAS3, TAS4, TAS6, NORM, TIS3, TIDY, SPS1-SPS4, PLS6 y PROJ (PLS6 y MVT3 no estaban en la lista pero son dependencias directas), LCASH/MCASH/GCASH, SCALEX/SCALEY/SCALEY2. Respeta las caidas entre rutinas del 6502 (TAS2 -> NORM, SPS1/SPS4 -> TAS2, TIS3 -> DVIDT, TAS3/TAS4 -> MAD, SCALEY -> SCALEY2, LCASH -> MCASH) y los acarreos de entrada/salida (p. ej. SQUA2 sale siempre con C = 0). TAS5 no existe en esta version.
- Prueba: `make test_state` (parte de `make test`): tools/ref6502_state.py transcribe el 6502 literalmente, tools/gen_vectors3.py genera 81.000 casos (27 rutinas x 3.000, con entradas aleatorias y valores limite) y tests/test_state.c los compara. Verificado con mutaciones: la prueba detecta cambios de un bit en acarreos, signos y ramas.
- Limites conocidos: V (puntero de VCSUB/TAS1) se pasa como puntero; los offsets Y de MAS2/MAS3 son 0 o NI (ranuras 0 y 1), como en el original; las transcripciones del 6502 son propias, no el binario (ver pruebas contra el original).

## Teclado y joystick (hecho en PC; backend QL sin probar)
keys.c: matriz KEYROW por IPC (comando 9), teclas simultaneas, flancos, cola de caracteres (UK, provisional), joysticks CTL1 (cursores+espacio) y CTL2 (F1-F5), controles de vuelo de Elite. `make test_keys` (27 pruebas). Backend QL: keys_ql.c + ql_ipc.s (trap #1 MT.IPCOM). `make ql_keys` genera una demo interactiva para validar en el QL.

## Sonido (hecho en PC; backend QL sin probar)
sound.c: efectos del original (SOBEEP, SOBLIP, SOEXPL, SOHISS, SOBOMB, LASNOISE, BEEP, BOOP, BELL, EXNO*, DEATH) traducidos a BEEP del IPC (comando 10) con los tiempos del 6502; BEEP OFF (11); prioridades para que un efecto menor no pise uno mayor; snd_enable = DNOIZ. `make test_sound` (26 pruebas). `make ql_sound` genera una demo para ajustar de oido (SND_NOISE_FUZZ/RND). Ruido y barridos son aproximaciones: el pitch del QL no es lineal y no se ha podido oir.

## Universo (hecho en PC, probado contra el oraculo)
universe.c: galaxias, sistemas, mercado, distancia, hiperespacio galactico y descripciones, portados de TXTELITE.C 1.5 (Ian Bell). `make test_universe` compila el TXTELITE.C original como oraculo y compara 8 galaxias: sistemas, descripciones y mercados identicos (12.288 registros); la distancia sigue el algoritmo del 6502 (verificada contra una version independiente en 131.072 pares), no el calculo en coma flotante de Text Elite. Demo: `make host_demo_universe` / `make ql_universe`.

## Pendiente
1.  (texto completado)   2. Teclado (IPC, rollover) y joystick.   3. Sonido (BEEP/IPC).
4. Matematica y aleatorios (DORND, MULTU, divisiones).   5. Proyeccion 3D y dibujo de naves (LL9).
6. Datos: naves, textos, galaxias, mercado.   7. Bucle principal, tacticas, combate.   8. Menus y fichero.

## Consola del runtime de c68 (QL)
- `plat_ql.c`: `plat_init` redefine la ventana de consola a pantalla completa, borde negro y sin cursor (SD.WDEF/SD.CLEAR/SD.CURS) y registra `plat_shutdown` con `atexit`.
- La cola de teclado de la consola se vacia con IO.FBYTE (timeout 0) al arrancar, en cada `plat_present()` y al salir, para que SuperBASIC no reciba las teclas. `ql_trap3` (en `ql_ipc.asm`) es el stub generico de trap #3.
- Pendiente de validar en QL real/emulador (QPC2, uQLx, sQLux): que `fgetchid(stdin)` devuelva el canal correcto y que no queden pulsaciones al salir.

## Proyeccion 3D (hecho en PC, verificado contra el original)
- `shipdata.c/.h` (generado por tools/gen_shipdata.py): 32 planos de nave en un bloque; algunos comparten aristas/caras con otro plano (desplazamientos de 16 bits con signo), por eso se construyen punteros con `bp_ptr`.
- `ship3d.c`: LL9, LL51, SHPPT, CLIP/CLIP2 (LL118/120/123/129), LSPUT/LSCLR. Ganchos: `ship_loin_hook` (por defecto gfx_line XOR), `ship_doexp_hook`, `ship_planet_hook`. `ship_c_in` = acarreo de entrada a LL9.
- Verificacion: `make test_orig` ejecuta el codigo original en un emulador de 6502 (ver tools/README_ref.txt). Pendiente: DOEXP, PLANET/SUN/CIRCLE, STARS, DIALS, SCAN, MVEIT; aplicar la misma verificacion a ship_math.c y emath.c.
- Demo: `make host_demo_ship` (PC) y `make ql_ship` (QL, sin probar).
