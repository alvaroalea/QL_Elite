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
Pendiente (leen o escriben el estado del juego: INWK, K%, etc.): TIS3, TAS1-TAS6, MAS1-MAS4, VCSU1/VCSUB, SPS1-4, LCASH/MCASH/GCASH, PROJ, TIDY, NORM, SCALEX/Y. No hay ARCTAN en el codigo de esta version.

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
