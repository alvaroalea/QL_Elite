# Inventario de rutinas (Elite Apple II -> Elite QL)

Generado por `tools/inventory.py` a partir de la fuente de referencia (Mark Moxon, Elite Apple II) y de `tools/ported.txt`. No se edita a mano: se cambia `tools/ported.txt` y se ejecuta `python3 tools/inventory.py`.

## Resumen

- Rutinas de la fuente: **528**
- **Portadas** (con la funcion C que las cubre): **426**, de ellas verificadas contra el 6502: 298, **no verificables** (dependen del hardware del Apple II o de pantallas propias; no se vuelven a pedir): 57 y pendientes de verificar: 71
- **No necesarias** en la version QL (cargador, proteccion anticopia, RWTS del DOS, tablas de HGR, utilidades que da el compilador de C, etc.): **102**
- **Sin portar** (pendientes de implementar, incluidas las parciales): **0**

Comprobacion: total = portadas + no necesarias + sin portar (528 = 426 + 102 + 0).

| Area | Total | Portadas | De ellas verificadas | No verificables | No necesarias | Pendientes verificar | Pendientes implementar |
|---|---:|---:|---:|---:|---:|---:|---:|
| Charts | 9 | 9 | 9 | 0 | 0 | 0 | 0 |
| Dashboard | 29 | 29 | 4 | 0 | 0 | 25 | 0 |
| Drawing circles | 9 | 9 | 9 | 0 | 0 | 0 | 0 |
| Drawing lines | 27 | 23 | 20 | 3 | 4 | 0 | 0 |
| Drawing pixels | 6 | 3 | 3 | 0 | 3 | 0 | 0 |
| Drawing planets | 7 | 7 | 7 | 0 | 0 | 0 | 0 |
| Drawing ships | 5 | 4 | 4 | 0 | 1 | 0 | 0 |
| Drawing suns | 3 | 3 | 3 | 0 | 0 | 0 | 0 |
| Drawing the screen | 18 | 8 | 7 | 1 | 10 | 0 | 0 |
| Equipment | 6 | 6 | 5 | 1 | 0 | 0 | 0 |
| Flight | 29 | 29 | 12 | 0 | 0 | 17 | 0 |
| Keyboard | 19 | 15 | 3 | 12 | 4 | 0 | 0 |
| Loader | 5 | 0 | 0 | 0 | 5 | 0 | 0 |
| Main loop | 1 | 1 | 1 | 0 | 0 | 0 | 0 |
| Market | 19 | 19 | 8 | 11 | 0 | 0 | 0 |
| Maths (Arithmetic) | 52 | 51 | 22 | 0 | 1 | 29 | 0 |
| Maths (Geometry) | 19 | 19 | 19 | 0 | 0 | 0 | 0 |
| Missions | 16 | 10 | 10 | 0 | 6 | 0 | 0 |
| Moving | 8 | 8 | 8 | 0 | 0 | 0 | 0 |
| Save and load | 61 | 27 | 7 | 20 | 34 | 0 | 0 |
| Sound | 13 | 11 | 10 | 1 | 2 | 0 | 0 |
| Stardust | 7 | 7 | 7 | 0 | 0 | 0 | 0 |
| Start and end | 8 | 8 | 3 | 5 | 0 | 0 | 0 |
| Status | 7 | 7 | 7 | 0 | 0 | 0 | 0 |
| Tactics | 8 | 7 | 7 | 0 | 1 | 0 | 0 |
| Text | 77 | 73 | 70 | 3 | 4 | 0 | 0 |
| Tube | 1 | 0 | 0 | 0 | 1 | 0 | 0 |
| Universe | 34 | 32 | 32 | 0 | 2 | 0 | 0 |
| Utility routines | 18 | 1 | 1 | 0 | 17 | 0 | 0 |
| Workspaces | 7 | 0 | 0 | 0 | 7 | 0 | 0 |
| **Total** | **528** | **426** | **298** | **57** | **102** | **71** | **0** |

Columnas: *Portadas* = verificadas + no verificables + pendientes de verificar (incluye los equivalentes propios); *No verificables* = portadas que no tienen equivalente comparable en el original (estado no-verificable: no se piden en las rondas de verificacion); *Pendientes verificar* = estados transcripcion, sin-verificar y propia; *Pendientes implementar* = sin portar o parciales.

## Rutinas sin portar, por area


## Estados

**verificada** = comparada con el 6502 original (emulador); **transcripcion** = comparada con una transcripcion literal propia del 6502 (falta el emulador); **sin-verificar** = portada, anotada en PENDIENTES.md; **propia** = sustituto o equivalente propio (la funcion C cubre lo que hacia la rutina, con otra forma); **parcial** = existe una version reducida y falta parte (cuenta como pendiente de implementar); **no-verificable** = portada, pero no se puede comparar con el 6502 (depende del hardware del Apple II, como el modo texto, o de pantallas propias del port): queda fuera de los pendientes de verificar; **no-necesaria** = no hace falta en la version QL (la nota dice por que); vacio = sin portar.

## Charts (9 portadas de 9)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| HME2 | Search the galaxy for a system | verificada | `chart_find_system` | make test_charts (busqueda con el motor de texto real: nombre completo, sin mayusculas, misma longitud); la entrada de texto (MT26) es edit_text. El original con QQ17 = $80 no encuentra el sistema 0 y con 0 o $40 ninguno (con $C0, el estado normal, todos) |
| TT22 | Show the Long-range Chart | verificada | `chart_draw`, `tt22` | make test_charts (lineas, marco, cada PIXEL con ZZ = 255 = dos puntos, TT14 y TT103); el titulo es el de menu.c |
| TT16 | Move the crosshairs on a chart | verificada | `chart_tt16` | make test_charts (borra la cruz, TT123 en y con -Y y en x, y cae en TT103); scr_chart la usa con pasos de 1 (local) o 2 (galactica) por fotograma; el objetivo que se muestra abajo es el de TT111 (propio) |
| TT103 | Draw a small set of crosshairs on a chart | verificada | `chart_cursor` | make test_charts |
| TT123 | Move galactic coordinates by a signed delta | verificada | `tt123` | make test_charts (no se mueve si daria la vuelta) |
| TT105 | Draw crosshairs on the Short-range Chart, with clipping | verificada | `chart_cursor` | make test_charts |
| TT23 | Show the Short-range Chart | verificada | `chart_draw`, `tt23` | make test_charts (filas libres de INWK, nombres con XC/YC, SUN con el acarreo de cpl, readdistnce); con Y = 255 lee INWK-1 = SC+1 (nunca 0) |
| TT114 | Display either the Long-range or Short-range Chart | verificada | `chart_draw` | make test_charts (galactica o local segun QQ11) |
| hm | Select the closest system and redraw the chart crosshairs | verificada | `chart_hm` | make test_charts |

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
| LL164 | Make the hyperspace sound and draw the hyperspace tunnel | verificada | `tunnel_ll164` | make test_draw (nueve SOBLIP, HFS2 con STP = 4, cada SOHISS y CIRCLE2, RAND); el primer SBC |
| LAUN | Make the launch sound and draw the launch tunnel | verificada | `tunnel_laun` | make test_draw (dos SOHISS de 256, HFS2 con STP = 8; RAND); en TT110 los mismos anillos se borran despues con HFS1 |
| HFS2 | Draw the launch or hyperspace tunnel | verificada | `tunnel_draw` | HFS1/HFL1 contra el 6502 (make test_planet); STP 4 hiperespacio, 8 lanzamiento; el color (blanco) es una eleccion propia |
| TT14 | Draw a circle with crosshairs on a chart | verificada | `tt14` | make test_charts (TT126 en la local; cruz, K de SCALEY/SCALEY2 y TT128) |
| TT128 | Draw a circle on a chart | verificada | `tt14` | make test_planet (circulo de las cartas con CIRCLE2) |
| CIRCLE | Draw a circle for the planet | verificada | `circle_draw` | make test_planet (secuencias de PLANET contra el 6502) |
| CIRCLE2 | Draw a circle (for the planet or chart) | verificada | `circle2_draw` | make test_planet (secuencias de PLANET contra el 6502) |
| CHKON | Check whether any part of a circle appears on the extended screen | verificada | `sun_chkon` | make test_planet (secuencias de PLANET contra el 6502) |

## Drawing lines (23 portadas de 27, 4 no necesarias)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| BOMBOFF | Draw the zig-zag lightning bolt for the energy bomb | verificada | `bomb_off` | make test_equip (lineas del rayo) |
| BOMBEFF2 | Erase the energy bomb zig-zag lightning bolt, make the sound of | verificada | `bomb_eff2`, `bomb_eff`, `bomb_frame` | make test_draw (con SOBOMB, CLICK y DORND reales; QQ11 y DNOIZ al azar); bomb_frame (parte 16) y la tecla con la comprobacion de la version publicada (IB disk) sin verificar |
| BOMBON | Randomise and draw the energy bomb's zig-zag lightning bolt lines | verificada | `bomb_on` | make test_equip |
| BOMBPOS | A set of x-coordinates that are used as the basis for the energy | verificada | `bombpos` | tabla |
| BOMBTBX | This is where we store the x-coordinates for the energy bomb's | verificada | `bombtbx` |  |
| BOMBTBY | This is where we store the y-coordinates for the energy bomb's | verificada | `bombtby` |  |
| NLI4 | Draw a line of dashes underneath a title on the text screen | no-verificable | `nlin4` | guiones bajo la fila 1 en el modo texto del Apple II; el QL no tiene modo texto y NLIN4 dibuja siempre la linea de NLIN2 en y = 19 (en medio de la fila 2) |
| NLIN3 | Print a title and draw a horizontal line at row 19 to box it in | no-verificable | `ui_title` | titulo con la linea que lo subraya (menu.c); en las pantallas de misiones, TT27 + nlin4 (dt_nlin4_hook) |
| NLIN4 | Draw a horizontal line at pixel row 19 to box in a title | verificada | `nlin4` | make test_draw (modo grafico: NLIN2 con y = 19) |
| NLIN | Draw a horizontal line at pixel row 23 to box in a title | no-verificable | `ui_title` | titulo con la linea que lo subraya (menu.c); NLIN = INCYC + NLIN2(23) |
| NLIN2 | Draw a screen-wide horizontal line at the pixel row in A | verificada | `nlin2` | make test_draw (HLOIN de 0 a 255 en azul) |
| HLOIN2 | Remove a line from the sun line heap and draw it on-screen | verificada | `hloin2` | make test_planet (secuencias de PLANET contra el 6502) |
| LASLI | Draw the laser lines for when we fire our lasers | verificada | `lasli`, `lasli2` | vertice, calor, DENGY y las cuatro lineas (LL30); LASLI2 borra en XOR |
| DVLOIN | Draw a vertical line from (A, GCYT) to (A, GCYB) | verificada | `dvloin` | por inspeccion: VLOIN de (A, GCYT) a (A, GCYB), y VLOIN esta verificada con make test_lines |
| TT15 | Draw a set of crosshairs | verificada | `tt15`, `chart_tt15` | make test_draw (HLOIN y VLOIN con sus argumentos y COL) |
| EDGES | Draw a horizontal line given a centre and a half-width | verificada | `edges` | make test_planet (secuencias de PLANET contra el 6502) |
| LL118 | Move a point along a line until it is on-screen | verificada | `ship_clip` | dentro de ship3d.c (verificada con el 6502) |
| LL145 | Clip line: Work out which end-points are on-screen, if any | verificada | `ship_clip` | recorte de linea (verificada con el 6502) |
| LSPUT | Draw a ship line using flicker-free animation | verificada | `ship_lsput` | linea de nave en el monton con animacion sin parpadeo (dentro de LL9, verificada) |
| sightcol | Colours for the crosshair sights on the different laser types | verificada | `chart_sight` | make test_draw (azul, rojo, blanco, blanco) |
| beamcol | An unused table of laser colours | no-necesaria |  | tabla sin uso en el original |
| TWFL | Ready-made pixel bytes for the left end of a horizontal line | no-necesaria |  | tablas de la pantalla de alta resolucion del Apple II (direcciones de fila, mascaras de pixel y colores); gfx.c escribe directamente en la pantalla del QL |
| TWFR | Ready-made character rows for the right end of a horizontal line | no-necesaria |  | tablas de la pantalla de alta resolucion del Apple II (direcciones de fila, mascaras de pixel y colores); gfx.c escribe directamente en la pantalla del QL |
| LOIN | Draw a line: Calculate the line gradient in the form of deltas | verificada | `loin_draw` | make test_lines punto a punto (pendiente con log/alogh, un extremo sin dibujar segun SWAP); en sun.c, loin() reproduce ademas los efectos en memoria (SWAP, X1..Y2, P, Q, S) de make test_planet |
| HLOIN | Draw a horizontal line from (X1, Y1) to (X2, Y1) | verificada | `gfx_hloin`, `hloin`, `lines_hloin` | make test_lines punto a punto en blanco (pares de puntos de X1 a X2-1); los colores del Apple II son patrones de bits (MASKT): en el QL, color liso; hloin() de sun.c deja X1/X2 como el original (make test_planet) |
| MASKT | High-resolution pixel bytes for drawing continuous lines of solid | no-necesaria |  | tablas de la pantalla de alta resolucion del Apple II (direcciones de fila, mascaras de pixel y colores); gfx.c escribe directamente en la pantalla del QL |
| VLOIN | Draw a vertical line from (X1, Y1) to (X1, Y2) | verificada | `gfx_vloin` | make test_lines punto a punto (CPIX de Y1 a Y2, ambos incluidos) |

## Drawing pixels (3 portadas de 6, 3 no necesarias)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| PIXEL2 | Draw a stardust particle relative to the screen centre | verificada | `pixel2` | make test_stars (con el acarreo con que vuelve PIXEL) |
| PIXEL | Draw a two-pixel dash, three-pixel dash or double-height | verificada | `gfx_dust` | make test_lines punto a punto (2 o 3 puntos segun ZZ y dos filas si ZZ < 80; el QL no muestra x >= 256) |
| TWOS3 | Ready-made two-pixel and three-pixel bytes in white, with an extra | no-necesaria |  | tabla de bytes de 2 y 3 puntos del modo hires del Apple II; gfx_dust pone los puntos directamente (make test_lines) |
| TWOS | Ready-made bytes for drawing one-pixel dots in the space view | no-necesaria |  | tabla de un punto del modo hires del Apple II; loin_draw pone el punto directamente (make test_lines) |
| TWOS2 | Ready-made two-bit pixel bytes for plotting colour pixels | no-necesaria |  | tabla de dos puntos del modo hires del Apple II (CPIX); gfx_cpix pone los puntos directamente (make test_lines) |
| CPIX | Draw a colour pixel | verificada | `gfx_cpix` | a traves de VLOIN en make test_lines (2 puntos de ancho en XOR); sin los patrones de color del Apple II |

## Drawing planets (7 portadas de 7)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| PL2 | Remove the planet or sun from the screen | verificada | `planet` | make test_planet |
| PLANET | Draw the planet or sun | verificada | `planet` | make test_planet (con PL9, PL2, WP1) |
| PL9 | Draw the planet as a plain circle | verificada | `planet` | make test_planet |
| WPLS2 | Remove the planet from the screen | verificada | `circle_wpls2` | make test_planet; depende de que LOIN deje SWAP como el original (loin() de sun.c) |
| WP1 | Reset the ball line heap | verificada | `circle_wpls2` | make test_planet; RES2 pone LSX2 y LSY2 a $FF |
| PL21 | Return from a planet/sun-drawing routine with a failure flag | verificada | `planet` | make test_planet (SEC; RTS) |
| PLS6 | Calculate (X K) = (A P+1 P) / (z_sign z_hi z_lo) | verificada | `sm_pls6` | make test_geom (codigo original en el emulador) |

## Drawing ships (4 portadas de 5, 1 no necesarias)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| scacol | Ship colours on the scanner | verificada | `scacol`, `space_scacol` | make test_draw (tabla); CYAN y MAG son WHITE en el Apple II: el QL los muestra en cian y magenta |
| DOEXP | Draw an exploding ship | verificada | `doexp` | make test_draw (CPIX con X, Y y COL; monton, INWK+31 y RAND); el color de las particulas (azul/rojo/fuzzy/otro del Apple II) se muestra como cian/rojo/amarillo/blanco |
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
| TRADEMODE | Clear the screen and set up a trading screen | no-verificable | `ui_clear`, `gfx_clear` | TT66 + FLKB de las pantallas de comercio, que en el port son propias (menu.c) |
| DET1 | Show or hide the dashboard (for when we die) | no-necesaria |  | en el Apple II es un RTS |
| TT66 | Clear the screen and set the current view type | verificada | `tt66` | make test_screen |
| TTX66 | Clear the top part of the screen, draw a border box if required | verificada | `tt66` | make test_screen (MT2, LSP = 0, QQ17, DTW2, XC/YC, FLFLLS, LAS2, DLY, de, ee3 y el nombre de la vista con TT27) |
| cellocl | Lookup table for converting a text row number to the address of | no-necesaria |  | tablas de la pantalla de alta resolucion del Apple II (direcciones de fila, mascaras de pixel y colores); gfx.c escribe directamente en la pantalla del QL |
| SCTBL | Lookup table for converting a character row number to the address | no-necesaria |  | tablas de la pantalla de alta resolucion del Apple II (direcciones de fila, mascaras de pixel y colores); gfx.c escribe directamente en la pantalla del QL |
| SCTBH | Lookup table for converting a character row number to the address | no-necesaria |  | tablas de la pantalla de alta resolucion del Apple II (direcciones de fila, mascaras de pixel y colores); gfx.c escribe directamente en la pantalla del QL |
| SCTBH2 | Lookup table for converting a character row number to the address | no-necesaria |  | tablas de la pantalla de alta resolucion del Apple II (direcciones de fila, mascaras de pixel y colores); gfx.c escribe directamente en la pantalla del QL |
| WSCAN | Wait for 15 * 256 loop iterations | no-necesaria |  | retardo por bucles de instrucciones; el port espera por fotogramas (wait_frames) y por tiempo (snd_tick) |
| clss | Clear the screen, move the text cursor to the top-left corner and | verificada | `dt_clss_hook` | a traves de CHPR en make test_missions (pasar de la fila 23 borra y vuelve a (0, 0)) |
| TTX66K | Clear the whole screen or just the space view (as appropriate), | verificada | `ttx66k` | make test_screen (vistas graficas: cleargrap y BOX; de texto: cleartext; el QL no tiene modo texto) |
| cleargrap | Clear screen memory for the top part of the graphics screen mode | verificada | `tt66`, `clear_view` | make test_lines (puntos apagados: filas 0..135; los bordes azules del Apple quedan fuera de los 256 puntos) |
| CLYNS | Clear two character rows near the bottom of the screen | verificada | `clyns` | make test_screen (DLY, de, XC/YC, DTW2, QQ17; 64 espacios en las vistas de texto o clearrow 15 y 16 en las graficas) |
| clearrow | Clear a character row of screen memory, drawing blue borders along | verificada | `clyns` | make test_lines (filas 15 y 16 de CLYNS: y = 120..135) |
| HGR | Switch to the high-resolution graphics screen mode | no-necesaria |  | cambio de modo grafico y de texto del Apple II; el QL usa un solo modo (modo 8) |
| TEXT | Switch to the text screen mode | no-necesaria |  | cambio de modo grafico y de texto del Apple II; el QL usa un solo modo (modo 8) |

## Equipment (6 portadas de 6)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| EQSHP | Show the Equip Ship screen | verificada | `equip_buy` | make test_eqshp (3.000 EQSHP del original con eq, prx, refund, PRXS, LCASH y MCASH reales): cobra primero (sin dinero se avisa aunque ya lo tengas), si ya lo tienes devuelve el importe, laser antiguo reembolsado; la pantalla es scr_equip (propia) |
| eq | Subtract the price of equipment from the cash pot | verificada | `cash_spend` | make test_eqshp (3.000 EQSHP del original con eq, prx, refund, PRXS, LCASH y MCASH reales) |
| prx | Return the price of a piece of equipment | verificada | `equip_price` | make test_eqshp (3.000 EQSHP del original con eq, prx, refund, PRXS, LCASH y MCASH reales); el combustible (70 - QQ14) * 2 |
| qv | Print a menu of the four space views, for buying lasers | no-verificable | `scr_equip` | pide la vista (0 a 3) con un menu propio; en el original es TT217 + pantalla |
| refund | Install a new laser, processing a refund if applicable | verificada | `laser_refund` | make test_eqshp (3.000 EQSHP del original con eq, prx, refund, PRXS, LCASH y MCASH reales) |
| PRXS | Equipment prices | verificada | `equip_price` | make test_eqshp (3.000 EQSHP del original con eq, prx, refund, PRXS, LCASH y MCASH reales) |

## Flight (29 portadas de 29)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| DOENTRY | Dock at the space station and work out any mission progression | verificada | `missions_doentry`, `missions_choose`, `flight_run`, `combat_collide` | la decision de las sesiones informativas (EN1..EN5) verificada con make test_missions; se atraca por la ranura (ISDK), con el ordenador de atraque o, sin el, con C (provisional); energia y escudos se reponen al despegar |
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
| TT110 | Launch from a station or show the front space view | transcripcion | `space_launch` | RES2, planeta delante (z_sign = 1) y estacion detras (z = -256) con SOS1 y NWSPS, DELTA = 12, FIST |= BAD; make test_space |
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
| SIGHT | Draw the laser crosshairs | verificada | `chart_sight` | make test_draw (con sightcol) |

## Keyboard (15 portadas de 19, 4 no necesarias)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| TGINT | The keys used to toggle configuration settings when the game is | verificada | `tgint` | teclas DAXFYJKUT y su orden (DAMP..DISK); make test_dk4 |
| FLKB | Flush the keyboard buffer | no-verificable | `plat_flush_input` | vaciar el buffer de teclado: propio de la consola del QL |
| PAUSE2 | Wait until a key is pressed, ignoring any existing key press | no-verificable | `plat_wait_key` | esperar una tecla: propio de la consola del QL |
| TT214 | Ask a question with a "Y/N?" prompt and return the response | no-verificable | `scr_disk` | pregunta Y/N propia del menu de disco |
| YESNO | Wait until either "Y" or "N" is pressed | no-verificable | `scr_disk` | confirmacion Y/N propia del menu de disco |
| TT17 | Scan the keyboard for cursor key or joystick movement | no-verificable | `in_held` | cursores o joystick: lectura del teclado del QL (keys.c), sin equivalente comparable |
| TT102 | Process number key, save key, hyperspace and chart key presses | no-verificable | `ui_nav` | despacho de teclas a pantallas con la disposicion de teclas del QL |
| ZEKTRAN | Clear the key logger | no-necesaria |  | registrador de teclas del BBC (o sin uso): keys.c lee las filas directamente y no necesita limpiarlo |
| RDS1 | Scan one axis of the joystick and return its position | no-verificable | `ctl_read` | joystick del Apple II (paddles); en el QL son teclas CTL1 |
| RDKEY | Scan the keyboard for key presses and the joystick, and update the | no-verificable | `key_scan` | lectura del teclado del Apple II; el QL usa KEYROW por IPC |
| KYTB | Lookup table for in-flight keyboard controls | no-verificable | `keys.c` | tabla de teclas del Apple II; las del QL estan en keys.h |
| DKSANYKEY | An unused routine that scans the keyboard to detect whether any | no-necesaria |  | registrador de teclas del BBC (o sin uso): keys.c lee las filas directamente y no necesita limpiarlo |
| DKS2 | Read the joystick position | no-verificable | `ctl_read` | joystick del Apple II; en el QL son teclas CTL1 |
| DKS3 | Toggle a configuration setting and emit a beep | verificada | `dks3` | invierte la opcion con EOR si la tecla coincide con TGINT,Y; make test_dk4 (2.000 pausas con teclas al azar: opciones DAMP..DISK, DNOIZ, final por "-" o ESC y teclas leidas; RDKEY, WSCAN, DELAY y BELL son stubs, los pitidos no se cuentan) |
| U% | Clear the key logger | no-necesaria |  | registrador de teclas del BBC (o sin uso): keys.c lee las filas directamente y no necesita limpiarlo |
| DOKEY | Scan for the seven primary flight controls and apply the docking | no-verificable | `ctl_read` | lectura de las teclas de vuelo con la disposicion del QL; el tramo del ordenador de atraque esta verificado en make test_equip |
| DK4 | Scan for pause, configuration and secondary flight keys | verificada | `in_poll`, `freeze` | FREEZE (la pausa con "=") y sus teclas: Q/S sonido, DAXFYJKUT, ESC = DEATH2, "-" sigue; make test_dk4 (2.000 pausas con teclas al azar: opciones DAMP..DISK, DNOIZ, final por "-" o ESC y teclas leidas; RDKEY, WSCAN, DELAY y BELL son stubs, los pitidos no se cuentan). La lectura de "=" y de la tecla es de in_poll (propia) |
| TT217 | Scan the keyboard until a key is pressed | no-verificable | `in_wait` | esperar una tecla: propio de la consola del QL |
| KTRAN | An unused key logger buffer that's left over from the 6502 Second | no-necesaria |  | registrador de teclas del BBC (o sin uso): keys.c lee las filas directamente y no necesita limpiarlo |

## Loader (0 portadas de 5, 5 no necesarias)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| ENTRY | An entry point at the start of the game binary | no-necesaria |  | proteccion anticopia del original: suma de comprobacion, descifrado y desmezcla del codigo; en el QL no existe |
| S% | Checksum, decrypt and unscramble the main game code, and start the | no-necesaria |  | proteccion anticopia del original: suma de comprobacion, descifrado y desmezcla del codigo; en el QL no existe |
| BEGIN | Initialise the configuration variables and start the game | no-necesaria |  | arranque del Apple II (interrupciones, modo de pantalla, memoria); lo hacen main.c y app.c |
| COLD | Initialise the screen mode, clear memory and set up interrupt | no-necesaria |  | arranque del Apple II (interrupciones, modo de pantalla, memoria); lo hacen main.c y app.c |
| NMIpissoff | Acknowledge NMI interrupts and ignore them | no-necesaria |  | arranque del Apple II (interrupciones, modo de pantalla, memoria); lo hacen main.c y app.c |

## Main loop (1 portadas de 1)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| Main | Seed the random number generator | verificada | `ships_step`, `flight_run`, `encounters_tick`, `ships_part14`, `laser_fire`, `combat_hit`, `combat_killed`, `laser_frame_end`, `energy_tick`, `laser_cool`, `combat_collide`, `damage_checks`, `ecm_frame`, `ecm_key`, `missile_unarm`, `missile_target`, `missile_fire`, `missile_lock` | bucle de vuelo (partes 1-16) y bucle principal (partes 1-6): naves, laser, choques, recogida, atraque, partes 14 (estacion) y 15, bomba, ECM, misiles y aparicion de naves; verificados por partes con make test_laser, test_damage, test_equip, test_tactics y test_missions (MCNT baja como en el original) |

## Market (19 portadas de 19)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| tnpr1 | Work out if we have space for one tonne of cargo | verificada | `combat_collide`, `trade_fits` | recogida de cargas (make test_damage) |
| tnpr | Work out if we have space for a specific amount of cargo | verificada | `trade_room` |  |
| TT219 | Show the Buy Cargo screen | verificada | `trade_buy` |  |
| gnum | Get a number from the keyboard | no-verificable | `trade_screen` | entrada de la cantidad propia (cursores, cifras, M = maximo); en el original lee el teclado del Apple II |
| NWDAV4 | Print an "ITEM?" error, make a beep and rejoin the TT210 routine | no-verificable | `ui_msg` | mensaje de error propio con sonido |
| TT208 | Show the Sell Cargo screen | verificada | `trade_sell` |  |
| TT210 | Show a list of current cargo in our hold, optionally to sell | no-verificable | `trade_screen` | lista de venta propia (mismo listado con las existencias) |
| TT213 | Show the Inventory screen | no-verificable | `scr_inventory` | pantalla propia (maquetacion); los datos (QQ20, unidades) se verifican en TT208/TT219 |
| TT151 | Print the name, price and availability of a market item | verificada | `uni_gen_market` | precio de cada articulo (QQ24 * 4 con GC2), make test_univ6502; la pantalla del mercado es propia |
| TT152 | Print the unit ("t", "kg" or "g") for a market item | no-verificable | `unit_char` | unidad t/kg/g: el listado es propio; el dato (bits de unidad de QQ23) esta verificado |
| TT160 | Print "t" (for tonne) and a space | no-verificable | `unit_char` | idem TT152 |
| TT161 | Print "kg" (for kilograms) | no-verificable | `unit_char` | idem TT152 |
| TT16a | Print "g" (for grams) | no-verificable | `unit_char` | idem TT152 |
| TT163 | Print the headers for the table of market prices | no-verificable | `market_header` | cabecera propia del listado |
| TT167 | Show the Market Price screen | no-verificable | `scr_market` | pantalla propia (maquetacion); precios y existencias verificados (TT151, GVL) |
| var | Calculate QQ19+3 = economy * |economic_factor| | verificada | `uni_gen_market` | economia * factor, dentro de GVL y TT151 (make test_univ6502) |
| dn | Print the amount of money we have left in the cash pot, then make | no-verificable | `trade_info` | redibujo del dinero tras cada operacion (pantalla propia); el cobro esta verificado |
| ITEM | Macro definition for the market prices table | verificada | `uni_goods` | macro de la tabla QQ23 (make test_univ6502) |
| QQ23 | Market prices table | verificada | `uni_goods` | tabla de mercancias, comprobada a traves de GVL y TT151 (make test_univ6502) |

## Maths (Arithmetic) (51 portadas de 52, 1 no necesarias)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| log | Binary logarithm table (high byte) | verificada | `em_logL`, `em_alogh` | tablas de logaritmos y antilogaritmos de emath.c, comprobadas con FMLTU y DVID4 |
| logL | Binary logarithm table (low byte) | verificada | `em_logL`, `em_alogh` | tablas de logaritmos y antilogaritmos de emath.c, comprobadas con FMLTU y DVID4 |
| alogh | Binary antilogarithm table | verificada | `em_logL`, `em_alogh` | tablas de logaritmos y antilogaritmos de emath.c, comprobadas con FMLTU y DVID4 |
| PIX1 | Calculate (YY+1 SYL+Y) = (A P) + (S R) and draw stardust particle | propia | `pixel2` | dibujo del polvo estelar (stars.c) |
| MAS3 | Calculate A = x_hi^2 + y_hi^2 + z_hi^2 in the K% block | verificada | `sm_mas3` | make test_geom (codigo original en el emulador) |
| VCSU1 | Calculate vector K3(8 0) = [x y z] - coordinates of the sun or | transcripcion | `sm_vcsu1` |  |
| VCSUB | Calculate vector K3(8 0) = [x y z] - coordinates in (A V) | verificada | `sm_vcsub` | make test_geom (codigo original en el emulador) |
| TAS1 | Calculate K3 = (x_sign x_hi x_lo) - V(1 0) | verificada | `sm_tas1` | make test_geom (codigo original en el emulador) |
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
| LCASH | Subtract an amount of cash from the cash pot | verificada | `sm_lcash` | make test_geom (codigo original en el emulador) |
| MCASH | Add an amount of cash to the cash pot | verificada | `sm_mcash` | make test_geom (codigo original en el emulador) |
| GCASH | Calculate (Y X) = P * Q * 4 | verificada | `sm_gcash` | make test_geom (codigo original en el emulador) |
| GC2 | Calculate (Y X) = (A P) * 4 | propia | `tt22`, `tt23` | la multiplicacion por 4 va en linea en las cartas |
| SPS2 | Calculate (Y X) = A / 10 | verificada | `sm_sps2` | make test_geom (codigo original en el emulador) |
| DORND | Generate random numbers | verificada | `em_dornd` |  |
| TIS2 | Calculate A = A / Q | transcripcion | `em_tis2` |  |
| TIS3 | Calculate -(nosev_1 * roofv_1 + nosev_2 * roofv_2) / nosev_3 | verificada | `sm_tis3` | make test_geom (codigo original en el emulador) |
| DVIDT | Calculate (P+1 A) = (A P) / Q | verificada | `em_dvidt` | a traves de TIS3 en make test_geom; el bit que sale de P+1 al principio se pierde (estaba mal hasta la 16.a ronda) |
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
| MAS1 | Add an orientation vector coordinate to an INWK coordinate | verificada | `sm_mas1` | make test_geom (codigo original en el emulador) |
| MAS2 | Calculate a cap on the maximum distance to the planet or sun | verificada | `sm_mas2` | make test_geom (codigo original en el emulador) |
| TAS4 | Calculate the dot product of XX15 and one of the space station's | verificada | `sm_tas4` | make test_geom (codigo original en el emulador) |
| TAS6 | Negate the vector in XX15 so it points in the opposite direction | verificada | `sm_tas6` | make test_geom (codigo original en el emulador) |
| TAS3 | Calculate the dot product of XX15 and an orientation vector | verificada | `sm_tas3` | make test_geom (codigo original en el emulador) |
| SCALEY | Scale the y-coordinate in A to 0.375 * A | verificada | `sm_scaley` | make test_geom (codigo original en el emulador) |
| SCALEY2 | Scale the y-coordinate in A to 0.75 * A | verificada | `sm_scaley2` | make test_geom (codigo original en el emulador) |
| SCALEX | Scale the x-coordinate in A to 32 + 0.75 * A | verificada | `sm_scalex` | make test_geom (codigo original en el emulador) |
| SPS4 | Calculate the vector to the space station | verificada | `sm_sps4` | make test_geom (codigo original en el emulador) |
| SPS3 | Copy a space coordinate from the K% block into K3 | verificada | `sm_sps3` | make test_geom (codigo original en el emulador) |
| PROJ | Project the current ship or planet onto the screen | verificada | `sm_proj` | make test_geom (codigo original en el emulador) |
| FAROF | Compare x_hi, y_hi and z_hi with 224 | verificada | `farof`, `sm_farof2` | make test_geom |
| FAROF2 | Compare x_hi, y_hi and z_hi with A | verificada | `sm_farof2`, `ships_part14`, `farof` | make test_geom; FAROF2 con 192 en la parte 14 y con 224 (FAROF) al quitar naves |
| MAS4 | Calculate a cap on the maximum distance to a ship | verificada | `sm_mas4` | make test_geom (codigo original en el emulador) |
| SPS1 | Calculate the vector to the planet and store it in XX15 | verificada | `sm_sps1` | make test_geom (codigo original en el emulador) |
| TAS2 | Normalise the three-coordinate vector in K3 | verificada | `sm_tas2` | make test_geom (codigo original en el emulador) |
| NORM | Normalise the three-coordinate vector in XX15 | verificada | `sm_norm` | make test_geom (codigo original en el emulador) |
| TIDY | Orthonormalise the orientation vectors for a ship | verificada | `sm_tidy` | make test_geom (codigo original en el emulador) |
| LL51 | Calculate the dot product of XX15 and XX16 | verificada | `ship_ll51` |  |

## Missions (10 portadas de 16, 6 no necesarias)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| TRIBDIR | The low byte of the four 16-bit directions in which Trumble | no-necesaria |  | tablas de los sprites de los Trumbles (hardware VIC del C64); el contador de Trumbles seguira pendiente con las misiones |
| TRIBDIRH | The high byte of the four 16-bit directions in which Trumble | no-necesaria |  | tablas de los sprites de los Trumbles (hardware VIC del C64); el contador de Trumbles seguira pendiente con las misiones |
| SPMASK | Masks for updating sprite bits in VIC+$10 for the top bit of the | no-necesaria |  | tablas de los sprites de los Trumbles (hardware VIC del C64); el contador de Trumbles seguira pendiente con las misiones |
| BRIEF2 | Start mission 2 | verificada | `brief2` | mision 2: ir a Ceerdi, make test_missions |
| BRP | Print an extended token and show the Status Mode screen | verificada | `brief2`, `brief3`, `debrief`, `debrief2` | ficha y BAY, make test_missions |
| BRIEF3 | Receive the briefing and plans for mission 2 | verificada | `brief3` | los planos en Ceerdi, make test_missions |
| DEBRIEF2 | Finish mission 2 | verificada | `debrief2` | planos entregados: ENGY = 2, TALLY+1, make test_missions |
| DEBRIEF | Finish mission 1 | verificada | `debrief` | Constrictor destruido: 5.000 CR, make test_missions |
| TBRIEF | Start mission 3 | no-necesaria |  | comentada en el original (Trumbles, que esta version no tiene) |
| BRIEF | Start mission 1 and show the mission briefing | verificada | `brief` | mision 1: el Constrictor girando (LL9/MVEIT) y la ficha 10, make test_missions |
| BRIS | Clear the screen, display "INCOMING MESSAGE" and wait for 2 | verificada | `bris` | INCOMING MESSAGE y DELAY 100 (ficha de salto 25), make test_missions |
| PAUSE | Wait until a key is pressed for the Constrictor mission briefing | verificada | `mis_pause` | PAUSE2 y PAS1 (ficha de salto 22), make test_missions |
| PAS1 | Change to the text view for the Constrictor mission briefing | verificada | `pas1`, `mis_tt66` | TT66 con la vista 1, make test_missions |
| THERE | Check whether we are in the Constrictor's system in mission 1 | verificada | `there` | sistema del Constrictor, make test_missions |
| TRIBTA | A table for converting the number of Trumbles in the hold into a | no-necesaria |  | tablas de los sprites de los Trumbles (hardware VIC del C64); el contador de Trumbles seguira pendiente con las misiones |
| TRIBMA | A table for converting the number of Trumbles in the hold into a | no-necesaria |  | tablas de los sprites de los Trumbles (hardware VIC del C64); el contador de Trumbles seguira pendiente con las misiones |

## Moving (8 portadas de 8)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| MVT3 | Calculate K(3 2 1) = (x_sign x_hi x_lo) + K(3 2 1) | verificada | `sm_mvt3` | make test_geom (codigo original en el emulador) |
| MVS5 | Apply a 3.6 degree pitch or roll to an orientation vector | verificada | `mvs5` | make test_move (3.000 casos con Maths, TIDY y FMLTU/MLTU2 reales; SCAN y TACTICS registran) |
| SFS2 | Move a ship in space along one of the coordinate axes | verificada | `sfs2` | dentro de SFS1 (make test_laser, nave madre = estacion) |
| MVEIT | Move current ship: Tidy the orientation vectors | verificada | `mveit` | make test_move (3.000 casos con Maths, TIDY y FMLTU/MLTU2 reales; SCAN y TACTICS registran); encontro que el acarreo de MLTU2 no era el bit bajo del producto: es el ultimo bit que sale de ROR P (siempre 0), ahora MLTU2 esta transcrita literalmente en move.c; TACTICS/SCAN son ganchos (move_tactics_hook, space_scan_hook) |
| MVT1 | Calculate (x_sign x_hi x_lo) = (x_sign x_hi x_lo) + (A R) | verificada | `mvt1` | make test_move (3.000 casos con Maths, TIDY y FMLTU/MLTU2 reales; SCAN y TACTICS registran) |
| MVS4 | Apply pitch and roll to an orientation vector | verificada | `mvs4` | make test_move (3.000 casos con Maths, TIDY y FMLTU/MLTU2 reales; SCAN y TACTICS registran) |
| MVT6 | Calculate (A P+2 P+1) = (x_sign x_hi x_lo) + (A P+2 P+1) | verificada | `mvt6` | make test_move (3.000 casos con Maths, TIDY y FMLTU/MLTU2 reales; SCAN y TACTICS registran) |
| MV40 | Rotate the planet or sun's location in space by the amount of | verificada | `mv40`, `mv40_pre`, `mv45` | make test_move (3.000 casos con Maths, TIDY y FMLTU/MLTU2 reales; SCAN y TACTICS registran) (MV40 hasta MV45 y MV45 por separado); K2 byte 0 vale 0 como en el original (nadie lo escribe) |

## Save and load (27 portadas de 61, 34 no necesarias)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| wtable | 6-bit to 7-bit nibble conversion table | no-necesaria |  | RWTS y catalogo del DOS 3.3 del Apple II; QDOS se ocupa del disco desde C (save.c) |
| BRKBK | Set the standard BRKV handler for the game | no-necesaria |  | manejador de errores BRK del 6502; el runtime de C se ocupa de los errores |
| NA% | The data block for the last saved commander | no-verificable | `cmdr_default` | area de datos del ultimo comandante guardado: en el port son las variables del comandante y el fichero de save.c |
| CHK2 | Second checksum byte for the saved commander data file | verificada | `cmdr_check`, `cmdr_check2` | bytes de comprobacion (verificadas con el 6502) |
| CHK3 | Third checksum byte for the saved commander data file | verificada | `cmdr_check`, `cmdr_check2` | bytes de comprobacion (verificadas con el 6502) |
| CHK | First checksum byte for the saved commander data file | verificada | `cmdr_check`, `cmdr_check2` | bytes de comprobacion (verificadas con el 6502) |
| S1% | The drive and directory number used when saving or loading a | no-verificable | `save_dev` | unidad y directorio del DOS 3.3 del Apple II; el port usa la unidad de QDOS |
| NA2% | The data block for the default commander | verificada | `cmdr_default` | make test_jameson |
| CHECK | Calculate the checksum for the last saved commander data block | verificada | `cmdr_check` | make test_start (DFAULT con CHECK real: la suma del port coincide en 1.000 bloques) y make test_jameson |
| CHECK2 | Calculate the third checksum for the last saved commander data | verificada | `cmdr_check2` | make test_start (DFAULT con CHECK2 real) y make test_jameson |
| JAMESON | Restore the default JAMESON commander | verificada | `cmdr_default` | make test_jameson: los 76 bytes de NA2% (Q% = FALSE) ensamblados de la fuente son los de cmdr_default (COK con el bit 3 que pone DFAULT; CHK2/CHK3/CHK de fabrica = sumas del bloque) |
| COPYNAME | Copy the last saved commander's name from INWK+5 to comnam and pad | no-verificable | `cmdr_set_name`, `save_filename` | nombre del fichero del DOS: el port filtra y copia el nombre a su manera (cmdr_set_name) |
| TRNME | Copy the last saved commander's name from INWK to NA% | no-verificable | `cmdr_set_name`, `save_filename` | idem COPYNAME |
| TR1 | Copy the last saved commander's name from NA% to INWK | no-verificable | `cmdr_set_name`, `save_filename` | idem COPYNAME |
| GTNMEW | Fetch the name of a commander file to save or load | no-verificable | `edit_name` | entrada del nombre propia del port (edit_name) |
| FILEPR | Display the currently selected media (disk or tape) | no-verificable | `scr_disk` | menu de disco propio |
| OTHERFILEPR | Display the non-selected media (disk or tape) | no-verificable | `scr_disk` | menu de disco propio |
| SVE | Display the disk access menu and process saving of commander files | no-verificable | `scr_disk` | menu de disco propio; el cifrado MUTILATE y el fichero del DOS 3.3 no se portan (formato propio ELQD) |
| diskerror | Print a disk error, make a beep and wait for a key press | no-verificable | `scr_disk` | errores de disco de QDOS en el menu propio |
| thislong | Contains the length of the most recently entered commander name | no-necesaria |  | longitud de los nombres: en C se calcula con strlen |
| oldlong | Contains the length of the last saved commander name | no-necesaria |  | longitud de los nombres: en C se calcula con strlen |
| LOD | Load a commander file | no-verificable | `load_commander` | carga de fichero de QDOS (formato propio ELQD, make test_save); validacion CHK/CHK3/CHK2 verificada con make test_start |
| DERR1 | The error message for when a disk is write protected | no-verificable | `save_errstr` | mensajes de error de RWTS del DOS 3.3; QDOS tiene los suyos |
| DERR2 | The error message for when a disk is full | no-verificable | `save_errstr` | idem DERR1 |
| DERR3 | The error message for when a disk catalog is full | no-verificable | `save_errstr` | idem DERR1 |
| DERR4 | The error message for when there is a disk I/O error | no-verificable | `save_errstr` | idem DERR1 |
| DERR5 | The error message for when a file is not found | no-verificable | `save_errstr` | idem DERR1 |
| ERTAB | A lookup table for the five disk error messages | no-verificable | `save_errstr` | idem DERR1 |
| comnam | Storage for the commander filename, padded out with spaces to a | no-verificable | `cmdr_set_name`, `save_filename` | nombre del fichero del DOS 3.3 |
| rfile | Read a commander file from a DOS disk into the buffer | no-verificable | `load_commander` | lectura de fichero del DOS 3.3 (RWTS); el port usa fopen de QDOS |
| wfile | Write a commander file from the buffer to a DOS disk | no-verificable | `save_commander`, `write_file` | escritura de fichero del DOS 3.3 (RWTS); el port usa fopen de QDOS |
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
| BELL | Make a standard system beep | no-verificable | `snd_bell` | CHPR(7): el pitido de la consola del Apple II; el sonido sale por el BEEP del QL (propio) |
| EXNO | Make the sound of a laser strike on another ship | verificada | `snd_exno` | make test_soundorig (3.000 casos, DNOIZ/RAND/acarreo al azar): SOEXPL con Y = 15 |
| BOOP | Make a long, low beep | verificada | `snd_boop` | make test_soundorig (3.000 casos, DNOIZ/RAND/acarreo al azar): no gasta DORND; el sonido es propio |
| SOHISS | Make the sound of a launch from the station, hyperspace or missile | verificada | `snd_sohiss` | make test_soundorig (3.000 casos, DNOIZ/RAND/acarreo al azar): Y DORND (256 si Y = 0) solo con el sonido activo, con la cadena de acarreos de DORND; el sonido es propio |
| EXNO3 | Make the sound of a collision, or an exploding cargo canister or | verificada | `snd_exno3` | make test_soundorig (3.000 casos, DNOIZ/RAND/acarreo al azar): SOEXPL con Y = 40 |
| SOEXPL | Make an explosion sound | verificada | `snd_soexpl` | make test_soundorig (3.000 casos, DNOIZ/RAND/acarreo al azar): Y DORND (256 si Y = 0) solo con el sonido activo; el sonido es propio |
| BEEP | Make a short, high beep | verificada | `snd_beep` | make test_soundorig (3.000 casos, DNOIZ/RAND/acarreo al azar): no gasta DORND ni toca RAND; el sonido es propio |
| SOBEEP | Make a beep | verificada | `snd_sobeep` | make test_soundorig (3.000 casos, DNOIZ/RAND/acarreo al azar): no gasta DORND; el tono es la traduccion de los ciclos del bucle (propia) |
| SOBLIP | Make the sound of the hyperspace drive being engaged, or the sound | verificada | `snd_soblip` | make test_soundorig (3.000 casos, DNOIZ/RAND/acarreo al azar): no gasta DORND; el barrido es la traduccion de los ciclos (propia) |
| LASNOISE | Make the sound of our laser firing or the sound of us being hit by | verificada | `snd_lasnoise` | make test_soundorig (3.000 casos, DNOIZ/RAND/acarreo al azar): no gasta DORND; el barrido es propio |
| LASNOISE2 | An unused routine that makes a different laser sound | no-necesaria |  | sin uso en el original |
| SOBOMB | Make the sound of an energy bomb going off | verificada | `snd_sobomb` | make test_draw (los 25 DORND con el sonido activo, a traves de BOMBEFF2); el sonido es propio; tambien make test_soundorig |
| CLICK | Toggle the state of the speaker (i.e. move it in or out) to make a | no-necesaria |  | conmutacion directa del altavoz del Apple II; el sonido sale por el IPC del QL (sound.c) |

## Stardust (7 portadas de 7)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| FLIP | Reflect the stardust particles in the screen diagonal and redraw | verificada | `stars_flip` | make test_stars |
| STARS | The main routine for processing the stardust | verificada | `stars_update` | make test_stars (las 4 vistas) |
| STARS1 | Process the stardust for the front view | verificada | `stars1` | make test_stars (con los acarreos de KILL1: C = 1 tras CMP |
| STARS6 | Process the stardust for the rear view | verificada | `stars6` | make test_stars |
| STARS2 | Process the stardust for the left or right view | verificada | `stars2` | make test_stars (con ST2; KILL2 con C = 0 o 1 segun BCC/BEQ) |
| NWSTARS | Initialise the stardust field | verificada | `stars_new` | a traves de nWq (make test_stars); el original no crea el polvo si QQ11 != 0 (va a WPSHPS): aqui solo se llama en la vista espacial |
| nWq | Create a random cloud of stardust | verificada | `stars_new` | make test_stars (el primer DORND con el acarreo de entrada; cae en WPSHPS, que el port hace aparte) |

## Start and end (8 portadas de 8)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| RESET | Reset most variables | verificada | `space_reset_all` | make test_start (1.000 casos con ZERO, RES2 y ZINF reales: giros, QQ22, ECMA, ALP1/2, JSTGY, QQ12 y FSH/ASH/ENERGY a $FF) |
| RES2 | Reset a number of flight variables and workspaces | verificada | `space_reset`, `ships_zero`, `ship_zinf` | make test_start (1.000 casos: NOSTM, LSX2/LSY2 = $FF, MSTG, giros, DELTA/ALPHA/ALP1 = 3, SPBLB si SSPR, ECMOF si ECMA, ZERO, SLSP y ZINF; WPSHPS lo hace quien llama (clear_view); JSTX no se toca; ships_clear vacia ademas K% y los montones) |
| DEATH | Display the death screen | no-verificable | `flight_death`, `ships_death_debris` | D1 (Ze, fq1, restos) verificado en make test_damage; el resto es orquestacion: SOEXPL con Y = 210, RES2, DELTA * 4, TT66, nWq, GAME OVER con ex(146) en (12, 12), M% una vez y D2 64 veces (el bucle principal completo y el sonido son propios del port) |
| TT170 | Main entry point for the Elite game code | no-verificable | `game_run` | punto de entrada: RESET (verificado) y el flujo hacia el titulo; es el bucle principal propio del port (app.c) |
| DEATH2 | Reset most of the game and restart from the title screen | no-verificable | `game_run` | vuelta al titulo (RES2 verificado); el flujo del titulo es propio del port |
| BR1 | Show the "Load New Commander (Y/N)?" screen and start the game | no-verificable | `scr_title`, `scr_disk` | flujo de la pantalla de titulo y la carga de comandante propios del port (DFAULT y RESET verificados aparte) |
| DFAULT | Reset the current commander data block to the last saved commander | verificada | `cmdr_unpack`, `cmdr_pack` | make test_start (1.000 bloques con CHECK/CHECK2 reales: COK con bit 7 si falla CHK2 y bit 3; el original se queda en un bucle si CHK o CHK3 fallan, el port devuelve error; CHK2/CHK3/CHK van fuera del bloque copiado en el original y en b[74..76] en el port; QQ11 = 0 lo hace quien llama) |
| TITLE | Display a title screen with a rotating ship and prompt | no-verificable | `scr_title` | pantalla de titulo propia del port (nave que gira, menu de disco); depende del modo grafico del Apple II |

## Status (7 portadas de 7)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| STATUS | Show the Status Mode screen | verificada | `status_mode`, `scr_status`, `rating_name`, `legal_name` | transcripcion literal con el motor de fichas (titulo, estado, rango, equipo y lasers, letra a letra con columna y fila, TRADEMODE y TT111 registran); 800 casos, make test_missions |
| cmn | Print the commander's name | verificada | `cmn` | nombre del comandante con TT26 (ficha 4 de TT27 y titulo de STATUS), make test_missions |
| fwl | Print fuel and cash levels | verificada | `dt_fwl` | combustible y dinero, make test_missions |
| csh | Print the current amount of cash | verificada | `dt_csh` | dinero con BPRNT (9 cifras y punto), make test_missions |
| BAD | Calculate how bad we have been | verificada | `space_bad`, `bad`, `legal_name` | carga ilegal (encounters.c verificado con make test_missions; space_bad para TT110); estado legal con legal_name |
| BAY | Go to the docking bay (i.e. show the Status Mode screen) | verificada | `bay` | QQ12 = $FF y FRCE con f8 (pantalla de estado), make test_missions; brp y DOENTRY acaban en bay |
| EXNO2 | Process us making a kill | verificada | `exno2` | TALLY con KWL%/KWH% y mensaje 101; el sonido es snd_exno2 |

## Tactics (7 portadas de 8, 1 no necesarias)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| TACTICS | Apply tactics: Process missiles, both enemy missiles and our own | verificada | `tactics` | las 7 partes (la 7 en dockit.c); 20.000 casos con nave hija, misiles, laser, ECM, capsula |
| HITCH | Work out if the ship in INWK is in our crosshairs | verificada | `hitch` | incluido el borde del area de impacto |
| FRS1 | Launch a ship straight ahead of us, below the laser sights | verificada | `frs1` | make test_equip |
| FRMIS | Fire a missile from our ship | verificada | `frmis`, `missile_fire` | FRS1, GINF, ANGRY y ABORT reales: eventos de NWSHP, MESS, MSBAR (indicador y color) y SOHISS, NOMSL y el blanco; 1.500 casos (MSBAR con color BLACK deja A = 0 y FRMIS cae en ANGRY con ella), make test_equip; el argumento 120 de SOHISS no se captura desde el C |
| ANGRY | Make a ship or station hostile, and if this is a ship then enable | verificada | `angry` | escribe en el bloque K% (INF); la estacion es la ranura 1 |
| FR1 | Display the "missile jammed" message | verificada | `fr1` | MISSILE JAMMED (ficha 201 de MESS) cuando FRS1 no cabe, dentro de FRMIS en make test_equip |
| yetanotherrts | Contains an RTS | no-necesaria |  | contiene solo un RTS |
| SFRMIS | Add an enemy missile to our local bubble of universe | verificada | `sfrmis` | a traves de TACTICS |

## Text (73 portadas de 77, 4 no necesarias)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| MT27 | Print the captain's name during mission briefings | verificada | `jump` | ficha de salto (detok.c), make test_missions |
| MT28 | Print the location hint during the mission 1 briefing | verificada | `jump` | ficha de salto (detok.c), make test_missions |
| DETOK3 | Print an extended recursive token from the RUTOK token table | verificada | `detok3` | motor de fichas extendidas de TKN1/RUTOK (detok.c), make test_missions: todas las fichas |
| DETOK | Print an extended recursive token from the TKN1 token table | verificada | `detok` | motor de fichas extendidas de TKN1/RUTOK (detok.c), make test_missions: todas las fichas |
| DETOK2 | Print an extended text token (1-255) | verificada | `detok2` | motor de fichas extendidas de TKN1/RUTOK (detok.c), make test_missions: todas las fichas |
| MT1 | Switch to ALL CAPS when printing extended tokens | verificada | `jump` | ficha de salto (detok.c), make test_missions |
| MT2 | Switch to Sentence Case when printing extended tokens | verificada | `dt_mt2` | ficha de salto (detok.c), make test_missions |
| MT8 | Tab to column 6 and start a new word when printing extended tokens | verificada | `jump` | ficha de salto (detok.c), make test_missions |
| MT9 | Clear the screen and set the current view type to 1 | verificada | `jump` | ficha de salto (detok.c), make test_missions |
| MT13 | Switch to lower case when printing extended tokens | verificada | `jump` | ficha de salto (detok.c), make test_missions |
| MT6 | Switch to standard tokens in Sentence Case | verificada | `jump` | ficha de salto (detok.c), make test_missions |
| MT5 | Switch to extended tokens | verificada | `jump` | ficha de salto (detok.c), make test_missions |
| MT14 | Switch to justified text when printing extended tokens | verificada | `dt_mt14` | ficha de salto (detok.c), make test_missions |
| MT15 | Switch to left-aligned text when printing extended tokens | verificada | `dt_mt15` | ficha de salto (detok.c), make test_missions |
| MT17 | Print the selected system's adjective, e.g. Lavian for Lave | verificada | `jump` | adjetivo del sistema; reproduce el fallo del original (LDA BUF-1,X en pagina cero: VOWEL mira la pagina cero), make test_missions |
| MT18 | Print a random 1-8 letter word in Sentence Case | verificada | `jump` | 1 a 4 fichas de dos letras al azar con los acarreos de DORND del original, make test_missions |
| MT19 | Capitalise the next letter | verificada | `jump` | ficha de salto (detok.c), make test_missions |
| VOWEL | Test whether a character is a vowel | verificada | `vowel` | dentro de MT17 (detok.c) |
| WHITETEXT | Switch to white text | no-necesaria |  | RTS compartido con VOWEL (VRTS); en C no hace falta |
| JMTB | The extended token table for jump tokens 1-32 (DETOK) | verificada | `tokens_data.h`, `detok.c` | tablas y variables del motor de fichas (tools/gen_tokens.py, iguales a los binarios de referencia), make test_missions |
| TKN2 | The extended two-letter token lookup table | verificada | `tokens_data.h`, `detok.c` | tablas y variables del motor de fichas (tools/gen_tokens.py, iguales a los binarios de referencia), make test_missions |
| QQ16 | The two-letter token lookup table | verificada | `tokens_data.h`, `detok.c` | tablas y variables del motor de fichas (tools/gen_tokens.py, iguales a los binarios de referencia), make test_missions |
| plf2 | Print text followed by a newline and indent of 6 characters | verificada | `dt_plf2` | detok.c, make test_missions |
| TENS | A constant used when printing large numbers in BPRNT | verificada | `dt_bprnt` | impresion de numeros del original (detok.c), make test_missions (BPRNT con K, U y C al azar; pr2, pr5 y pr6 dentro de TT25, csh, fwl y tal); las pantallas propias siguen con ui_num |
| pr2 | Print an 8-bit number, left-padded to 3 digits, and optional point | verificada | `dt_pr2` | impresion de numeros del original (detok.c), make test_missions (BPRNT con K, U y C al azar; pr2, pr5 y pr6 dentro de TT25, csh, fwl y tal); las pantallas propias siguen con ui_num |
| TT11 | Print a 16-bit number, left-padded to n digits, and optional point | verificada | `dt_tt11` | impresion de numeros del original (detok.c), make test_missions (BPRNT con K, U y C al azar; pr2, pr5 y pr6 dentro de TT25, csh, fwl y tal); las pantallas propias siguen con ui_num |
| BPRNT | Print a 32-bit number, left-padded to a specific number of digits, | verificada | `dt_bprnt` | impresion de numeros del original (detok.c), make test_missions (BPRNT con K, U y C al azar; pr2, pr5 y pr6 dentro de TT25, csh, fwl y tal); las pantallas propias siguen con ui_num |
| DTW1 | A mask for applying the lower case part of Sentence Case to | verificada | `tokens_data.h`, `detok.c` | tablas y variables del motor de fichas (tools/gen_tokens.py, iguales a los binarios de referencia), make test_missions |
| DTW2 | A flag that indicates whether we are currently printing a word | verificada | `tokens_data.h`, `detok.c` | tablas y variables del motor de fichas (tools/gen_tokens.py, iguales a los binarios de referencia), make test_missions |
| DTW3 | A flag for switching between standard and extended text tokens | verificada | `tokens_data.h`, `detok.c` | tablas y variables del motor de fichas (tools/gen_tokens.py, iguales a los binarios de referencia), make test_missions |
| DTW4 | Flags that govern how justified extended text tokens are printed | verificada | `tokens_data.h`, `detok.c` | tablas y variables del motor de fichas (tools/gen_tokens.py, iguales a los binarios de referencia), make test_missions |
| DTW5 | The size of the justified text buffer at BUF | verificada | `tokens_data.h`, `detok.c` | tablas y variables del motor de fichas (tools/gen_tokens.py, iguales a los binarios de referencia), make test_missions |
| DTW6 | A flag to denote whether printing in lower case is enabled for | verificada | `tokens_data.h`, `detok.c` | tablas y variables del motor de fichas (tools/gen_tokens.py, iguales a los binarios de referencia), make test_missions |
| DTW8 | A mask for capitalising the next letter in an extended text token | verificada | `tokens_data.h`, `detok.c` | tablas y variables del motor de fichas (tools/gen_tokens.py, iguales a los binarios de referencia), make test_missions |
| FEED | Print a newline | verificada | `dt_feed` | detok.c, make test_missions |
| MT16 | Print the character in variable DTW7 | verificada | `jump` | ficha de salto (detok.c), make test_missions |
| TT26 | Print a character at the text cursor, with support for verified | verificada | `dt_dasc` | DASC con la justificacion de LL = 30 (detok.c); las pantallas propias siguen con txt_putc, make test_missions |
| MT23 | Move to row 9 and switch to lower case | verificada | `jump` | ficha de salto (detok.c), make test_missions |
| MT29 | Move to row 5 and switch to lower case when printing extended | verificada | `jump` | ficha de salto (detok.c), make test_missions |
| MTIN | Lookup table for random tokens in the extended token table (0-37) | verificada | `tokens_data.h`, `detok.c` | tablas y variables del motor de fichas (tools/gen_tokens.py, iguales a los binarios de referencia), make test_missions |
| DOXC | Move the text cursor to a specific column | verificada | `dt_xc`, `dt_yc` | cursor del motor de fichas (asignacion directa de XC/YC), dentro de las rutinas verificadas con make test_missions |
| DOYC | Move the text cursor to a specific row | verificada | `dt_xc`, `dt_yc` | cursor del motor de fichas (asignacion directa de XC/YC), dentro de las rutinas verificadas con make test_missions |
| INCYC | Move the text cursor to the next row | verificada | `dt_xc`, `dt_yc` | cursor del motor de fichas (asignacion directa de XC/YC), dentro de las rutinas verificadas con make test_missions |
| TT60 | Print a text token and a paragraph break | verificada | `dt_tt60` | detok.c, make test_missions |
| TTX69 | Print a paragraph break | verificada | `dt_ttx69` | detok.c, make test_missions |
| TT69 | Set Sentence Case and print a newline | verificada | `dt_tt69` | detok.c, make test_missions |
| TT67 | Print a newline | verificada | `dt_tt67` | detok.c, make test_missions |
| spc | Print a text token followed by a space | verificada | `dt_spc` | detok.c, make test_missions |
| pr6 | Print 16-bit number, left-padded to 5 digits, no point | verificada | `dt_pr6` | impresion de numeros del original (detok.c), make test_missions (BPRNT con K, U y C al azar; pr2, pr5 y pr6 dentro de TT25, csh, fwl y tal); las pantallas propias siguen con ui_num |
| pr5 | Print a 16-bit number, left-padded to 5 digits, and optional point | verificada | `dt_pr5` | impresion de numeros del original (detok.c), make test_missions (BPRNT con K, U y C al azar; pr2, pr5 y pr6 dentro de TT25, csh, fwl y tal); las pantallas propias siguen con ui_num |
| prq | Print a text token followed by a question mark | verificada | `dt_prq` | detok.c, make test_missions |
| TT162 | Print a space | verificada | `dt_tt162` | detok.c, make test_missions |
| dn2 | Make a low, long beep and delay for 0.5 seconds | no-verificable | `snd_boop` | pitido largo y espera (teclado/altavoz del Apple II) |
| plf | Print a text token followed by a newline | verificada | `dt_plf` | detok.c, make test_missions |
| TT68 | Print a text token followed by a colon | verificada | `dt_tt68` | detok.c, make test_missions |
| TT73 | Print a colon | verificada | `dt_tt27` | fichas normales de QQ18 (detok.c); csh, tal y fwl no se usan en las fichas y no estan, make test_missions |
| TT27 | Print a text token | verificada | `dt_tt27` | fichas normales de QQ18 (detok.c); csh, tal y fwl no se usan en las fichas y no estan, make test_missions |
| TT42 | Print a letter in lower case | verificada | `dt_tt27` | fichas normales de QQ18 (detok.c); csh, tal y fwl no se usan en las fichas y no estan, make test_missions |
| TT41 | Print a letter according to Sentence Case | verificada | `dt_tt27` | fichas normales de QQ18 (detok.c); csh, tal y fwl no se usan en las fichas y no estan, make test_missions |
| qw | Print a recursive token in the range 128-145 | verificada | `dt_tt27` | fichas normales de QQ18 (detok.c); csh, tal y fwl no se usan en las fichas y no estan, make test_missions |
| crlf | Tab to column 21 and print a colon | verificada | `dt_tt27` | fichas normales de QQ18 (detok.c); csh, tal y fwl no se usan en las fichas y no estan, make test_missions |
| TT45 | Print a letter in lower case | verificada | `dt_tt27` | fichas normales de QQ18 (detok.c); csh, tal y fwl no se usan en las fichas y no estan, make test_missions |
| TT46 | Print a character and switch to capitals | verificada | `dt_tt27` | fichas normales de QQ18 (detok.c); csh, tal y fwl no se usan en las fichas y no estan, make test_missions |
| TT74 | Print a character | verificada | `dt_tt27` | fichas normales de QQ18 (detok.c); csh, tal y fwl no se usan en las fichas y no estan, make test_missions |
| TT43 | Print a two-letter token or recursive token 0-95 | verificada | `dt_tt27` | fichas normales de QQ18 (detok.c); csh, tal y fwl no se usan en las fichas y no estan, make test_missions |
| ex | Print a recursive token | verificada | `dt_tt27` | fichas normales de QQ18 (detok.c); csh, tal y fwl no se usan en las fichas y no estan, make test_missions |
| MT26 | Fetch a line of text from the keyboard | no-verificable | `edit_text` | lectura de una linea del teclado (teclado/altavoz del Apple II) |
| RLINE | The OSWORD configuration block used to fetch a line of text from | no-verificable | `edit_text` | lectura de una linea del teclado (teclado/altavoz del Apple II) |
| CHPR2 | Character print vector handler | no-necesaria |  | impresion en el modo de texto del Apple II; el QL dibuja siempre en modo grafico |
| R5 | Make a beep and jump back into the character-printing routine at | no-necesaria |  | impresion en el modo de texto del Apple II; el QL dibuja siempre en modo grafico |
| RR5 | Print a character in the text screen mode | no-necesaria |  | impresion en el modo de texto del Apple II; el QL dibuja siempre en modo grafico |
| TT67K | Print a newline | verificada | `dt_chpr` | detok.c, make test_missions |
| CHPR | Print a character at the text cursor by poking into screen memory | verificada | `dt_chpr` | salida del motor de fichas: XC/YC, saltos de linea, clss (detok.c); las pantallas propias siguen con txt_putc, make test_missions |
| letter | Draw a character in the high-resolution screen mode using the game | verificada | `txt_putc`, `draw_glyph` | la fuente es la FONT del original (A.FLOWY.bin, tools/gen_font.py), comparada con make test_univ6502; el dibujo es el de la pantalla del QL |
| letter2 | Draw a character or indicator bulb bitmap in the high-resolution | verificada | `txt_putc`, `draw_glyph`, `dash` | letras con la FONT del original (make test_univ6502); las bombillas del panel salen del volcado de la pantalla original |
| SETXC | Move the text cursor to a specific column | verificada | `dt_xc`, `dt_yc` | cursor del motor de fichas (asignacion directa de XC/YC), dentro de las rutinas verificadas con make test_missions |
| SETYC | Move the text cursor to a specific row | verificada | `dt_xc`, `dt_yc` | cursor del motor de fichas (asignacion directa de XC/YC), dentro de las rutinas verificadas con make test_missions |

## Tube (0 portadas de 1, 1 no necesarias)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| newosrdch | The custom OSRDCH routine for reading characters | no-necesaria |  | rutina OSRDCH personalizada del BBC con procesador Tube; no aplica |

## Universe (32 portadas de 34, 2 no necesarias)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| SPIN | Randomly spawn cargo from a destroyed ship | verificada | `spin`, `spin2` | la cantidad es tipo AND byte 0 AND 15 (el TYA tapa el numero aleatorio) |
| UNIV | Table of pointers to the local universe's ship data blocks | verificada | `state_slot` | K% + n * NI%, comprobado en las secuencias de NWSHP/KILLSHP (make test_univ6502) |
| SFS1 | Spawn a child ship from the current (parent) ship | verificada | `sfs1` | nave hija desde K%; velocidad de los restos = X de DORND AND 15; acarreos modelados |
| PDESC | Print the system's extended description or a mission 1 directive | verificada | `pdesc` | descripciones especiales y la normal con la ficha 5 del original (RAND = QQ15+2..5); NRU% = 26; make test_missions |
| GINF | Fetch the address of a ship's data block into INF | no-necesaria |  | el puntero INF no existe en C: se indexa por ranura |
| ping | Set the selected system to the current system | verificada | `chart_ping_set`, `chart_ping` | QQ9/QQ10 = QQ0/QQ1, 250 casos con make test_univ6502 |
| TT20 | Twist the selected system's seeds four times | verificada | `uni_make_system`, `tweakseed` | las 8 galaxias x 256 sistemas (semillas, economia, gobierno, nivel, poblacion, productividad, radio y nombre) con make test_univ6502 |
| TT54 | Twist the selected system's seeds | verificada | `uni_make_system`, `tweakseed` | las 8 galaxias x 256 sistemas (semillas, economia, gobierno, nivel, poblacion, productividad, radio y nombre) con make test_univ6502 |
| TT146 | Print the distance to the selected system in light years | verificada | `tt25_raw` | distancia con pr5 y LIGHT YEARS, make test_missions |
| TT70 | Display "MAINLY " and jump to TT72 | verificada | `tt25_raw` | MAINLY, make test_missions |
| TT25 | Show the Data on System screen | verificada | `tt25`, `tt25_raw`, `scr_data` | pantalla de datos del sistema del original con el motor de fichas (missions.c), make test_missions (letra a letra, con TT24 y PDESC reales) |
| TT24 | Calculate system data from the system seeds | verificada | `uni_make_system` | las 8 galaxias x 256 sistemas (semillas, economia, gobierno, nivel, poblacion, productividad, radio y nombre) con make test_univ6502 |
| TT81 | Set the selected system's seeds to those of system 0 | verificada | `uni_build_galaxy` | las 8 galaxias x 256 sistemas (semillas, economia, gobierno, nivel, poblacion, productividad, radio y nombre) con make test_univ6502 |
| TT111 | Set the current system to the nearest system to a point | verificada | `uni_nearest`, `uni_distance`, `game_tt111` | sistema mas cercano y distancia QQ8 para 2.000 puntos de las 8 galaxias, make test_univ6502; TT111 desde las coordenadas del elegido (re-seleccion: hyp, TT110, STATUS, f6, HME5, QU5) con game_tt111, comparado en el caso N (gemelos de menor indice) |
| jmp | Set the current system to the selected system | verificada | `game_enter_system`, `jump_do` | QQ0/QQ1 = QQ9/QQ10, make test_univ6502 |
| hyp1 | Process a jump to the system closest to (QQ9, QQ10) | verificada | `jump_do`, `game_enter_system` | la entrada que usa el juego (hyp1+3, desde TT18): jmp, safehouse en QQ2, EV = 0, QQ28/tek/gov; 1.000 casos con make test_univ6502. El JSR TT111 de hyp1 no lo usa ningun camino del juego; la re-seleccion por TT111 (gemelos) esta en game_tt111 |
| GVL | Calculate the availability of market items | verificada | `uni_gen_market`, `game_enter_system` | QQ26 con DORND y disponibilidad AVL, make test_univ6502 |
| GTHG | Spawn a Thargoid ship and a Thargon companion | verificada | `gthg` | make test_equip |
| cpl | Print the selected system name | verificada | `uni_make_system`, `name` | las 8 galaxias x 256 sistemas (semillas, economia, gobierno, nivel, poblacion, productividad, radio y nombre) con make test_univ6502 |
| ypl | Print the current system name | verificada | `name` | nombre del sistema actual (cpl sobre QQ2), make test_univ6502 y make test_missions |
| tal | Print the current galaxy number | verificada | `dt_tt27` | numero de galaxia con pr2, make test_missions |
| SOS1 | Update the missile indicators, set up the planet data block | verificada | `sos1` | planeta 128/130 con giros 127; make test_missions |
| SOLAR | Set up various aspects of arriving in a new system | verificada | `ships_arrive` | FIST a la mitad (su acarreo entra en el ADC |
| NWSPS | Add a new space station to our local bubble of universe | verificada | `nwsps` | estacion en la ranura 1 (STX FRIN+1); Dodo con nivel tecnologico >= 10; make test_missions |
| NWSHP | Add a new ship to our local bubble of universe | verificada | `ship_new` | con el monton de lineas del original (SLSP, bytes 33 y 34): falla cuando el original falla; secuencias de NWSHP y KILLSHP con SHIPS.bin, make test_univ6502 |
| NwS1 | Flip the sign and double an INWK byte | verificada | `nwsps` | dentro de NWSPS (make test_missions) |
| KS3 | Set the SLSP ship line heap pointer after shuffling ship slots | no-necesaria |  | puntero SLSP del montón de lineas: ships.c usa un montón fijo por ranura |
| KS1 | Remove the current ship from our local bubble of universe | verificada | `ship_kill` | quita la nave actual (KILLSHP verificado con make test_univ6502) |
| KS4 | Remove the space station and replace it with the sun | verificada | `ks4` | la estacion lejana (FAROF) deja en su ranura el sol con y_sign = 6; make test_missions |
| KS2 | Check the local bubble for missiles with target lock | verificada | `ks2` | make test_tactics |
| KILLSHP | Remove a ship from our local bubble of universe | verificada | `ship_kill` | desplaza las ranuras y los montones (KS3), KS2, TARGET LOST, Constrictor, JUNK, MANY y KS4; make test_univ6502 |
| ZINF | Reset the INWK workspace and orientation vectors | verificada | `ship_zinf` | make test_univ6502 |
| Ze | Initialise the INWK workspace to a fairly aggressive ship | verificada | `ze` | dentro de D1 |
| spasto | Contains the address of the Coriolis space station's ship | verificada | `ship_bp_for` | plano de la estacion: Dodo con nivel tecnologico >= 10 (NWSPS, make test_missions) |

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
| ZERO | Reset the local bubble of universe and ship status | verificada | `ships_zero`, `ships_clear` | pone a 0 FRIN..de (FRIN, MANY, JUNK, auto, ECMP, MJ, CABTMP, LAS2, MSAR, VIEW, LASCT, GNTMP, HFX, EV, DLY y de): el 6502 los limpia y no toca nada fuera; ships_zero limpia todos los del C (HFX no existe en el port), make test_missions |
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
