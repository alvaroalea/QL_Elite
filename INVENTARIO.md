# Inventario de rutinas (Elite Apple II -> Elite QL)

Generado por `tools/inventory.py` a partir de la fuente de referencia (Mark Moxon, Elite Apple II) y de `tools/ported.txt`. No se edita a mano: se cambia `tools/ported.txt` y se ejecuta `python3 tools/inventory.py`.

## Resumen

- Rutinas de la fuente: **528**
- **Portadas** (con la funcion C que las cubre): **375**, de ellas verificadas contra el 6502: 77 y pendientes de verificar: 298
- **No necesarias** en la version QL (cargador, proteccion anticopia, RWTS del DOS, tablas de HGR, motor de textos comprimidos, utilidades que da el compilador de C, etc.): **135**
- **Sin portar** (pendientes de implementar, incluidas las parciales): **18**

Comprobacion: total = portadas + no necesarias + sin portar (528 = 375 + 135 + 18).

| Area | Total | Portadas | De ellas verificadas | No necesarias | Pendientes verificar | Pendientes implementar |
|---|---:|---:|---:|---:|---:|---:|
| Charts | 9 | 9 | 0 | 0 | 9 | 0 |
| Dashboard | 29 | 29 | 4 | 0 | 25 | 0 |
| Drawing circles | 9 | 9 | 6 | 0 | 3 | 0 |
| Drawing lines | 27 | 23 | 11 | 4 | 12 | 0 |
| Drawing pixels | 6 | 6 | 0 | 0 | 6 | 0 |
| Drawing planets | 7 | 7 | 7 | 0 | 0 | 0 |
| Drawing ships | 5 | 4 | 2 | 1 | 2 | 0 |
| Drawing suns | 3 | 3 | 3 | 0 | 0 | 0 |
| Drawing the screen | 18 | 8 | 0 | 10 | 8 | 0 |
| Equipment | 6 | 6 | 0 | 0 | 6 | 0 |
| Flight | 29 | 28 | 10 | 0 | 18 | 1 |
| Keyboard | 19 | 13 | 0 | 4 | 13 | 2 |
| Loader | 5 | 0 | 0 | 5 | 0 | 0 |
| Main loop | 1 | 0 | 0 | 0 | 0 | 1 |
| Market | 19 | 19 | 4 | 0 | 15 | 0 |
| Maths (Arithmetic) | 52 | 51 | 13 | 1 | 38 | 0 |
| Maths (Geometry) | 19 | 19 | 2 | 0 | 17 | 0 |
| Missions | 16 | 0 | 0 | 5 | 0 | 11 |
| Moving | 8 | 8 | 1 | 0 | 7 | 0 |
| Save and load | 61 | 27 | 3 | 34 | 24 | 0 |
| Sound | 13 | 11 | 0 | 2 | 11 | 0 |
| Stardust | 7 | 7 | 0 | 0 | 7 | 0 |
| Start and end | 8 | 7 | 0 | 0 | 7 | 1 |
| Status | 7 | 7 | 1 | 0 | 6 | 0 |
| Tactics | 8 | 7 | 5 | 1 | 2 | 0 |
| Text | 77 | 34 | 0 | 41 | 34 | 2 |
| Tube | 1 | 0 | 0 | 1 | 0 | 0 |
| Universe | 34 | 32 | 5 | 2 | 27 | 0 |
| Utility routines | 18 | 1 | 0 | 17 | 1 | 0 |
| Workspaces | 7 | 0 | 0 | 7 | 0 | 0 |
| **Total** | **528** | **375** | **77** | **135** | **298** | **18** |

Columnas: *Portadas* = verificadas + pendientes de verificar (incluye los equivalentes propios); *Pendientes verificar* = estados transcripcion, sin-verificar y propia; *Pendientes implementar* = sin portar o parciales.

## Rutinas sin portar, por area

- **Flight** (1): `DOENTRY` (parcial)
- **Keyboard** (2): `TGINT`, `DKS3`
- **Main loop** (1): `Main` (parcial)
- **Missions** (11): `BRIEF2`, `BRP`, `BRIEF3`, `DEBRIEF2`, `DEBRIEF`, `TBRIEF`, `BRIEF`, `BRIS`, `PAUSE`, `PAS1`, `THERE`
- **Start and end** (1): `DEATH` (parcial)
- **Text** (2): `MT27`, `MT28`

## Estados

**verificada** = comparada con el 6502 original (emulador); **transcripcion** = comparada con una transcripcion literal propia del 6502 (falta el emulador); **sin-verificar** = portada, anotada en PENDIENTES.md; **propia** = sustituto o equivalente propio (la funcion C cubre lo que hacia la rutina, con otra forma); **parcial** = existe una version reducida y falta parte (cuenta como pendiente de implementar); **no-necesaria** = no hace falta en la version QL (la nota dice por que); vacio = sin portar.

## Charts (9 portadas de 9)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| HME2 | Search the galaxy for a system | sin-verificar | `chart_find_system` | busqueda por nombre completo sin mayusculas; solo atracado (tecla F); la entrada de texto es edit_text en menu.c |
| TT22 | Show the Long-range Chart | sin-verificar | `chart_draw` |  |
| TT16 | Move the crosshairs on a chart | propia | `scr_chart` | movimiento de la mira propio: libre, y el objetivo es el sistema mas cercano (TT111); el original mueve QQ9/QQ10 directamente |
| TT103 | Draw a small set of crosshairs on a chart | sin-verificar | `chart_cursor` |  |
| TT123 | Move galactic coordinates by a signed delta | propia | `scr_chart` | suma con saturacion (clampi) en lugar de la rutina |
| TT105 | Draw crosshairs on the Short-range Chart, with clipping | sin-verificar | `chart_cursor` |  |
| TT23 | Show the Short-range Chart | sin-verificar | `chart_draw` |  |
| TT114 | Display either the Long-range or Short-range Chart | sin-verificar | `chart_draw` | dispatcher galactica/local; el redibujado tras un salto hecho desde una carta no hace falta aun: el salto solo se inicia desde la vista de vuelo |
| hm | Select the closest system and redraw the chart crosshairs | sin-verificar | `chart_hm` | tecla D: sistema mas cercano a la cruz, nombre y distancia |

## Dashboard (29 portadas de 29)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| DIALS | Update the dashboard: speed indicator | sin-verificar | `dash_update` | velocidad, alabeo, cabeceo, 4 bancos de energia, escudos, combustible, altitud y temperaturas |
| DIS1 | Update a bar-based indicator on the dashboard | sin-verificar | `dis1` |  |
| DIS5 | Update the roll or pitch indicator on the dashboard | sin-verificar | `dis5` | marcador de alabeo o cabeceo (2 puntos de ancho) |
| dialY | The screen y-coordinate of the top-left corner of each indicator | sin-verificar | `dialY` |  |
| dialle | The threshold value for each indicator that defines low and high | sin-verificar | `dialle` |  |
| dialc1 | The colour for each indicator for values that are below the | sin-verificar | `dialc1` |  |
| dialc2 | The colour for each indicator for values that are on or above the | sin-verificar | `dialc2` |  |
| cntr | Apply damping to the pitch or roll dashboard indicator | sin-verificar | `space_cntr` |  |
| BUMP2 | Bump up the value of the pitch or roll dashboard indicator | sin-verificar | `space_bump2` |  |
| REDU2 | Reduce the value of the pitch or roll dashboard indicator | sin-verificar | `space_redu2` |  |
| WPSHPS | Clear the scanner, reset the ball line and sun line heaps | propia | `space_scanner_reset` | solo la parte del radar: ninguna nave figura como dibujada |
| COMPAS | Update the compass | sin-verificar | `compas` | el acarreo de ADC |
| SP1 | Draw the space station on the compass | sin-verificar | `compas` | estacion si SSPR |
| SP2 | Draw a dot on the compass, given the planet/station vector | sin-verificar | `compas` |  |
| DOT | Draw a dash on the compass | sin-verificar | `dot` |  |
| ABORT | Unarm missiles and update the dashboard indicators | verificada | `abort_missile` | dentro de las teclas de misil (make test_tactics) |
| ABORT2 | Set/unset the lock target for a missile and update the dashboard | verificada | `abort2` | MSAR = 0 (MSBAR vuelve con Y = 0); make test_tactics |
| msblob | Display the dashboard's missile indicators in green | sin-verificar | `dash_msblob` | NOMSL indicadores en verde y el resto apagados |
| ECMOF | Switch off the E.C.M. and turn off the dashboard bulb | verificada | `ecm_off` | dentro de MA16..MA66 (make test_tactics) |
| MSBARS | Draw two horizontal lines as part of an indicator bar, from | sin-verificar | `msbars` | cuatro lineas bajo Y1 con gfx_hloin |
| ECBLB2 | Start up the E.C.M. (light up the indicator, start the countdown | verificada | `ecm_on` | a traves de TACTICS (make test_tactics) |
| ECBLB | Light up the E.C.M. indicator bulb ("E") on the dashboard | sin-verificar | `dash_ecblb` | bombilla E (XOR); la enciende y apaga el ECM |
| SPBLB | Light up the space station indicator ("S") on the dashboard | sin-verificar | `dash_spblb` | bombilla S (estacion cerca) |
| BULB | Draw an indicator bulb on the dashboard | sin-verificar | `bulb` | 7 x 8 puntos en XOR; posicion (192, 176) la S y (56, 176) la E |
| ECBT | The character bitmap for the E.C.M. indicator bulb | sin-verificar | `bulb_e` |  |
| SPBT | The bitmap definition for the space station indicator bulb | sin-verificar | `bulb_s` |  |
| MSBAR | Draw a specific indicator in the dashboard's missile bar | sin-verificar | `msbar` | borra el color anterior y dibuja el nuevo (XOR) |
| msloc | The screen x-coordinates of the four missile indicators on the | sin-verificar | `msloc` | x = 40, 32, 24, 16 |
| SCAN | Display the current ship on the scanner | sin-verificar | `space_scan` | el bit 4 de INWK+31 (dibujada), colores de scacol y recorte a 146..190 como el original |

## Drawing circles (9 portadas de 9)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| BLINE | Draw a circle segment and add it to the ball line heap | verificada | `bline` | make test_planet (secuencias de PLANET contra el 6502) |
| LL164 | Make the hyperspace sound and draw the hyperspace tunnel | sin-verificar | `arrive` | sonido (SOBLIP) y tunel al saltar; el salto en si es propio |
| LAUN | Make the launch sound and draw the launch tunnel | sin-verificar | `flight_run` | sonido (SOHISS) y tunel al despegar |
| HFS2 | Draw the launch or hyperspace tunnel | verificada | `tunnel_draw` | HFS1/HFL1 contra el 6502 (make test_planet); STP 4 hiperespacio, 8 lanzamiento; el color (blanco) es una eleccion propia |
| TT14 | Draw a circle with crosshairs on a chart | sin-verificar | `tt14` |  |
| TT128 | Draw a circle on a chart | verificada | `tt14` | make test_planet (circulo de las cartas con CIRCLE2) |
| CIRCLE | Draw a circle for the planet | verificada | `circle_draw` | make test_planet (secuencias de PLANET contra el 6502) |
| CIRCLE2 | Draw a circle (for the planet or chart) | verificada | `circle2_draw` | make test_planet (secuencias de PLANET contra el 6502) |
| CHKON | Check whether any part of a circle appears on the extended screen | verificada | `sun_chkon` | make test_planet (secuencias de PLANET contra el 6502) |

## Drawing lines (23 portadas de 27, 4 no necesarias)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| BOMBOFF | Draw the zig-zag lightning bolt for the energy bomb | verificada | `bomb_off` | make test_equip (lineas del rayo) |
| BOMBEFF2 | Erase the energy bomb zig-zag lightning bolt, make the sound of | sin-verificar | `bomb_eff2`, `bomb_eff`, `bomb_frame` | cuatro BOMBEFF por fotograma y el tramo MA18 de la parte 16; tecla con la comprobacion de la version publicada (IB disk) |
| BOMBON | Randomise and draw the energy bomb's zig-zag lightning bolt lines | verificada | `bomb_on` | make test_equip |
| BOMBPOS | A set of x-coordinates that are used as the basis for the energy | verificada | `bombpos` | tabla |
| BOMBTBX | This is where we store the x-coordinates for the energy bomb's | verificada | `bombtbx` |  |
| BOMBTBY | This is where we store the y-coordinates for the energy bomb's | verificada | `bombtby` |  |
| NLI4 | Draw a line of dashes underneath a title on the text screen | propia | `ui_title` | titulo con la linea que lo subraya (menu.c) |
| NLIN3 | Print a title and draw a horizontal line at row 19 to box it in | propia | `ui_title` | titulo con la linea que lo subraya (menu.c) |
| NLIN4 | Draw a horizontal line at pixel row 19 to box in a title | propia | `ui_title` | titulo con la linea que lo subraya (menu.c) |
| NLIN | Draw a horizontal line at pixel row 23 to box in a title | propia | `ui_title` | titulo con la linea que lo subraya (menu.c) |
| NLIN2 | Draw a screen-wide horizontal line at the pixel row in A | propia | `ui_title` | titulo con la linea que lo subraya (menu.c) |
| HLOIN2 | Remove a line from the sun line heap and draw it on-screen | verificada | `hloin2` | make test_planet (secuencias de PLANET contra el 6502) |
| LASLI | Draw the laser lines for when we fire our lasers | verificada | `lasli`, `lasli2` | vertice, calor, DENGY y las cuatro lineas (LL30); LASLI2 borra en XOR |
| DVLOIN | Draw a vertical line from (A, GCYT) to (A, GCYB) | propia | `dvloin` | linea vertical de las cartas (chart.c) |
| TT15 | Draw a set of crosshairs | sin-verificar | `tt15` |  |
| EDGES | Draw a horizontal line given a centre and a half-width | verificada | `edges` | make test_planet (secuencias de PLANET contra el 6502) |
| LL118 | Move a point along a line until it is on-screen | verificada | `ship_clip` | dentro de ship3d.c (verificada con el 6502) |
| LL145 | Clip line: Work out which end-points are on-screen, if any | verificada | `ship_clip` | recorte de linea (verificada con el 6502) |
| LSPUT | Draw a ship line using flicker-free animation | verificada | `ship_lsput` | linea de nave en el monton con animacion sin parpadeo (dentro de LL9, verificada) |
| sightcol | Colours for the crosshair sights on the different laser types | propia | `chart_sight` | colores de la mira segun el laser: dentro de chart_sight (sin verificar) |
| beamcol | An unused table of laser colours | no-necesaria |  | tabla sin uso en el original |
| TWFL | Ready-made pixel bytes for the left end of a horizontal line | no-necesaria |  | tablas de la pantalla de alta resolucion del Apple II (direcciones de fila, mascaras de pixel y colores); gfx.c escribe directamente en la pantalla del QL |
| TWFR | Ready-made character rows for the right end of a horizontal line | no-necesaria |  | tablas de la pantalla de alta resolucion del Apple II (direcciones de fila, mascaras de pixel y colores); gfx.c escribe directamente en la pantalla del QL |
| LOIN | Draw a line: Calculate the line gradient in the form of deltas | propia | `gfx_line`, `loin` | linea en XOR; en sun.c, loin() reproduce los efectos en memoria del original (SWAP, orden de los extremos, P, Q, S), verificados con make test_planet |
| HLOIN | Draw a horizontal line from (X1, Y1) to (X2, Y1) | propia | `gfx_hloin`, `hloin` | pares de puntos de X1 a X2-1; hloin() de sun.c redondea X1/X2 a pares y deja X2 como el original (verificado con make test_planet) |
| MASKT | High-resolution pixel bytes for drawing continuous lines of solid | no-necesaria |  | tablas de la pantalla de alta resolucion del Apple II (direcciones de fila, mascaras de pixel y colores); gfx.c escribe directamente en la pantalla del QL |
| VLOIN | Draw a vertical line from (X1, Y1) to (X1, Y2) | sin-verificar | `gfx_vloin` | 2 puntos de ancho, extremos incluidos |

## Drawing pixels (6 portadas de 6)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| PIXEL2 | Draw a stardust particle relative to the screen centre | sin-verificar | `pixel2` |  |
| PIXEL | Draw a two-pixel dash, three-pixel dash or double-height | sin-verificar | `gfx_dust` | ancho segun ZZ y dos lineas si ZZ < 80; sin la geometria de bytes del Apple II |
| TWOS3 | Ready-made two-pixel and three-pixel bytes in white, with an extra | propia |  | tablas de bytes del Apple II: aqui gfx_xor_pixel |
| TWOS | Ready-made bytes for drawing one-pixel dots in the space view | propia |  |  |
| TWOS2 | Ready-made two-bit pixel bytes for plotting colour pixels | propia |  |  |
| CPIX | Draw a colour pixel | sin-verificar | `gfx_cpix` | 2 puntos de ancho (x, x+1) en XOR; sin las tablas de bytes del Apple II |

## Drawing planets (7 portadas de 7)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| PL2 | Remove the planet or sun from the screen | verificada | `planet` | make test_planet |
| PLANET | Draw the planet or sun | verificada | `planet` | make test_planet (con PL9, PL2, WP1) |
| PL9 | Draw the planet as a plain circle | verificada | `planet` | make test_planet |
| WPLS2 | Remove the planet from the screen | verificada | `circle_wpls2` | make test_planet; depende de que LOIN deje SWAP como el original (loin() de sun.c) |
| WP1 | Reset the ball line heap | verificada | `circle_wpls2` | make test_planet; RES2 pone LSX2 y LSY2 a $FF |
| PL21 | Return from a planet/sun-drawing routine with a failure flag | verificada | `planet` | make test_planet (SEC; RTS) |
| PLS6 | Calculate (X K) = (A P+1 P) / (z_sign z_hi z_lo) | verificada | `sm_pls6` |  |

## Drawing ships (4 portadas de 5, 1 no necesarias)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| scacol | Ship colours on the scanner | sin-verificar | `scacol` | cadena con el color de cada tipo |
| DOEXP | Draw an exploding ship | sin-verificar | `doexp` | nube de la nave que explota; el color de las particulas (azul/rojo/fuzzy/otro del Apple II) se muestra como cian/rojo/amarillo/blanco |
| exlook | A table to shift X left by one place when X is 0 or 1 | no-necesaria |  | tabla de EXNO (sonido de explosion): el sonido ya esta en sound.c |
| SHPPT | Draw a distant ship as a point rather than a full wireframe | verificada | `ship_shppt` |  |
| LL9 | Draw ship: Check if ship is exploding, check if ship is in front | verificada | `ship_ll9` |  |

## Drawing suns (3 portadas de 3)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| FLFLLS | Reset the sun line heap | verificada | `sun_flflls` | make test_planet (secuencias de PLANET contra el 6502) |
| SUN | Draw the sun: Set up all the variables needed to draw the sun | verificada | `sun_draw` | make test_planet (secuencias de PLANET contra el 6502) |
| WPLS | Remove the sun from the screen | verificada | `sun_wpls` | make test_planet (secuencias de PLANET contra el 6502) |

## Drawing the screen (8 portadas de 18, 10 no necesarias)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| SCTBX1 | Lookup table for converting a pixel x-coordinate to the bit number | no-necesaria |  | tablas de la pantalla de alta resolucion del Apple II (direcciones de fila, mascaras de pixel y colores); gfx.c escribe directamente en la pantalla del QL |
| SCTBX2 | Lookup table for converting a pixel x-coordinate to the byte | no-necesaria |  | tablas de la pantalla de alta resolucion del Apple II (direcciones de fila, mascaras de pixel y colores); gfx.c escribe directamente en la pantalla del QL |
| TRADEMODE | Clear the screen and set up a trading screen | propia | `ui_clear`, `gfx_clear` | borrado de pantalla y de zonas (gfx_fill_lines, txt_clear_rows, clear_view) |
| DET1 | Show or hide the dashboard (for when we die) | no-necesaria |  | en el Apple II es un RTS |
| TT66 | Clear the screen and set the current view type | propia | `gfx_fill_lines` | borrado rapido de la vista y de la pantalla |
| TTX66 | Clear the top part of the screen, draw a border box if required | propia | `ui_clear`, `gfx_clear` | borrado de pantalla y de zonas (gfx_fill_lines, txt_clear_rows, clear_view) |
| cellocl | Lookup table for converting a text row number to the address of | no-necesaria |  | tablas de la pantalla de alta resolucion del Apple II (direcciones de fila, mascaras de pixel y colores); gfx.c escribe directamente en la pantalla del QL |
| SCTBL | Lookup table for converting a character row number to the address | no-necesaria |  | tablas de la pantalla de alta resolucion del Apple II (direcciones de fila, mascaras de pixel y colores); gfx.c escribe directamente en la pantalla del QL |
| SCTBH | Lookup table for converting a character row number to the address | no-necesaria |  | tablas de la pantalla de alta resolucion del Apple II (direcciones de fila, mascaras de pixel y colores); gfx.c escribe directamente en la pantalla del QL |
| SCTBH2 | Lookup table for converting a character row number to the address | no-necesaria |  | tablas de la pantalla de alta resolucion del Apple II (direcciones de fila, mascaras de pixel y colores); gfx.c escribe directamente en la pantalla del QL |
| WSCAN | Wait for 15 * 256 loop iterations | no-necesaria |  | retardo por bucles de instrucciones; el port espera por fotogramas (wait_frames) y por tiempo (snd_tick) |
| clss | Clear the screen, move the text cursor to the top-left corner and | propia | `ui_clear`, `gfx_clear` | borrado de pantalla y de zonas (gfx_fill_lines, txt_clear_rows, clear_view) |
| TTX66K | Clear the whole screen or just the space view (as appropriate), | propia | `ui_clear`, `gfx_clear` | borrado de pantalla y de zonas (gfx_fill_lines, txt_clear_rows, clear_view) |
| cleargrap | Clear screen memory for the top part of the graphics screen mode | propia | `ui_clear`, `gfx_clear` | borrado de pantalla y de zonas (gfx_fill_lines, txt_clear_rows, clear_view) |
| CLYNS | Clear two character rows near the bottom of the screen | propia | `txt_clear_rows` | borrado de filas de texto con el relleno rapido |
| clearrow | Clear a character row of screen memory, drawing blue borders along | propia | `ui_clear`, `gfx_clear` | borrado de pantalla y de zonas (gfx_fill_lines, txt_clear_rows, clear_view) |
| HGR | Switch to the high-resolution graphics screen mode | no-necesaria |  | cambio de modo grafico y de texto del Apple II; el QL usa un solo modo (modo 8) |
| TEXT | Switch to the text screen mode | no-necesaria |  | cambio de modo grafico y de texto del Apple II; el QL usa un solo modo (modo 8) |

## Equipment (6 portadas de 6)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| EQSHP | Show the Equip Ship screen | sin-verificar | `equip_buy` | la pantalla es scr_equip (propia); equip_buy lleva las reglas, el cobro y el reembolso |
| eq | Subtract the price of equipment from the cash pot | sin-verificar | `cash_spend` | dentro de equip_buy (cobra antes de reembolsar el laser antiguo) |
| prx | Return the price of a piece of equipment | sin-verificar | `equip_price` |  |
| qv | Print a menu of the four space views, for buying lasers | propia | `scr_equip` | pide la vista (0 a 3) al comprar un laser |
| refund | Install a new laser, processing a refund if applicable | sin-verificar | `laser_refund` |  |
| PRXS | Equipment prices | sin-verificar | `equip_price` |  |

## Flight (28 portadas de 29, 1 sin portar)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| DOENTRY | Dock at the space station and work out any mission progression | parcial | `flight_run`, `combat_collide` | se atraca entrando por la ranura (ISDK), con el ordenador de atraque (C) o, sin el, con C (provisional); energia y escudos se reponen al despegar; falta el progreso de las misiones (paso de misiones) |
| ESCAPE | Launch our escape pod | sin-verificar | `flight_escape`, `ships_escape` | la Cobra se aleja 97 fotogramas (MVEIT + LL9), carga, FIST y capsula a cero, 70 de combustible, atraque |
| DOCKIT | Apply docking manoeuvres to the ship in INWK | verificada | `dockit` | make test_equip (con TA2, GOPL, TA151 y la cola de TACTICS en dockit.c) |
| DCS1 | Calculate the vector from the ideal docking position to the ship | verificada | `dcs1` | dentro de DOCKIT |
| SESCP | Spawn an escape pod from the current (parent) ship | verificada | `sescp` | a traves de TACTICS (la capsula de una nave casi sin energia; make test_tactics) |
| dockEd | Print a message to say there is no hyperspacing allowed inside the | propia | `ui_nav` | mensaje DOCKED al pulsar H o G atracado |
| hyp | Start the hyperspace process | propia | `start_hyper` | inicio y cuenta atras del hiperespacio (flight.c), con jump_possible y jump_do |
| wW | Start a hyperspace countdown | propia | `start_hyper` | inicio y cuenta atras del hiperespacio (flight.c), con jump_possible y jump_do |
| TTX110 | Set the current system to the nearest system and return to hyp | propia | `find_system` | sistema mas cercano a la mira |
| Ghy | Perform a galactic hyperspace jump | propia | `jump_galactic` | salto galactico |
| ee3 | Print the hyperspace countdown in the top-left of the screen | propia | `say_hyper` | cuenta atras del hiperespacio en pantalla |
| TT147 | Print an error when a system is out of hyperspace range | propia | `start_hyper` | mensaje de fuera de alcance |
| MJP | Process a mis-jump into witchspace | sin-verificar | `misjump` | salto fallido: tunel, MJ = $FF, tres Thargoides (GTHG), polvo de 2, QQ1 EOR 31; la distancia en el hiperespacio se mide desde (QQ0, QQ1) |
| TT18 | Try to initiate a jump into hyperspace | propia | `start_hyper` | inicio y cuenta atras del hiperespacio (flight.c), con jump_possible y jump_do |
| TT110 | Launch from a station or show the front space view | propia | `space_launch`, `flight_run` | despegue y vista frontal |
| SHD | Charge a shield and drain some energy from the energy banks | verificada | `shd` | cae en DENGY: cargar un escudo gasta 1 de energia |
| DENGY | Drain some energy from the energy banks | verificada | `dengy` |  |
| OOPS | Take some damage | verificada | `oops` | escudo segun el signo de z de la nave (K%); el SBC usa el acarreo de quien llama |
| me2 | Remove an in-flight message from the space view | propia | `dly_tick`, `flight_mess_off` | el mensaje se borra a los 20 fotogramas (DLY) |
| WARP | Perform an in-system jump | verificada | `warp` | make test_equip (el repintado de LOOK1 lo hace flight.c) |
| me1 | Erase an old in-flight message and display a new one | propia | `say` | mensaje en vuelo: borra el anterior y muestra el nuevo |
| MESS | Display an in-flight message | propia | `mess`, `say` | mensaje en vuelo en la fila 24 (no en la vista) durante DLY = 20 fotogramas; los avisos propios del port usan say |
| mes9 | Print a text token, possibly followed by " DESTROYED" | propia | `flight_mess`, `mess_de` | " DESTROYED" con la bandera de (vale para un mensaje) |
| OUCH | Potentially lose cargo or equipment following damage | verificada | `ouch` | QQ20+X con X = 0..21 (mercancias, ECM, BST, BOMB, ENGY, DKCMP) |
| ou2 | Display "E.C.M.SYSTEM DESTROYED" as an in-flight message | verificada | `ouch` |  |
| ou3 | Display "FUEL SCOOPS DESTROYED" as an in-flight message | verificada | `ouch` |  |
| PLUT | Flip the coordinate axes for the four different views | sin-verificar | `plut` | ejes de INWK para las vistas trasera, izquierda y derecha |
| LOOK1 | Initialise the space view | propia | `change_view` | cambio de vista (FLIP + mira) y redraw_all (NWSTARS + mira) al volver de una pantalla de texto |
| SIGHT | Draw the laser crosshairs | sin-verificar | `chart_sight` | dos cruces concentricas con TT15 de la vista espacial; color segun el laser (pulsos azul, haz rojo, otros blanco) |

## Keyboard (13 portadas de 19, 4 no necesarias, 2 sin portar)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| TGINT | The keys used to toggle configuration settings when the game is |  |  |  |
| FLKB | Flush the keyboard buffer | propia | `plat_flush_input` |  |
| PAUSE2 | Wait until a key is pressed, ignoring any existing key press | propia | `plat_wait_key` | esperar una tecla ignorando la anterior |
| TT214 | Ask a question with a "Y/N?" prompt and return the response | propia | `scr_disk` |  |
| YESNO | Wait until either "Y" or "N" is pressed | propia | `scr_disk` | confirmaciones Y/N dentro del menu de disco |
| TT17 | Scan the keyboard for cursor key or joystick movement | propia | `in_held` | cursores o joystick |
| TT102 | Process number key, save key, hyperspace and chart key presses | propia | `ui_nav` | en vuelo: 1-4 vistas, 5-9 pantallas, 0 carta galactica, H y G saltos; atracado: U despega, 0 galactica, 5-9 como en vuelo |
| ZEKTRAN | Clear the key logger | no-necesaria |  | registrador de teclas del BBC (o sin uso): keys.c lee las filas directamente y no necesita limpiarlo |
| RDS1 | Scan one axis of the joystick and return its position | propia | `ctl_read` | palanca de mando CTL1 (keys.c) |
| RDKEY | Scan the keyboard for key presses and the joystick, and update the | propia | `key_scan` | lectura de las filas del teclado por IPC (keys.c) |
| KYTB | Lookup table for in-flight keyboard controls | propia | `keys.c` | tabla de teclas de vuelo: las teclas del QL se definen en keys.h |
| DKSANYKEY | An unused routine that scans the keyboard to detect whether any | no-necesaria |  | registrador de teclas del BBC (o sin uso): keys.c lee las filas directamente y no necesita limpiarlo |
| DKS2 | Read the joystick position | propia | `ctl_read` | palanca de mando CTL1 (keys.c) |
| DKS3 | Toggle a configuration setting and emit a beep |  |  |  |
| U% | Clear the key logger | no-necesaria |  | registrador de teclas del BBC (o sin uso): keys.c lee las filas directamente y no necesita limpiarlo |
| DOKEY | Scan for the seven primary flight controls and apply the docking | propia | `ctl_read` | cursores = alabeo y cabeceo, F acelera, D frena, A o espacio disparan; JSTX/JSTY los calcula space_controls |
| DK4 | Scan for pause, configuration and secondary flight keys | propia | `in_poll` | teclas secundarias por caracteres (key_getchar) |
| TT217 | Scan the keyboard until a key is pressed | propia | `in_wait` |  |
| KTRAN | An unused key logger buffer that's left over from the 6502 Second | no-necesaria |  | registrador de teclas del BBC (o sin uso): keys.c lee las filas directamente y no necesita limpiarlo |

## Loader (0 portadas de 5, 5 no necesarias)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| ENTRY | An entry point at the start of the game binary | no-necesaria |  | proteccion anticopia del original: suma de comprobacion, descifrado y desmezcla del codigo; en el QL no existe |
| S% | Checksum, decrypt and unscramble the main game code, and start the | no-necesaria |  | proteccion anticopia del original: suma de comprobacion, descifrado y desmezcla del codigo; en el QL no existe |
| BEGIN | Initialise the configuration variables and start the game | no-necesaria |  | arranque del Apple II (interrupciones, modo de pantalla, memoria); lo hacen main.c y app.c |
| COLD | Initialise the screen mode, clear memory and set up interrupt | no-necesaria |  | arranque del Apple II (interrupciones, modo de pantalla, memoria); lo hacen main.c y app.c |
| NMIpissoff | Acknowledge NMI interrupts and ignore them | no-necesaria |  | arranque del Apple II (interrupciones, modo de pantalla, memoria); lo hacen main.c y app.c |

## Main loop (0 portadas de 1, 1 sin portar)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| Main | Seed the random number generator | parcial | `ships_step`, `flight_run`, `laser_fire`, `combat_hit`, `combat_killed`, `laser_frame_end`, `energy_tick`, `laser_cool`, `combat_collide`, `damage_checks`, `ecm_frame`, `ecm_key`, `missile_unarm`, `missile_target`, `missile_fire`, `missile_lock` | bucle principal: naves, laser, choques, recogida, atraque, parte 15, bomba, ECM y misiles (verificados con make test_laser, test_damage, test_equip, test_tactics); faltan la generacion de encuentros (parte 13, TT100), la aparicion de la estacion (parte 14) y los Thargoides fuera del hiperespacio |

## Market (19 portadas de 19)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| tnpr1 | Work out if we have space for one tonne of cargo | verificada | `combat_collide`, `trade_fits` | recogida de cargas (make test_damage) |
| tnpr | Work out if we have space for a specific amount of cargo | verificada | `trade_room` |  |
| TT219 | Show the Buy Cargo screen | verificada | `trade_buy` |  |
| gnum | Get a number from the keyboard | propia | `trade_screen` | la cantidad se teclea en el propio listado (cursores, cifras, M = maximo) |
| NWDAV4 | Print an "ITEM?" error, make a beep and rejoin the TT210 routine | propia | `ui_msg` | los errores salen como mensaje en la fila 28 con sonido |
| TT208 | Show the Sell Cargo screen | verificada | `trade_sell` |  |
| TT210 | Show a list of current cargo in our hold, optionally to sell | propia | `trade_screen` | la lista de venta es el mismo listado con las existencias en bodega |
| TT213 | Show the Inventory screen | sin-verificar | `scr_inventory` | maquetacion propia |
| TT151 | Print the name, price and availability of a market item | sin-verificar | `trade_price` |  |
| TT152 | Print the unit ("t", "kg" or "g") for a market item | propia | `unit_char` | unidad (t, kg, g) en menu.c |
| TT160 | Print "t" (for tonne) and a space | propia | `unit_char` |  |
| TT161 | Print "kg" (for kilograms) | propia | `unit_char` |  |
| TT16a | Print "g" (for grams) | propia | `unit_char` |  |
| TT163 | Print the headers for the table of market prices | propia | `market_header` | cabecera del mercado en menu.c |
| TT167 | Show the Market Price screen | sin-verificar | `scr_market` | maquetacion propia |
| var | Calculate QQ19+3 = economy * |economic_factor| | sin-verificar | `uni_gen_market` | precio = base + (QQ26 AND mask) + gradiente * economia; viene de Text Elite (universe.c), comparar con el 6502 |
| dn | Print the amount of money we have left in the cash pot, then make | propia | `trade_info` | el dinero restante se redibuja tras cada operacion |
| ITEM | Macro definition for the market prices table | sin-verificar | `uni_goods` | tabla de mercancias copiada de Text Elite |
| QQ23 | Market prices table | sin-verificar | `uni_goods` | comparar con la tabla del 6502 |

## Maths (Arithmetic) (51 portadas de 52, 1 no necesarias)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| log | Binary logarithm table (high byte) | verificada | `em_logL`, `em_alogh` | tablas de logaritmos y antilogaritmos de emath.c, comprobadas con FMLTU y DVID4 |
| logL | Binary logarithm table (low byte) | verificada | `em_logL`, `em_alogh` | tablas de logaritmos y antilogaritmos de emath.c, comprobadas con FMLTU y DVID4 |
| alogh | Binary antilogarithm table | verificada | `em_logL`, `em_alogh` | tablas de logaritmos y antilogaritmos de emath.c, comprobadas con FMLTU y DVID4 |
| PIX1 | Calculate (YY+1 SYL+Y) = (A P) + (S R) and draw stardust particle | propia | `pixel2` | dibujo del polvo estelar (stars.c) |
| MAS3 | Calculate A = x_hi^2 + y_hi^2 + z_hi^2 in the K% block | transcripcion | `sm_mas3` |  |
| VCSU1 | Calculate vector K3(8 0) = [x y z] - coordinates of the sun or | transcripcion | `sm_vcsu1` |  |
| VCSUB | Calculate vector K3(8 0) = [x y z] - coordinates in (A V) | transcripcion | `sm_vcsub` |  |
| TAS1 | Calculate K3 = (x_sign x_hi x_lo) - V(1 0) | transcripcion | `sm_tas1` |  |
| MU5 | Set K(3 2 1 0) = (A A A A) and clear the C flag | propia | `emath.c` | entradas internas de MULTU, MU11 y compañia: en C van dentro de cada funcion |
| MULT3 | Calculate K(3 2 1 0) = (A P+1 P) * Q | transcripcion | `em_mult3` |  |
| MLS2 | Calculate (S R) = XX(1 0) and (A P) = A * ALP1 | propia | `mls1`, `mls2` | envoltorios de la multiplicacion con signo (stars.c) |
| MLS1 | Calculate (A P) = ALP1 * A | propia | `mls1`, `mls2` | envoltorios de la multiplicacion con signo (stars.c) |
| MU6 | Set P(1 0) = (A A) | propia | `emath.c` | entradas internas de MULTU, MU11 y compañia: en C van dentro de cada funcion |
| SQUA | Clear bit 7 of A and calculate (A P) = A * A | transcripcion | `em_squa` |  |
| SQUA2 | Calculate (A P) = A * A | transcripcion | `em_squa2` |  |
| MU1 | Copy X into P and A, and clear the C flag | propia | `emath.c` | entradas internas de MULTU, MU11 y compañia: en C van dentro de cada funcion |
| MLU1 | Calculate Y1 = y_hi and (A P) = |y_hi| * Q for Y-th stardust | transcripcion | `em_mlu1` |  |
| MLU2 | Calculate (A P) = |A| * Q | transcripcion | `em_mlu2` |  |
| MULTU | Calculate (A P) = P * Q | transcripcion | `em_multu` |  |
| MU11 | Calculate (A P) = P * X | transcripcion | `em_mu11` |  |
| FMLTU2 | Calculate A = K * sin(A) | verificada | `em_fmltu2` | a traves de CIRCLE2 en make test_planet |
| FMLTU | Calculate A = A * Q / 256 | verificada | `em_fmltu` |  |
| MLTU2 | Calculate (A P+1 P) = (A ~P) * Q | transcripcion | `em_mltu2` |  |
| MUT3 | An unused routine that does the same as MUT2 | no-necesaria |  | sin uso en el original |
| MUT2 | Calculate (S R) = XX(1 0) and (A P) = Q * A | propia | `mls1`, `mls2` | envoltorios de la multiplicacion con signo (stars.c) |
| MUT1 | Calculate R = XX and (A P) = Q * A | propia | `mls1`, `mls2` | envoltorios de la multiplicacion con signo (stars.c) |
| MULT1 | Calculate (A P) = Q * A | transcripcion | `em_mult1` |  |
| MULT12 | Calculate (S R) = Q * A | transcripcion | `em_mult12` |  |
| MAD | Calculate (A X) = Q * A + (S R) | transcripcion | `em_mad` |  |
| ADD | Calculate (A X) = (A P) + (S R) | transcripcion | `em_add` |  |
| TIS1 | Calculate (A ?) = (-X * A + (S R)) / 96 | transcripcion | `em_tis1` |  |
| DV42 | Calculate (P R) = 256 * DELTA / z_hi | transcripcion | `em_dv42` |  |
| DV41 | Calculate (P R) = 256 * DELTA / A | transcripcion | `em_dv41` |  |
| DVID4 | Calculate (P R) = 256 * A / Q | transcripcion | `em_dvid4` |  |
| DVID3B2 | Calculate K(3 2 1 0) = (A P+1 P) / (z_sign z_hi z_lo) | propia | `em_dvid3b` | la entrada con INWK se hace en ship_math.c y en planet() |
| LCASH | Subtract an amount of cash from the cash pot | transcripcion | `sm_lcash` |  |
| MCASH | Add an amount of cash to the cash pot | transcripcion | `sm_mcash` |  |
| GCASH | Calculate (Y X) = P * Q * 4 | transcripcion | `sm_gcash` |  |
| GC2 | Calculate (Y X) = (A P) * 4 | propia | `tt22`, `tt23` | la multiplicacion por 4 va en linea en las cartas |
| SPS2 | Calculate (Y X) = A / 10 | transcripcion | `sm_sps2` |  |
| DORND | Generate random numbers | verificada | `em_dornd` |  |
| TIS2 | Calculate A = A / Q | transcripcion | `em_tis2` |  |
| TIS3 | Calculate -(nosev_1 * roofv_1 + nosev_2 * roofv_2) / nosev_3 | transcripcion | `sm_tis3` |  |
| DVIDT | Calculate (P+1 A) = (A P) / Q | transcripcion | `em_dvidt` |  |
| LL5 | Calculate Q = SQRT(R Q) | verificada | `em_ll5` | transcripcion literal con el acarreo de salida (lo usa SUN); a traves de SUN en make test_planet |
| LL28 | Calculate R = 256 * A / Q | verificada | `em_ll28` |  |
| LL38 | Calculate (S A) = (S R) + (A Q) | transcripcion | `em_ll38` |  |
| LL61 | Calculate (U R) = 256 * A / Q | verificada | `em_ll61` |  |
| LL62 | Calculate 128 - (U R) | verificada | `ship_ll9` | etiqueta ll62 de LL9 (verificada) |
| LL120 | Calculate (Y X) = (S x1_lo) * XX12+2 or (S x1_lo) / XX12+2 | verificada | `ll120_123` | recorte de linea (ship3d.c, verificadas) |
| LL123 | Calculate (Y X) = (S R) / XX12+2 or (S R) * XX12+2 | verificada | `ll120_123` | recorte de linea (ship3d.c, verificadas) |
| LL129 | Calculate Q = XX12+2, A = S EOR XX12+3 and (S R) = |S R| | verificada | `ll129` | dentro de ship3d.c (pendiente de la linea, verificada con el 6502) |

## Maths (Geometry) (19 portadas de 19)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| MAS1 | Add an orientation vector coordinate to an INWK coordinate | transcripcion | `sm_mas1` |  |
| MAS2 | Calculate a cap on the maximum distance to the planet or sun | transcripcion | `sm_mas2` |  |
| TAS4 | Calculate the dot product of XX15 and one of the space station's | transcripcion | `sm_tas4` |  |
| TAS6 | Negate the vector in XX15 so it points in the opposite direction | transcripcion | `sm_tas6` |  |
| TAS3 | Calculate the dot product of XX15 and an orientation vector | transcripcion | `sm_tas3` |  |
| SCALEY | Scale the y-coordinate in A to 0.375 * A | transcripcion | `sm_scaley` |  |
| SCALEY2 | Scale the y-coordinate in A to 0.75 * A | transcripcion | `sm_scaley2` |  |
| SCALEX | Scale the x-coordinate in A to 32 + 0.75 * A | transcripcion | `sm_scalex` |  |
| SPS4 | Calculate the vector to the space station | transcripcion | `sm_sps4` |  |
| SPS3 | Copy a space coordinate from the K% block into K3 | transcripcion | `sm_sps3` |  |
| PROJ | Project the current ship or planet onto the screen | verificada | `sm_proj` |  |
| FAROF | Compare x_hi, y_hi and z_hi with 224 | sin-verificar | `farof` |  |
| FAROF2 | Compare x_hi, y_hi and z_hi with A | propia | `farof` | FAROF con A = 224 |
| MAS4 | Calculate a cap on the maximum distance to a ship | transcripcion | `sm_mas4` |  |
| SPS1 | Calculate the vector to the planet and store it in XX15 | transcripcion | `sm_sps1` |  |
| TAS2 | Normalise the three-coordinate vector in K3 | transcripcion | `sm_tas2` |  |
| NORM | Normalise the three-coordinate vector in XX15 | transcripcion | `sm_norm` |  |
| TIDY | Orthonormalise the orientation vectors for a ship | transcripcion | `sm_tidy` |  |
| LL51 | Calculate the dot product of XX15 and XX16 | verificada | `ship_ll51` |  |

## Missions (0 portadas de 16, 5 no necesarias, 11 sin portar)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| TRIBDIR | The low byte of the four 16-bit directions in which Trumble | no-necesaria |  | tablas de los sprites de los Trumbles (hardware VIC del C64); el contador de Trumbles seguira pendiente con las misiones |
| TRIBDIRH | The high byte of the four 16-bit directions in which Trumble | no-necesaria |  | tablas de los sprites de los Trumbles (hardware VIC del C64); el contador de Trumbles seguira pendiente con las misiones |
| SPMASK | Masks for updating sprite bits in VIC+$10 for the top bit of the | no-necesaria |  | tablas de los sprites de los Trumbles (hardware VIC del C64); el contador de Trumbles seguira pendiente con las misiones |
| BRIEF2 | Start mission 2 |  |  |  |
| BRP | Print an extended token and show the Status Mode screen |  |  |  |
| BRIEF3 | Receive the briefing and plans for mission 2 |  |  |  |
| DEBRIEF2 | Finish mission 2 |  |  |  |
| DEBRIEF | Finish mission 1 |  |  |  |
| TBRIEF | Start mission 3 |  |  |  |
| BRIEF | Start mission 1 and show the mission briefing |  |  |  |
| BRIS | Clear the screen, display "INCOMING MESSAGE" and wait for 2 |  |  |  |
| PAUSE | Wait until a key is pressed for the Constrictor mission briefing |  |  |  |
| PAS1 | Change to the text view for the Constrictor mission briefing |  |  |  |
| THERE | Check whether we are in the Constrictor's system in mission 1 |  |  |  |
| TRIBTA | A table for converting the number of Trumbles in the hold into a | no-necesaria |  | tablas de los sprites de los Trumbles (hardware VIC del C64); el contador de Trumbles seguira pendiente con las misiones |
| TRIBMA | A table for converting the number of Trumbles in the hold into a | no-necesaria |  | tablas de los sprites de los Trumbles (hardware VIC del C64); el contador de Trumbles seguira pendiente con las misiones |

## Moving (8 portadas de 8)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| MVT3 | Calculate K(3 2 1) = (x_sign x_hi x_lo) + K(3 2 1) | transcripcion | `sm_mvt3` |  |
| MVS5 | Apply a 3.6 degree pitch or roll to an orientation vector | sin-verificar | `mvs5` |  |
| SFS2 | Move a ship in space along one of the coordinate axes | verificada | `sfs2` | dentro de SFS1 (make test_laser, nave madre = estacion) |
| MVEIT | Move current ship: Tidy the orientation vectors | sin-verificar | `mveit` | partes 1 a 9; TACTICS es un gancho (move_tactics_hook) hasta el paso 3; los acarreos de MLTU2 salen del bit bajo del producto |
| MVT1 | Calculate (x_sign x_hi x_lo) = (x_sign x_hi x_lo) + (A R) | sin-verificar | `mvt1` |  |
| MVS4 | Apply pitch and roll to an orientation vector | sin-verificar | `mvs4` |  |
| MVT6 | Calculate (A P+2 P+1) = (x_sign x_hi x_lo) + (A P+2 P+1) | sin-verificar | `mvt6` |  |
| MV40 | Rotate the planet or sun's location in space by the amount of | sin-verificar | `mv40` | gira la posicion y sigue por MV45 (orientacion, contadores y radar); K2 byte 0 queda a 0 (en el original es lo que hubiera en K2) |

## Save and load (27 portadas de 61, 34 no necesarias)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| wtable | 6-bit to 7-bit nibble conversion table | no-necesaria |  | RWTS y catalogo del DOS 3.3 del Apple II; QDOS se ocupa del disco desde C (save.c) |
| BRKBK | Set the standard BRKV handler for the game | no-necesaria |  | manejador de errores BRK del 6502; el runtime de C se ocupa de los errores |
| NA% | The data block for the last saved commander | propia | `cmdr_default` | datos del comandante por defecto y del ultimo guardado |
| CHK2 | Second checksum byte for the saved commander data file | verificada | `cmdr_check`, `cmdr_check2` | bytes de comprobacion (verificadas con el 6502) |
| CHK3 | Third checksum byte for the saved commander data file | verificada | `cmdr_check`, `cmdr_check2` | bytes de comprobacion (verificadas con el 6502) |
| CHK | First checksum byte for the saved commander data file | verificada | `cmdr_check`, `cmdr_check2` | bytes de comprobacion (verificadas con el 6502) |
| S1% | The drive and directory number used when saving or loading a | propia | `save_dev` | unidad y directorio |
| NA2% | The data block for the default commander | propia | `cmdr_default` | datos del comandante por defecto y del ultimo guardado |
| CHECK | Calculate the checksum for the last saved commander data block | transcripcion | `cmdr_check` |  |
| CHECK2 | Calculate the third checksum for the last saved commander data | transcripcion | `cmdr_check2` |  |
| JAMESON | Restore the default JAMESON commander | sin-verificar | `cmdr_default` |  |
| COPYNAME | Copy the last saved commander's name from INWK+5 to comnam and pad | propia | `cmdr_set_name`, `save_filename` | copia del nombre del comandante |
| TRNME | Copy the last saved commander's name from INWK to NA% | propia | `cmdr_set_name`, `save_filename` | copia del nombre del comandante |
| TR1 | Copy the last saved commander's name from NA% to INWK | propia | `cmdr_set_name`, `save_filename` | copia del nombre del comandante |
| GTNMEW | Fetch the name of a commander file to save or load | propia | `edit_name` | nombre del comandante |
| FILEPR | Display the currently selected media (disk or tape) | propia | `scr_disk` | unidad actual y errores de disco en el menu de disco |
| OTHERFILEPR | Display the non-selected media (disk or tape) | propia | `scr_disk` | unidad actual y errores de disco en el menu de disco |
| SVE | Display the disk access menu and process saving of commander files | propia | `scr_disk` |  |
| diskerror | Print a disk error, make a beep and wait for a key press | propia | `scr_disk` | unidad actual y errores de disco en el menu de disco |
| thislong | Contains the length of the most recently entered commander name | no-necesaria |  | longitud de los nombres: en C se calcula con strlen |
| oldlong | Contains the length of the last saved commander name | no-necesaria |  | longitud de los nombres: en C se calcula con strlen |
| LOD | Load a commander file | propia | `load_commander` |  |
| DERR1 | The error message for when a disk is write protected | propia | `save_errstr` | mensajes de error de disco |
| DERR2 | The error message for when a disk is full | propia | `save_errstr` | mensajes de error de disco |
| DERR3 | The error message for when a disk catalog is full | propia | `save_errstr` | mensajes de error de disco |
| DERR4 | The error message for when there is a disk I/O error | propia | `save_errstr` | mensajes de error de disco |
| DERR5 | The error message for when a file is not found | propia | `save_errstr` | mensajes de error de disco |
| ERTAB | A lookup table for the five disk error messages | propia | `save_errstr` | mensajes de error de disco |
| comnam | Storage for the commander filename, padded out with spaces to a | propia | `cmdr_set_name`, `save_filename` | copia del nombre del comandante |
| rfile | Read a commander file from a DOS disk into the buffer | propia | `load_commander` | lectura del fichero de comandante |
| wfile | Write a commander file from the buffer to a DOS disk | propia | `save_commander`, `write_file` | escritura del fichero de comandante |
| findf | Search the disk catalog for an existing file | no-necesaria |  | RWTS y catalogo del DOS 3.3 del Apple II; QDOS se ocupa del disco desde C (save.c) |
| finde | Search the disk catalog for an empty file entry | no-necesaria |  | RWTS y catalogo del DOS 3.3 del Apple II; QDOS se ocupa del disco desde C (save.c) |
| rentry | Search the disk catalog for an existing file or an empty file | no-necesaria |  | RWTS y catalogo del DOS 3.3 del Apple II; QDOS se ocupa del disco desde C (save.c) |
| getsct | Analyse the VTOC sector to allocate one free sector | no-necesaria |  | RWTS y catalogo del DOS 3.3 del Apple II; QDOS se ocupa del disco desde C (save.c) |
| isfull | Check the disk to ensure there are least two free sectors, one for | no-necesaria |  | RWTS y catalogo del DOS 3.3 del Apple II; QDOS se ocupa del disco desde C (save.c) |
| gettsl | Read a file's track/sector list | no-necesaria |  | RWTS y catalogo del DOS 3.3 del Apple II; QDOS se ocupa del disco desde C (save.c) |
| rvtoc | Read the VTOC sector into the buffer | no-necesaria |  | RWTS y catalogo del DOS 3.3 del Apple II; QDOS se ocupa del disco desde C (save.c) |
| rsect | Read a specific sector from disk into the buffer | no-necesaria |  | RWTS y catalogo del DOS 3.3 del Apple II; QDOS se ocupa del disco desde C (save.c) |
| wsect | Write a specific sector from the buffer to disk | no-necesaria |  | RWTS y catalogo del DOS 3.3 del Apple II; QDOS se ocupa del disco desde C (save.c) |
| rwts | Read or write a specific sector | no-necesaria |  | RWTS y catalogo del DOS 3.3 del Apple II; QDOS se ocupa del disco desde C (save.c) |
| trytrk | Try finding a specific track on the disk | no-necesaria |  | RWTS y catalogo del DOS 3.3 del Apple II; QDOS se ocupa del disco desde C (save.c) |
| rdrght | Check that this is the correct track | no-necesaria |  | RWTS y catalogo del DOS 3.3 del Apple II; QDOS se ocupa del disco desde C (save.c) |
| prterr | Return from the RWTS code with a "Disk write protected" error | no-necesaria |  | RWTS y catalogo del DOS 3.3 del Apple II; QDOS se ocupa del disco desde C (save.c) |
| rttrk | Read or write a sector on the current track | no-necesaria |  | RWTS y catalogo del DOS 3.3 del Apple II; QDOS se ocupa del disco desde C (save.c) |
| drverr | Return from the RWTS code with a "Disk I/O error" | no-necesaria |  | RWTS y catalogo del DOS 3.3 del Apple II; QDOS se ocupa del disco desde C (save.c) |
| rttrk3 | Successfully return from the RWTS code with no error reported | no-necesaria |  | RWTS y catalogo del DOS 3.3 del Apple II; QDOS se ocupa del disco desde C (save.c) |
| read | Read a sector's worth of data into the buffr2 buffer | no-necesaria |  | RWTS y catalogo del DOS 3.3 del Apple II; QDOS se ocupa del disco desde C (save.c) |
| write | Write a sector's worth of data from the buffr2 buffer to the | no-necesaria |  | RWTS y catalogo del DOS 3.3 del Apple II; QDOS se ocupa del disco desde C (save.c) |
| rdaddr | Read a track address field | no-necesaria |  | RWTS y catalogo del DOS 3.3 del Apple II; QDOS se ocupa del disco desde C (save.c) |
| seek | Fast seek routine | no-necesaria |  | RWTS y catalogo del DOS 3.3 del Apple II; QDOS se ocupa del disco desde C (save.c) |
| armwat | Implement the arm move delay | no-necesaria |  | RWTS y catalogo del DOS 3.3 del Apple II; QDOS se ocupa del disco desde C (save.c) |
| armtab | Phase-on time table in 100-usec intervals | no-necesaria |  | RWTS y catalogo del DOS 3.3 del Apple II; QDOS se ocupa del disco desde C (save.c) |
| armtb2 | Phase-off time table in 100-usec intervals | no-necesaria |  | RWTS y catalogo del DOS 3.3 del Apple II; QDOS se ocupa del disco desde C (save.c) |
| prenib | Convert 256 8-bit bytes in buffer into 342 6-bit nibbles in buffr2 | no-necesaria |  | RWTS y catalogo del DOS 3.3 del Apple II; QDOS se ocupa del disco desde C (save.c) |
| pstnib | Convert 342 6-bit nibbles in buffr2 into 256 8-bit bytes in buffer | no-necesaria |  | RWTS y catalogo del DOS 3.3 del Apple II; QDOS se ocupa del disco desde C (save.c) |
| wbyte | Write one byte to disk | no-necesaria |  | RWTS y catalogo del DOS 3.3 del Apple II; QDOS se ocupa del disco desde C (save.c) |
| scttab | Lookup table to translate logical (requested) sector number to | no-necesaria |  | RWTS y catalogo del DOS 3.3 del Apple II; QDOS se ocupa del disco desde C (save.c) |
| rtable | 64 disk nibbles of "6-and-2" Read Translate Table | no-necesaria |  | RWTS y catalogo del DOS 3.3 del Apple II; QDOS se ocupa del disco desde C (save.c) |
| MUTILATE | Encrypt the commander file in the buffer at comfil | no-necesaria |  | cifrado antimanipulacion del fichero de comandante del original; save.c usa su propio cifrado XOR (formato ELQD) |
| UNMUTILATE | Decrypt the commander file in the buffer at comfil | no-necesaria |  | cifrado antimanipulacion del fichero de comandante del original; save.c usa su propio cifrado XOR (formato ELQD) |

## Sound (11 portadas de 13, 2 no necesarias)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| BELL | Make a standard system beep | propia | `snd_bell` |  |
| EXNO | Make the sound of a laser strike on another ship | propia | `snd_exno` |  |
| BOOP | Make a long, low beep | propia | `snd_boop` |  |
| SOHISS | Make the sound of a launch from the station, hyperspace or missile | propia | `snd_sohiss` |  |
| EXNO3 | Make the sound of a collision, or an exploding cargo canister or | propia | `snd_exno3` |  |
| SOEXPL | Make an explosion sound | propia | `snd_soexpl` |  |
| BEEP | Make a short, high beep | propia | `snd_beep` |  |
| SOBEEP | Make a beep | propia | `snd_sobeep` |  |
| SOBLIP | Make the sound of the hyperspace drive being engaged, or the sound | propia | `snd_soblip` |  |
| LASNOISE | Make the sound of our laser firing or the sound of us being hit by | propia | `snd_lasnoise` |  |
| LASNOISE2 | An unused routine that makes a different laser sound | no-necesaria |  | sin uso en el original |
| SOBOMB | Make the sound of an energy bomb going off | propia | `snd_sobomb` |  |
| CLICK | Toggle the state of the speaker (i.e. move it in or out) to make a | no-necesaria |  | conmutacion directa del altavoz del Apple II; el sonido sale por el IPC del QL (sound.c) |

## Stardust (7 portadas de 7)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| FLIP | Reflect the stardust particles in the screen diagonal and redraw | sin-verificar | `stars_flip` |  |
| STARS | The main routine for processing the stardust | sin-verificar | `stars_update` | vistas 0 (STARS1), 1 (STARS6), 2 y 3 (STARS2) |
| STARS1 | Process the stardust for the front view | sin-verificar | `stars1` | los acarreos del 6502 se siguen con una variable c (LSR/ROR/SBC/ADC) |
| STARS6 | Process the stardust for the rear view | sin-verificar | `stars6` |  |
| STARS2 | Process the stardust for the left or right view | sin-verificar | `stars2` | con ST2 al entrar y al salir |
| NWSTARS | Initialise the stardust field | sin-verificar | `stars_new` | el original no crea el polvo si QQ11 != 0; aqui solo se llama en la vista espacial |
| nWq | Create a random cloud of stardust | sin-verificar | `stars_new` |  |

## Start and end (7 portadas de 8, 1 sin portar)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| RESET | Reset most variables | propia | `game_new` | reinicia las variables de la partida |
| RES2 | Reset a number of flight variables and workspaces | propia | `space_reset`, `ships_clear` | reinicia las variables de vuelo |
| DEATH | Display the death screen | parcial | `flight_death`, `ships_death_debris` | D1 verificado (Ze, fq1, restos); explosion, GAME OVER y 65 vueltas del bucle de vuelo propios en la forma |
| TT170 | Main entry point for the Elite game code | propia | `game_run` | punto de entrada del juego (app.c) |
| DEATH2 | Reset most of the game and restart from the title screen | propia | `game_run` | vuelve a la pantalla de titulo (nuevo comandante o cargar) |
| BR1 | Show the "Load New Commander (Y/N)?" screen and start the game | propia | `scr_title`, `scr_disk` | el titulo ofrece cargar un comandante por el menu de disco |
| DFAULT | Reset the current commander data block to the last saved commander | sin-verificar | `cmdr_unpack` |  |
| TITLE | Display a title screen with a rotating ship and prompt | propia | `scr_title` | pantalla de titulo con la nave que gira |

## Status (7 portadas de 7)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| STATUS | Show the Status Mode screen | sin-verificar | `scr_status` | maquetacion propia; rango y estado legal en rating_name / legal_name |
| cmn | Print the commander's name | propia | `scr_status` | nombre del comandante en la pantalla de estado |
| fwl | Print fuel and cash levels | propia | `scr_status`, `ui_num` | combustible y dinero |
| csh | Print the current amount of cash | propia | `scr_status`, `ui_num` | combustible y dinero |
| BAD | Calculate how bad we have been | propia | `legal_name` | estado legal: calculo y nombre |
| BAY | Go to the docking bay (i.e. show the Status Mode screen) | propia | `scr_status` | pantalla de estado al atracar |
| EXNO2 | Process us making a kill | verificada | `exno2` | TALLY con KWL%/KWH% y mensaje 101; el sonido es snd_exno2 |

## Tactics (7 portadas de 8, 1 no necesarias)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| TACTICS | Apply tactics: Process missiles, both enemy missiles and our own | verificada | `tactics` | las 7 partes (la 7 en dockit.c); 20.000 casos con nave hija, misiles, laser, ECM, capsula |
| HITCH | Work out if the ship in INWK is in our crosshairs | verificada | `hitch` | incluido el borde del area de impacto |
| FRS1 | Launch a ship straight ahead of us, below the laser sights | verificada | `frs1` | make test_equip |
| FRMIS | Fire a missile from our ship | sin-verificar | `frmis`, `missile_fire` | FRS1 (verificada) + ANGRY + ABORT; cae en ANGRY con la A que deja SOHISS (sonido desactivado: 0) |
| ANGRY | Make a ship or station hostile, and if this is a ship then enable | verificada | `angry` | escribe en el bloque K% (INF); la estacion es la ranura 1 |
| FR1 | Display the "missile jammed" message | propia | `fr1` | MISSILE JAMMED (mensaje) |
| yetanotherrts | Contains an RTS | no-necesaria |  | contiene solo un RTS |
| SFRMIS | Add an enemy missile to our local bubble of universe | verificada | `sfrmis` | a traves de TACTICS |

## Text (34 portadas de 77, 41 no necesarias, 2 sin portar)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| MT27 | Print the captain's name during mission briefings |  |  |  |
| MT28 | Print the location hint during the mission 1 briefing |  |  |  |
| DETOK3 | Print an extended recursive token from the RUTOK token table | no-necesaria |  | motor de textos comprimidos del original: el port guarda los textos como cadenas de C (menu.c, universe.c) y no lo necesita; los textos de las misiones se escribiran como cadenas |
| DETOK | Print an extended recursive token from the TKN1 token table | no-necesaria |  | motor de textos comprimidos del original: el port guarda los textos como cadenas de C (menu.c, universe.c) y no lo necesita; los textos de las misiones se escribiran como cadenas |
| DETOK2 | Print an extended text token (1-255) | no-necesaria |  | motor de textos comprimidos del original: el port guarda los textos como cadenas de C (menu.c, universe.c) y no lo necesita; los textos de las misiones se escribiran como cadenas |
| MT1 | Switch to ALL CAPS when printing extended tokens | no-necesaria |  | motor de textos comprimidos del original: el port guarda los textos como cadenas de C (menu.c, universe.c) y no lo necesita; los textos de las misiones se escribiran como cadenas |
| MT2 | Switch to Sentence Case when printing extended tokens | no-necesaria |  | motor de textos comprimidos del original: el port guarda los textos como cadenas de C (menu.c, universe.c) y no lo necesita; los textos de las misiones se escribiran como cadenas |
| MT8 | Tab to column 6 and start a new word when printing extended tokens | no-necesaria |  | motor de textos comprimidos del original: el port guarda los textos como cadenas de C (menu.c, universe.c) y no lo necesita; los textos de las misiones se escribiran como cadenas |
| MT9 | Clear the screen and set the current view type to 1 | no-necesaria |  | motor de textos comprimidos del original: el port guarda los textos como cadenas de C (menu.c, universe.c) y no lo necesita; los textos de las misiones se escribiran como cadenas |
| MT13 | Switch to lower case when printing extended tokens | no-necesaria |  | motor de textos comprimidos del original: el port guarda los textos como cadenas de C (menu.c, universe.c) y no lo necesita; los textos de las misiones se escribiran como cadenas |
| MT6 | Switch to standard tokens in Sentence Case | no-necesaria |  | motor de textos comprimidos del original: el port guarda los textos como cadenas de C (menu.c, universe.c) y no lo necesita; los textos de las misiones se escribiran como cadenas |
| MT5 | Switch to extended tokens | no-necesaria |  | motor de textos comprimidos del original: el port guarda los textos como cadenas de C (menu.c, universe.c) y no lo necesita; los textos de las misiones se escribiran como cadenas |
| MT14 | Switch to justified text when printing extended tokens | no-necesaria |  | motor de textos comprimidos del original: el port guarda los textos como cadenas de C (menu.c, universe.c) y no lo necesita; los textos de las misiones se escribiran como cadenas |
| MT15 | Switch to left-aligned text when printing extended tokens | no-necesaria |  | motor de textos comprimidos del original: el port guarda los textos como cadenas de C (menu.c, universe.c) y no lo necesita; los textos de las misiones se escribiran como cadenas |
| MT17 | Print the selected system's adjective, e.g. Lavian for Lave | propia | `goat_soup` | adjetivo del sistema y palabra aleatoria dentro de la descripcion (universe.c) |
| MT18 | Print a random 1-8 letter word in Sentence Case | propia | `goat_soup` | adjetivo del sistema y palabra aleatoria dentro de la descripcion (universe.c) |
| MT19 | Capitalise the next letter | no-necesaria |  | motor de textos comprimidos del original: el port guarda los textos como cadenas de C (menu.c, universe.c) y no lo necesita; los textos de las misiones se escribiran como cadenas |
| VOWEL | Test whether a character is a vowel | no-necesaria |  | motor de textos comprimidos del original: el port guarda los textos como cadenas de C (menu.c, universe.c) y no lo necesita; los textos de las misiones se escribiran como cadenas |
| WHITETEXT | Switch to white text | no-necesaria |  | motor de textos comprimidos del original: el port guarda los textos como cadenas de C (menu.c, universe.c) y no lo necesita; los textos de las misiones se escribiran como cadenas |
| JMTB | The extended token table for jump tokens 1-32 (DETOK) | no-necesaria |  | motor de textos comprimidos del original: el port guarda los textos como cadenas de C (menu.c, universe.c) y no lo necesita; los textos de las misiones se escribiran como cadenas |
| TKN2 | The extended two-letter token lookup table | no-necesaria |  | motor de textos comprimidos del original: el port guarda los textos como cadenas de C (menu.c, universe.c) y no lo necesita; los textos de las misiones se escribiran como cadenas |
| QQ16 | The two-letter token lookup table | no-necesaria |  | motor de textos comprimidos del original: el port guarda los textos como cadenas de C (menu.c, universe.c) y no lo necesita; los textos de las misiones se escribiran como cadenas |
| plf2 | Print text followed by a newline and indent of 6 characters | propia | `txt_print_at`, `txt_putc` | saltos de linea, espacios, dos puntos y signos de interrogacion se escriben con las funciones de texto |
| TENS | A constant used when printing large numbers in BPRNT | propia | `ui_num`, `ui_tenths` | impresion de numeros (menu.c) |
| pr2 | Print an 8-bit number, left-padded to 3 digits, and optional point | propia | `ui_num`, `ui_tenths` | impresion de numeros (menu.c) |
| TT11 | Print a 16-bit number, left-padded to n digits, and optional point | propia | `ui_num`, `ui_tenths` | impresion de numeros (menu.c) |
| BPRNT | Print a 32-bit number, left-padded to a specific number of digits, | propia | `ui_num`, `ui_tenths` | impresion de numeros (menu.c) |
| DTW1 | A mask for applying the lower case part of Sentence Case to | no-necesaria |  | motor de textos comprimidos del original: el port guarda los textos como cadenas de C (menu.c, universe.c) y no lo necesita; los textos de las misiones se escribiran como cadenas |
| DTW2 | A flag that indicates whether we are currently printing a word | no-necesaria |  | motor de textos comprimidos del original: el port guarda los textos como cadenas de C (menu.c, universe.c) y no lo necesita; los textos de las misiones se escribiran como cadenas |
| DTW3 | A flag for switching between standard and extended text tokens | no-necesaria |  | motor de textos comprimidos del original: el port guarda los textos como cadenas de C (menu.c, universe.c) y no lo necesita; los textos de las misiones se escribiran como cadenas |
| DTW4 | Flags that govern how justified extended text tokens are printed | no-necesaria |  | motor de textos comprimidos del original: el port guarda los textos como cadenas de C (menu.c, universe.c) y no lo necesita; los textos de las misiones se escribiran como cadenas |
| DTW5 | The size of the justified text buffer at BUF | no-necesaria |  | motor de textos comprimidos del original: el port guarda los textos como cadenas de C (menu.c, universe.c) y no lo necesita; los textos de las misiones se escribiran como cadenas |
| DTW6 | A flag to denote whether printing in lower case is enabled for | no-necesaria |  | motor de textos comprimidos del original: el port guarda los textos como cadenas de C (menu.c, universe.c) y no lo necesita; los textos de las misiones se escribiran como cadenas |
| DTW8 | A mask for capitalising the next letter in an extended text token | no-necesaria |  | motor de textos comprimidos del original: el port guarda los textos como cadenas de C (menu.c, universe.c) y no lo necesita; los textos de las misiones se escribiran como cadenas |
| FEED | Print a newline | propia | `txt_print_at`, `txt_putc` | saltos de linea, espacios, dos puntos y signos de interrogacion se escriben con las funciones de texto |
| MT16 | Print the character in variable DTW7 | no-necesaria |  | motor de textos comprimidos del original: el port guarda los textos como cadenas de C (menu.c, universe.c) y no lo necesita; los textos de las misiones se escribiran como cadenas |
| TT26 | Print a character at the text cursor, with support for verified | propia | `txt_putc`, `draw_glyph` | impresion de caracteres sobre la pantalla grafica (text.c) |
| MT23 | Move to row 9 and switch to lower case | no-necesaria |  | motor de textos comprimidos del original: el port guarda los textos como cadenas de C (menu.c, universe.c) y no lo necesita; los textos de las misiones se escribiran como cadenas |
| MT29 | Move to row 5 and switch to lower case when printing extended | no-necesaria |  | motor de textos comprimidos del original: el port guarda los textos como cadenas de C (menu.c, universe.c) y no lo necesita; los textos de las misiones se escribiran como cadenas |
| MTIN | Lookup table for random tokens in the extended token table (0-37) | no-necesaria |  | motor de textos comprimidos del original: el port guarda los textos como cadenas de C (menu.c, universe.c) y no lo necesita; los textos de las misiones se escribiran como cadenas |
| DOXC | Move the text cursor to a specific column | propia | `txt_col` | columna del cursor |
| DOYC | Move the text cursor to a specific row | propia | `txt_row` | fila del cursor |
| INCYC | Move the text cursor to the next row | propia | `txt_row` | fila del cursor |
| TT60 | Print a text token and a paragraph break | propia | `txt_print_at`, `txt_putc` | saltos de linea, espacios, dos puntos y signos de interrogacion se escriben con las funciones de texto |
| TTX69 | Print a paragraph break | propia | `txt_print_at`, `txt_putc` | saltos de linea, espacios, dos puntos y signos de interrogacion se escriben con las funciones de texto |
| TT69 | Set Sentence Case and print a newline | propia | `txt_print_at`, `txt_putc` | saltos de linea, espacios, dos puntos y signos de interrogacion se escriben con las funciones de texto |
| TT67 | Print a newline | propia | `txt_print_at`, `txt_putc` | saltos de linea, espacios, dos puntos y signos de interrogacion se escriben con las funciones de texto |
| spc | Print a text token followed by a space | propia | `txt_print_at`, `txt_putc` | saltos de linea, espacios, dos puntos y signos de interrogacion se escriben con las funciones de texto |
| pr6 | Print 16-bit number, left-padded to 5 digits, no point | propia | `ui_num`, `ui_tenths` | impresion de numeros (menu.c) |
| pr5 | Print a 16-bit number, left-padded to 5 digits, and optional point | propia | `ui_num`, `ui_tenths` | impresion de numeros (menu.c) |
| prq | Print a text token followed by a question mark | propia | `txt_print_at`, `txt_putc` | saltos de linea, espacios, dos puntos y signos de interrogacion se escriben con las funciones de texto |
| TT162 | Print a space | propia | `txt_print_at`, `txt_putc` | saltos de linea, espacios, dos puntos y signos de interrogacion se escriben con las funciones de texto |
| dn2 | Make a low, long beep and delay for 0.5 seconds | propia | `snd_boop` | pitido largo y espera |
| plf | Print a text token followed by a newline | propia | `txt_print_at`, `txt_putc` | saltos de linea, espacios, dos puntos y signos de interrogacion se escriben con las funciones de texto |
| TT68 | Print a text token followed by a colon | propia | `txt_print_at`, `txt_putc` | saltos de linea, espacios, dos puntos y signos de interrogacion se escriben con las funciones de texto |
| TT73 | Print a colon | propia | `txt_print_at`, `txt_putc` | saltos de linea, espacios, dos puntos y signos de interrogacion se escriben con las funciones de texto |
| TT27 | Print a text token | no-necesaria |  | motor de textos comprimidos del original: el port guarda los textos como cadenas de C (menu.c, universe.c) y no lo necesita; los textos de las misiones se escribiran como cadenas |
| TT42 | Print a letter in lower case | no-necesaria |  | motor de textos comprimidos del original: el port guarda los textos como cadenas de C (menu.c, universe.c) y no lo necesita; los textos de las misiones se escribiran como cadenas |
| TT41 | Print a letter according to Sentence Case | no-necesaria |  | motor de textos comprimidos del original: el port guarda los textos como cadenas de C (menu.c, universe.c) y no lo necesita; los textos de las misiones se escribiran como cadenas |
| qw | Print a recursive token in the range 128-145 | no-necesaria |  | motor de textos comprimidos del original: el port guarda los textos como cadenas de C (menu.c, universe.c) y no lo necesita; los textos de las misiones se escribiran como cadenas |
| crlf | Tab to column 21 and print a colon | propia | `txt_print_at`, `txt_putc` | saltos de linea, espacios, dos puntos y signos de interrogacion se escriben con las funciones de texto |
| TT45 | Print a letter in lower case | no-necesaria |  | motor de textos comprimidos del original: el port guarda los textos como cadenas de C (menu.c, universe.c) y no lo necesita; los textos de las misiones se escribiran como cadenas |
| TT46 | Print a character and switch to capitals | no-necesaria |  | motor de textos comprimidos del original: el port guarda los textos como cadenas de C (menu.c, universe.c) y no lo necesita; los textos de las misiones se escribiran como cadenas |
| TT74 | Print a character | no-necesaria |  | motor de textos comprimidos del original: el port guarda los textos como cadenas de C (menu.c, universe.c) y no lo necesita; los textos de las misiones se escribiran como cadenas |
| TT43 | Print a two-letter token or recursive token 0-95 | no-necesaria |  | motor de textos comprimidos del original: el port guarda los textos como cadenas de C (menu.c, universe.c) y no lo necesita; los textos de las misiones se escribiran como cadenas |
| ex | Print a recursive token | no-necesaria |  | motor de textos comprimidos del original: el port guarda los textos como cadenas de C (menu.c, universe.c) y no lo necesita; los textos de las misiones se escribiran como cadenas |
| MT26 | Fetch a line of text from the keyboard | propia | `edit_text` | lectura de una linea del teclado |
| RLINE | The OSWORD configuration block used to fetch a line of text from | propia | `edit_text` | lectura de una linea del teclado |
| CHPR2 | Character print vector handler | no-necesaria |  | impresion en el modo de texto del Apple II; el QL dibuja siempre en modo grafico |
| R5 | Make a beep and jump back into the character-printing routine at | no-necesaria |  | impresion en el modo de texto del Apple II; el QL dibuja siempre en modo grafico |
| RR5 | Print a character in the text screen mode | no-necesaria |  | impresion en el modo de texto del Apple II; el QL dibuja siempre en modo grafico |
| TT67K | Print a newline | propia | `txt_print_at`, `txt_putc` | saltos de linea, espacios, dos puntos y signos de interrogacion se escriben con las funciones de texto |
| CHPR | Print a character at the text cursor by poking into screen memory | propia | `txt_putc`, `draw_glyph` | impresion de caracteres sobre la pantalla grafica (text.c) |
| letter | Draw a character in the high-resolution screen mode using the game | propia | `txt_putc`, `draw_glyph` | impresion de caracteres sobre la pantalla grafica (text.c) |
| letter2 | Draw a character or indicator bulb bitmap in the high-resolution | propia | `txt_putc`, `draw_glyph` | impresion de caracteres sobre la pantalla grafica (text.c) |
| SETXC | Move the text cursor to a specific column | propia | `txt_col` | columna del cursor |
| SETYC | Move the text cursor to a specific row | propia | `txt_row` | fila del cursor |

## Tube (0 portadas de 1, 1 no necesarias)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| newosrdch | The custom OSRDCH routine for reading characters | no-necesaria |  | rutina OSRDCH personalizada del BBC con procesador Tube; no aplica |

## Universe (32 portadas de 34, 2 no necesarias)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| SPIN | Randomly spawn cargo from a destroyed ship | verificada | `spin`, `spin2` | la cantidad es tipo AND byte 0 AND 15 (el TYA tapa el numero aleatorio) |
| UNIV | Table of pointers to the local universe's ship data blocks | propia | `state_slot` | tabla de punteros a los bloques de las naves |
| SFS1 | Spawn a child ship from the current (parent) ship | verificada | `sfs1` | nave hija desde K%; velocidad de los restos = X de DORND AND 15; acarreos modelados |
| PDESC | Print the system's extended description or a mission 1 directive | propia | `uni_describe` | descripcion del sistema; falta la directiva de la mision 1 |
| GINF | Fetch the address of a ship's data block into INF | no-necesaria |  | el puntero INF no existe en C: se indexa por ranura |
| ping | Set the selected system to the current system | sin-verificar | `chart_ping` | tecla O: el cursor vuelve al sistema actual |
| TT20 | Twist the selected system's seeds four times | propia | `twist`, `nextgalaxy` | giro de las semillas (universe.c, de Text Elite) |
| TT54 | Twist the selected system's seeds | propia | `twist`, `nextgalaxy` | giro de las semillas (universe.c, de Text Elite) |
| TT146 | Print the distance to the selected system in light years | propia | `chart_info` | distancia en anos luz |
| TT70 | Display "MAINLY " and jump to TT72 | propia | `uni_describe` | MAINLY en la descripcion del sistema |
| TT25 | Show the Data on System screen | sin-verificar | `scr_data` |  |
| TT24 | Calculate system data from the system seeds | propia | `uni_make_system`, `uni_build_galaxy` | datos del sistema y semillas de la galaxia (universe.c: iguales al oraculo de Text Elite) |
| TT81 | Set the selected system's seeds to those of system 0 | propia | `uni_make_system`, `uni_build_galaxy` | datos del sistema y semillas de la galaxia (universe.c: iguales al oraculo de Text Elite) |
| TT111 | Set the current system to the nearest system to a point | sin-verificar | `uni_nearest` |  |
| jmp | Set the current system to the selected system | propia | `game_enter_system`, `jump_do` | entrada en el sistema y salto al mas cercano |
| hyp1 | Process a jump to the system closest to (QQ9, QQ10) | propia | `game_enter_system`, `jump_do` | entrada en el sistema y salto al mas cercano |
| GVL | Calculate the availability of market items | propia | `uni_gen_market` | disponibilidad del mercado |
| GTHG | Spawn a Thargoid ship and a Thargon companion | verificada | `gthg` | make test_equip |
| cpl | Print the selected system name | propia | `put_name`, `chart_info` | nombre del sistema, del actual y numero de galaxia |
| ypl | Print the current system name | propia | `put_name`, `chart_info` | nombre del sistema, del actual y numero de galaxia |
| tal | Print the current galaxy number | propia | `put_name`, `chart_info` | nombre del sistema, del actual y numero de galaxia |
| SOS1 | Update the missile indicators, set up the planet data block | sin-verificar | `make_planet` |  |
| SOLAR | Set up various aspects of arriving in a new system | sin-verificar | `ships_launch` | planeta con las formulas de SOLAR; al despegar el sol se sustituye por la estacion; ships_arrive pone planeta y sol |
| NWSPS | Add a new space station to our local bubble of universe | sin-verificar | `make_station` | la posicion de la estacion es propia |
| NWSHP | Add a new ship to our local bubble of universe | sin-verificar | `ship_new` | sin heap compartido (SLSP): un monton de 256 bytes por ranura; NEWB inicial de E% |
| NwS1 | Flip the sign and double an INWK byte | propia | `make_station` | los tres NwS1 que invierten el morro de la estacion |
| KS3 | Set the SLSP ship line heap pointer after shuffling ship slots | no-necesaria |  | puntero SLSP del montón de lineas: ships.c usa un montón fijo por ranura |
| KS1 | Remove the current ship from our local bubble of universe | propia | `ship_kill` | quita la nave y desplaza las demas |
| KS4 | Remove the space station and replace it with the sun | propia | `ships_step` | la estacion se quita y vuelve el sol (regla propia) |
| KS2 | Check the local bubble for missiles with target lock | verificada | `ks2` | make test_tactics |
| KILLSHP | Remove a ship from our local bubble of universe | sin-verificar | `ship_kill` | sin puntos de muerte ni mensajes (paso 3) |
| ZINF | Reset the INWK workspace and orientation vectors | sin-verificar | `ship_zinf` |  |
| Ze | Initialise the INWK workspace to a fairly aggressive ship | verificada | `ze` | dentro de D1 |
| spasto | Contains the address of the Coriolis space station's ship | propia | `ship_bp_for` | plano de la estacion (Coriolis o Dodo) |

## Utility routines (1 portadas de 18, 17 no necesarias)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| DEEOR | Unscramble the main code | no-necesaria |  | proteccion anticopia del original: suma de comprobacion, descifrado y desmezcla del codigo; en el QL no existe |
| DEEORS | Decrypt a multi-page block of memory | no-necesaria |  | proteccion anticopia del original: suma de comprobacion, descifrado y desmezcla del codigo; en el QL no existe |
| G% | Denotes the start of the main game code, from ELITE A to ELITE K | no-necesaria |  | proteccion anticopia del original: suma de comprobacion, descifrado y desmezcla del codigo; en el QL no existe |
| DELAY | Wait for a specified time, measured in loop iterations | no-necesaria |  | retardo por bucles de instrucciones; el port espera por fotogramas (wait_frames) y por tiempo (snd_tick) |
| SWAPPZERO | An unused routine that swaps bytes in and out of zero page | no-necesaria |  | sin uso en el original (intercambio de la pagina cero) |
| brkd | A flag that indicates whether a system error has occurred | no-necesaria |  | manejador de errores BRK del 6502; el runtime de C se ocupa de los errores |
| BRBR | The standard BRKV handler for the game | no-necesaria |  | manejador de errores BRK del 6502; el runtime de C se ocupa de los errores |
| ZERO | Reset the local bubble of universe and ship status | propia | `ships_clear` | reinicio de la burbuja local: ranuras, contadores y montones de lineas |
| ZEBC | Zero-fill pages $B and $C | no-necesaria |  | relleno con ceros y copia de paginas de memoria: en C son memset, memcpy o variables estaticas |
| ZES1 | Zero-fill the page whose number is in X | no-necesaria |  | relleno con ceros y copia de paginas de memoria: en C son memset, memcpy o variables estaticas |
| ZES2 | Zero-fill a specific page | no-necesaria |  | relleno con ceros y copia de paginas de memoria: en C son memset, memcpy o variables estaticas |
| backtonormal | Disable the keyboard, set the SVN flag to 0, and return with A = 0 | no-necesaria |  | desactiva el teclado del BBC y vuelve con A = 0; no aplica |
| CLDELAY | Delay by iterating through 5 * 256 (1280) empty loops | no-necesaria |  | retardo por bucles de instrucciones; el port espera por fotogramas (wait_frames) y por tiempo (snd_tick) |
| ZES1k | Zero-fill the page whose number is in X | no-necesaria |  | relleno con ceros y copia de paginas de memoria: en C son memset, memcpy o variables estaticas |
| ZES2k | Zero-fill a specific page | no-necesaria |  | relleno con ceros y copia de paginas de memoria: en C son memset, memcpy o variables estaticas |
| ZESNEW | Zero-fill memory from SC(1 0) to the end of the page | no-necesaria |  | relleno con ceros y copia de paginas de memoria: en C son memset, memcpy o variables estaticas |
| mvblockK | Copy a specific number of pages in memory | no-necesaria |  | relleno con ceros y copia de paginas de memoria: en C son memset, memcpy o variables estaticas |
| F% | Denotes the end of the main game code, from ELITE A to ELITE K | no-necesaria |  | proteccion anticopia del original: suma de comprobacion, descifrado y desmezcla del codigo; en el QL no existe |

## Workspaces (0 portadas de 7, 7 no necesarias)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| ZP | Lots of important variables are stored in the zero page workspace | no-necesaria |  | la pagina cero se reparte en variables globales de C (state.c, emath.c): lo da el compilador |
| XX3 | Temporary storage space for complex calculations | no-necesaria |  | tabla de coordenadas de pantalla de los vertices: matriz xx3 de ship3d.c |
| K% | Ship data blocks and ship line heaps | no-necesaria |  | bloques de las naves: matriz de state.c (state_slot) y un monton de lineas por ranura en ships.c |
| Disk | Variables used by the disk operations and DOS 3.3 RWTS routines | no-necesaria |  | variables de las operaciones de disco y de la RWTS del DOS 3.3: el QL usa QDOS |
| UP | Configuration variables | no-necesaria |  | variables de configuracion: se sustituyen por variables de C (state.c, sound.c); los interruptores se portaran con TGINT y DKS3 |
| WP | Variables | no-necesaria |  | montones de la bola, del sol y de las naves y datos del polvo estelar: matrices de C (sun.c, stars.c, ships.c) |
| Option | Variables that are predominantly used to store the game options | no-necesaria |  | variables de configuracion: se sustituyen por variables de C (state.c, sound.c); los interruptores se portaran con TGINT y DKS3 |
