# Pendientes de verificacion (lista viva)

Estrategia acordada (2026-10-16):

1. Se porta **todo** el juego a C **sin** verificar cada rutina contra el original mientras tanto.
2. Cada funcion nueva se anota aqui (y en `tools/ported.txt` / `INVENTARIO.md`) con su rutina de origen y como se verificara.
3. Cuando haya una version que **compile y arranque en el QL**, se empieza a verificar esta lista contra el emulador de 6502 (`tools/cpu6502.py`, `tools/orig.py`).
4. El orden de la lista lo cambia el **feedback del QL real**: lo que el usuario vea raro sube de prioridad.

Prioridad inicial: **A** = visible de inmediato en el QL y afecta a la partida; **B** = afecta a la partida pero se nota menos; **C** = presentacion.
Estado: `sin-verificar` | `transcripcion` (comparada con una transcripcion literal propia en Python, falta el emulador) | `verificada`.
Resultado de las pruebas propias en el PC (`make test_game`, `make test_save`): solo prueban que el C hace lo que se quiere, **no** que sea igual al original.

## Lista

| # | Funcion C | Rutina original | Que comparar con el 6502 | Prio | Estado |
|---|---|---|---|---|---|
| 1 | `trade_fits` (y `trade_room`, sin desbordamiento) | `tnpr` | **Verificada (2026-10-16)** con el 6502 original: 20.000 casos (`make test_trade`), 0 discrepancias, 4 mutaciones detectadas. Con `TRADE_FAITHFUL=1` (por defecto) se reproduce la suma de 8 bits del original: en kg/g `198 + 63` da la vuelta y el original dice que "cabe" | A | verificada |
| 2 | `trade_buy`, `trade_sell`, `trade_max_buy` | `TT219`, `TT208` (+ `GCASH`/`LCASH`/`MCASH` ejecutados en el emulador) | **Verificada (2026-10-16)**: dinero, bodega y existencias tras comprar/vender coinciden en todo el dominio alcanzable (cantidad <= existencias o <= bodega, que es lo que deja pedir `gnum`). El importe de 16 bits de `GCASH` se reproduce (`TRADE_FAITHFUL=1`). Las rutinas `sm_gcash`/`sm_lcash`/`sm_mcash` siguen en `transcripcion` (el test ejecuta las originales, no esas) | A | verificada |
| 3 | `trade_price` y mercado (`uni_gen_market`) | `TT151`, `var`, `GVL` | Precio y existencias para todas las combinaciones sistema/`QQ26`; la comparacion con `TXTELITE.C` ya existe pero no con el 6502 | A | sin-verificar |
| 4 | `equip_buy`, `equip_price`, `laser_refund`, `equip_count` | `EQSHP`, `PRXS`, `refund`, `qv` | Dinero, equipo y lasers tras cada compra; articulos ofrecidos por nivel tecnologico; cobra antes de reembolsar el laser antiguo | A | sin-verificar |
| 5 | `cmdr_check`, `cmdr_check2` | `CHECK`, `CHECK2` | Ya coinciden con una transcripcion literal (3.003 bloques) y con las constantes de fabrica `$03`/`$27`; falta el emulador | B | transcripcion |
| 6 | `cmdr_default`, `cmdr_unpack`, `cmdr_pack`, `cmdr_seal` | `JAMESON`, `DFAULT`, `SVE` | Bloque de 77 bytes campo a campo; `cok` bit 7 y bit 3; `svc >> 1` al guardar | B | sin-verificar |
| 7 | `rating_name`, `legal_name` | `STATUS` (`st4`, `st3`, `st5`) | Rango segun `TALLY` (umbrales 8, 16, 32, 64, 128, 512, 2560, 6400) y estado legal segun `FIST` | B | sin-verificar |
| 8 | `jump_do`, `jump_possible`, `jump_distance`, `jump_galactic`, `game_enter_system` | `TT18`, `TT111`, `hyp`, `TT114` | Combustible gastado, sistema de llegada, semilla de galaxia tras el salto galactico, `QQ26` y mercado nuevos | B | sin-verificar |
| 9 | `uni_nearest` | `TT111` | Sistema mas cercano a un punto para todos los puntos (la comparacion con el oraculo de Text Elite no cubre esta funcion) | B | sin-verificar |
| 10 | `chart_draw`, `chart_cursor` (`chart.c`: cartas galactica y local) | `TT22`, `TT23`, `TT14`, `TT15`, `TT103`, `TT105` | **Portado el 2026-10-16** con la geometria de la fuente (galactica de 192 x 96 con marco, circulo de combustible y cruz propia de tamano 7; local con 3 px por unidad en x y 1,5 en y, solo sistemas a menos de 7,0 AL con fila de texto libre, radio del planeta `(w2hi AND 1) + 2 + C`, con el acarreo de `cpl` calculado con las semillas). Comparar con el 6502: posiciones de los puntos y de los nombres, fila de cada nombre, tamano de cada planeta y la secuencia de segmentos de las cruces. **Marco de la galactica corregido (2026-10-16, tras la prueba en el QL):** `HLOIN` enmascara X1 y X2 a pares y cubre X1..X2-1 (alinea la linea inferior y centra los brazos de las cruces), el marco hereda el azul de `NLIN2` y sus verticales son de 2 puntos. La linea bajo el titulo va de 0 a 255 como en el original (`CHART_TOP_FULLWIDTH` en `chart.h`; con 0 solo cierra el marco). Diferencias conocidas: movimiento de la mira propio. El short range lo dio por bueno el usuario | A | sin-verificar |
| 11 | `scr_data` | `TT25`, `TT24` | Texto y formato de la ficha de sistema (poblacion en miles de millones, productividad, radio) | C | sin-verificar |
| 12 | `scr_status`, `scr_inventory`, `scr_market` | `STATUS`, `TT213`, `TT167` | Contenido y orden de las lineas (la maquetacion es propia) | C | sin-verificar |
| 13 | `flight_run` (polvo, panel, hiperespacio, atraque) | `STARS`, `DIALS`, `hyp`, `main loop` | **Sustituto provisional**: no es un port. Polvo ya dependiente de la vista (delante/atras/izquierda/derecha) y barra de velocidad actualizada al momento (2026-10-16), pero no es `STARS`. Se sustituira al portar `STARS`, `MVEIT`, `DIALS`, `SCAN`, `TACTICS`... | A | propia |
| 14 | `save_commander`, `load_commander`, `save_encode/decode` | `SVE`, `LOD`, `MUTILATE`, `UNMUTILATE` | **Formato propio** (94 bytes con cifrado XOR). Solo debe comprobarse que los tres checksums y el bloque de 77 bytes coinciden con los del original | C | propia |
| 15 | `plat_wait_key` (QL) | - | **Confirmado en el QL real por el usuario (2026-10-16)** | A | verificada |
| 16 | `shipview_draw` (titulo animado) | `LL9` + `LSPUT`/`LSCLR` | **Corregido (2026-10-16)**: el estado de la nave (`INWK+31`, bit 3: hay lineas en el monton) se ponia a 0 en cada fotograma y LL9 no borraba las viejas (caos de pixeles en el QL). Ahora el estado va por nave. `make test_shipview` (animar = dibujar desde cero, 6 naves; falla sin el arreglo). Falta confirmar en el QL | A | sin-verificar |
| 17 | `plat_exit`, `ql_trap1` (QL) | - | Salida sin "Press a key to exit" con `MT.FRJOB`. Comprobar en el QL: que no aparezca el mensaje, que vuelva a SuperBASIC y que no queden pulsaciones | A | sin-verificar |
| 18 | Aritmetica de `ship3d.c`/`ship_math.c` bajo c68 | - | **Corregido (2026-10-16)** el fallo de c68 `-O` (flags del `move.b` usados para `sge`); `make c68_check`: 32 naves x 6 poses y una sesion completa iguales que en el PC. Falta confirmar en el QL real (`ship_ql`: las 6 naves; titulo) | A | sin-verificar |
| 19 | `sun_draw`, `sun_chkon`, `sun_wpls`, `sun_flflls`, `edges`, `hloin2` | `SUN` (4 partes), `CHKON`, `WPLS`, `FLFLLS`, `EDGES`, `HLOIN2` | Transcripcion literal con acarreos. Comparar con el 6502 (con `HLOIN`/`LL5`/`DORND` sustituidos por stubs que registran las llamadas): la lista exacta de lineas (X1, X2, Y1) y el contenido final de `LSO` para muchos (K3, K4, K, SUNX) | A | sin-verificar |
| 20 | `circle_draw`, `circle2_draw`, `bline`, `circle_wpls2` | `CIRCLE`, `CIRCLE2`, `BLINE`, `WPLS2` | Transcripcion literal; usa `ship_clip` (LL145, ya verificada) y `em_fmltu2`. Comparar la lista de segmentos (LOIN) y los montones `LSX2`/`LSY2`/`LSP` para muchos (K, K3, K4, STP) | A | sin-verificar |
| 21 | `chart_hm`, `chart_ping`, `chart_find_system` (+ `edit_text`, tecla F/D/O en `scr_chart`) | `hm`, `ping`, `HME2` | Portados el 2026-10-16. Comparar con el 6502: sistema que elige `hm` desde cualquier cruz, `ping`, y que la busqueda por nombre de `HME2` encuentra los mismos sistemas (nombre completo, sin mayusculas) y rechaza los demas. `F` solo atracado, como el original | B | sin-verificar |
| 22 | `key_ctx_menu`, `key_ctx_flight`, `key_set_scan`, `ctl_read` (cursores) | `DOKEY`, `DK4`, `TT17`, `RDKEY` | **Prioridad A (rendimiento en el QL real).** Menus: 3 lecturas de KEYROW por fotograma (filas 1 y 7 + una rotando); vuelo: 4 (filas 1, 4, 7 + una de 0, 2, 3, 6); la fila 5 no se lee. Cursores = alabeo y cabeceo y joystick CTL1. Comprobar en el QL: que va mas rapido, que no se pierden teclas de las filas rotantes (4, 5, C, B, M, G, E, T, U, 0, 2) y que el cabeceo no esta invertido | A | sin-verificar |
| 23 | `uni_goods`, `uni_gen_market` (datos de mercado) | `QQ23`, `ITEM`, `var` | La tabla de mercancias viene de Text Elite y el precio de `uni_gen_market`; comparar con la tabla del 6502 y con `var`/`TT151` para todas las combinaciones de economia y `QQ26` | B | sin-verificar |

## Reglas para esta lista

- Al portar una rutina nueva, se anade una fila (o se amplia una existente) y una linea en `tools/ported.txt`; luego `python3 tools/inventory.py`.
- Para verificar una fila: ejecutar la rutina original en `tools/orig.py` con entradas aleatorias y valores limite, comparar con el C, probar con mutaciones de una instruccion (como se hizo con `LL9`) y pasar la fila a `verificada`.
- Si en el QL real algo falla o se ve distinto, anotar aqui la fila afectada y subirla de prioridad.
