# Elite QL - plan de port (version disco, hasta 640 KB)

Referencia: https://elite.bbcelite.com/apple/ (Apple II, Firebird 1985). Codigo QL escrito desde cero, sin copiar comentarios.

## Decisiones
- Pantalla: modo 8 (256x256). Vista espacial 256x136 centrada en (128,68), como en el Apple II; el panel inferior aprovecha las filas extra.
- Se dibuja directo en el formato QL (gfx.c). Backend PC con el mismo formato para probar la logica sin hardware.
- Logica en C (c68); bucles criticos (lineas, proyeccion 3D) pasaran a ensamblador 68000 tras perfilar.
- Disco: guardado/carga de comandante en fichero QDOS (flp1_/mdv1_), sustituye a rwts.

## Capa de plataforma (hecha, probada en PC)
pixel, xor_pixel, get, line (normal/XOR), hline, rect, clear.

## Pendiente
1. Texto: fuente propia + CHPR/cursor.   2. Teclado (IPC, rollover) y joystick.   3. Sonido (BEEP/IPC).
4. Matematica y aleatorios (DORND, MULTU, divisiones).   5. Proyeccion 3D y dibujo de naves (LL9).
6. Datos: naves, textos, galaxias, mercado.   7. Bucle principal, tacticas, combate.   8. Menus y fichero.
