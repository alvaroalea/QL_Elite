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
| 1 | `trade_room` | `tnpr` | Para cada mercancia y bodega: cabe o no cabe (incluye el +1 del acarreo de `CPX`, el limite de 200 en kg/g y los tribbles) | A | sin-verificar |
| 2 | `trade_buy`, `trade_sell`, `trade_max_buy` | `TT219`, `TT208` (+ `GCASH`/`LCASH`/`MCASH`) | Dinero y bodega tras comprar/vender; el original calcula el importe en 16 bits (`GCASH`): aqui es 32 bits (**diferencia intencionada**, ver `game.c`) | A | sin-verificar |
| 3 | `trade_price` y mercado (`uni_gen_market`) | `TT151`, `var`, `GVL` | Precio y existencias para todas las combinaciones sistema/`QQ26`; la comparacion con `TXTELITE.C` ya existe pero no con el 6502 | A | sin-verificar |
| 4 | `equip_buy`, `equip_price`, `laser_refund`, `equip_count` | `EQSHP`, `PRXS`, `refund`, `qv` | Dinero, equipo y lasers tras cada compra; articulos ofrecidos por nivel tecnologico; cobra antes de reembolsar el laser antiguo | A | sin-verificar |
| 5 | `cmdr_check`, `cmdr_check2` | `CHECK`, `CHECK2` | Ya coinciden con una transcripcion literal (3.003 bloques) y con las constantes de fabrica `$03`/`$27`; falta el emulador | B | transcripcion |
| 6 | `cmdr_default`, `cmdr_unpack`, `cmdr_pack`, `cmdr_seal` | `JAMESON`, `DFAULT`, `SVE` | Bloque de 77 bytes campo a campo; `cok` bit 7 y bit 3; `svc >> 1` al guardar | B | sin-verificar |
| 7 | `rating_name`, `legal_name` | `STATUS` (`st4`, `st3`, `st5`) | Rango segun `TALLY` (umbrales 8, 16, 32, 64, 128, 512, 2560, 6400) y estado legal segun `FIST` | B | sin-verificar |
| 8 | `jump_do`, `jump_possible`, `jump_distance`, `jump_galactic`, `game_enter_system` | `TT18`, `TT111`, `hyp`, `TT114` | Combustible gastado, sistema de llegada, semilla de galaxia tras el salto galactico, `QQ26` y mercado nuevos | B | sin-verificar |
| 9 | `uni_nearest` | `TT111` | Sistema mas cercano a un punto para todos los puntos (la comparacion con el oraculo de Text Elite no cubre esta funcion) | B | sin-verificar |
| 10 | `scr_chart` (cartas galactica y local) | `TT22`, `TT23`, `TT103`, `TT105`, `TT114` | Escala y posicion de los puntos, radio del circulo de combustible, desplazamiento de la mira (ahora pasos de 1 y 2 unidades, no los del original) | C | sin-verificar |
| 11 | `scr_data` | `TT25`, `TT24` | Texto y formato de la ficha de sistema (poblacion en miles de millones, productividad, radio) | C | sin-verificar |
| 12 | `scr_status`, `scr_inventory`, `scr_market` | `STATUS`, `TT213`, `TT167` | Contenido y orden de las lineas (la maquetacion es propia) | C | sin-verificar |
| 13 | `flight_run` (polvo, panel, hiperespacio, atraque) | `STARS`, `DIALS`, `hyp`, `main loop` | **Sustituto provisional**: no es un port. Se sustituira al portar `STARS`, `MVEIT`, `DIALS`, `SCAN`, `TACTICS`... | A | propia |
| 14 | `save_commander`, `load_commander`, `save_encode/decode` | `SVE`, `LOD`, `MUTILATE`, `UNMUTILATE` | **Formato propio** (94 bytes con cifrado XOR). Solo debe comprobarse que los tres checksums y el bloque de 77 bytes coinciden con los del original | C | propia |
| 15 | `plat_wait_key` (QL) | - | En el QL: que espere una tecla y que no queden pulsaciones en la consola al salir | A | sin-verificar |

## Reglas para esta lista

- Al portar una rutina nueva, se anade una fila (o se amplia una existente) y una linea en `tools/ported.txt`; luego `python3 tools/inventory.py`.
- Para verificar una fila: ejecutar la rutina original en `tools/orig.py` con entradas aleatorias y valores limite, comparar con el C, probar con mutaciones de una instruccion (como se hizo con `LL9`) y pasar la fila a `verificada`.
- Si en el QL real algo falla o se ve distinto, anotar aqui la fila afectada y subirla de prioridad.
