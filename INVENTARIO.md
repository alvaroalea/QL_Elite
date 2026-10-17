# Inventario de rutinas (Elite Apple II -> Elite QL)

Generado por `tools/inventory.py` a partir de la fuente de referencia y de `tools/ported.txt`.

Rutinas: 528. Portadas o con equivalente propio: 187 (verificadas con el emulador: 12). Marcadas como no necesarias en el estado actual del juego: 8.

Estados: **verificada** = comparada con el 6502 original (emulador); **transcripcion** = comparada con una transcripcion literal propia del 6502 (falta el emulador); **sin-verificar** = portada, en PENDIENTES.md; **propia** = sustituto o equivalente propio (la funcion C cubre lo que hacia la rutina, con otra forma); **no-necesaria** = la rutina no hace falta en el estado actual del juego (la nota dice por que y cuando hara falta); vacio = sin portar.

## Charts (9/9)

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

## Dashboard (25/29, 4 no necesarias)

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
| ABORT | Unarm missiles and update the dashboard indicators | no-necesaria |  | misiles aun no existen |
| ABORT2 | Set/unset the lock target for a missile and update the dashboard | no-necesaria |  | misiles aun no existen |
| msblob | Display the dashboard's missile indicators in green | sin-verificar | `dash_msblob` | NOMSL indicadores en verde y el resto apagados |
| ECMOF | Switch off the E.C.M. and turn off the dashboard bulb | no-necesaria |  | el ECM aun no existe en vuelo |
| MSBARS | Draw two horizontal lines as part of an indicator bar, from | sin-verificar | `msbars` | cuatro lineas bajo Y1 con gfx_hloin |
| ECBLB2 | Start up the E.C.M. (light up the indicator, start the countdown | no-necesaria |  | el ECM aun no existe en vuelo |
| ECBLB | Light up the E.C.M. indicator bulb ("E") on the dashboard | sin-verificar | `dash_ecblb` | bombilla E (aun sin ECM en vuelo) |
| SPBLB | Light up the space station indicator ("S") on the dashboard | sin-verificar | `dash_spblb` | bombilla S (estacion cerca) |
| BULB | Draw an indicator bulb on the dashboard | sin-verificar | `bulb` | 7 x 8 puntos en XOR; posicion (192, 176) la S y (56, 176) la E |
| ECBT | The character bitmap for the E.C.M. indicator bulb | sin-verificar | `bulb_e` |  |
| SPBT | The bitmap definition for the space station indicator bulb | sin-verificar | `bulb_s` |  |
| MSBAR | Draw a specific indicator in the dashboard's missile bar | sin-verificar | `msbar` | borra el color anterior y dibuja el nuevo (XOR) |
| msloc | The screen x-coordinates of the four missile indicators on the | sin-verificar | `msloc` | x = 40, 32, 24, 16 |
| SCAN | Display the current ship on the scanner | sin-verificar | `space_scan` | el bit 4 de INWK+31 (dibujada), colores de scacol y recorte a 146..190 como el original |

## Drawing circles (9/9)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| BLINE | Draw a circle segment and add it to the ball line heap | sin-verificar | `bline` |  |
| LL164 | Make the hyperspace sound and draw the hyperspace tunnel | sin-verificar | `arrive` | sonido (SOBLIP) y tunel al saltar; el salto en si es propio |
| LAUN | Make the launch sound and draw the launch tunnel | sin-verificar | `flight_run` | sonido (SOHISS) y tunel al despegar |
| HFS2 | Draw the launch or hyperspace tunnel | sin-verificar | `tunnel_draw` | STP 4 hiperespacio, 8 lanzamiento; el color (blanco) es una eleccion propia |
| TT14 | Draw a circle with crosshairs on a chart | sin-verificar | `tt14` |  |
| TT128 | Draw a circle on a chart | sin-verificar | `tt14` |  |
| CIRCLE | Draw a circle for the planet | sin-verificar | `circle_draw` |  |
| CIRCLE2 | Draw a circle (for the planet or chart) | sin-verificar | `circle2_draw` |  |
| CHKON | Check whether any part of a circle appears on the extended screen | sin-verificar | `sun_chkon` |  |

## Drawing lines (5/27)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| BOMBOFF | Draw the zig-zag lightning bolt for the energy bomb |  |  |  |
| BOMBEFF2 | Erase the energy bomb zig-zag lightning bolt, make the sound of |  |  |  |
| BOMBON | Randomise and draw the energy bomb's zig-zag lightning bolt lines |  |  |  |
| BOMBPOS | A set of x-coordinates that are used as the basis for the energy |  |  |  |
| BOMBTBX | This is where we store the x-coordinates for the energy bomb's |  |  |  |
| BOMBTBY | This is where we store the y-coordinates for the energy bomb's |  |  |  |
| NLI4 | Draw a line of dashes underneath a title on the text screen |  |  |  |
| NLIN3 | Print a title and draw a horizontal line at row 19 to box it in |  |  |  |
| NLIN4 | Draw a horizontal line at pixel row 19 to box in a title |  |  |  |
| NLIN | Draw a horizontal line at pixel row 23 to box in a title |  |  |  |
| NLIN2 | Draw a screen-wide horizontal line at the pixel row in A |  |  |  |
| HLOIN2 | Remove a line from the sun line heap and draw it on-screen | sin-verificar | `hloin2` |  |
| LASLI | Draw the laser lines for when we fire our lasers |  |  |  |
| DVLOIN | Draw a vertical line from (A, GCYT) to (A, GCYB) |  |  |  |
| TT15 | Draw a set of crosshairs | sin-verificar | `tt15` |  |
| EDGES | Draw a horizontal line given a centre and a half-width | sin-verificar | `edges` |  |
| LL118 | Move a point along a line until it is on-screen |  |  |  |
| LL145 | Clip line: Work out which end-points are on-screen, if any |  |  |  |
| LSPUT | Draw a ship line using flicker-free animation |  |  |  |
| sightcol | Colours for the crosshair sights on the different laser types |  |  |  |
| beamcol | An unused table of laser colours |  |  |  |
| TWFL | Ready-made pixel bytes for the left end of a horizontal line |  |  |  |
| TWFR | Ready-made character rows for the right end of a horizontal line |  |  |  |
| LOIN | Draw a line: Calculate the line gradient in the form of deltas |  |  |  |
| HLOIN | Draw a horizontal line from (X1, Y1) to (X2, Y1) | sin-verificar | `gfx_hloin` | mascara a pares y cobertura X1..X2-1; sin la geometria de bytes del Apple II |
| MASKT | High-resolution pixel bytes for drawing continuous lines of solid |  |  |  |
| VLOIN | Draw a vertical line from (X1, Y1) to (X1, Y2) | sin-verificar | `gfx_vloin` | 2 puntos de ancho, extremos incluidos |

## Drawing pixels (6/6)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| PIXEL2 | Draw a stardust particle relative to the screen centre | sin-verificar | `pixel2` |  |
| PIXEL | Draw a two-pixel dash, three-pixel dash or double-height | sin-verificar | `gfx_dust` | ancho segun ZZ y dos lineas si ZZ < 80; sin la geometria de bytes del Apple II |
| TWOS3 | Ready-made two-pixel and three-pixel bytes in white, with an extra | propia |  | tablas de bytes del Apple II: aqui gfx_xor_pixel |
| TWOS | Ready-made bytes for drawing one-pixel dots in the space view | propia |  |  |
| TWOS2 | Ready-made two-bit pixel bytes for plotting colour pixels | propia |  |  |
| CPIX | Draw a colour pixel | sin-verificar | `gfx_cpix` | 2 puntos de ancho (x, x+1) en XOR; sin las tablas de bytes del Apple II |

## Drawing planets (7/7)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| PL2 | Remove the planet or sun from the screen | sin-verificar | `planet` |  |
| PLANET | Draw the planet or sun | sin-verificar | `planet` | planeta (circulo con CIRCLE) y sol (SUN); PL9, PL2 y WP1 van dentro |
| PL9 | Draw the planet as a plain circle | sin-verificar | `planet` |  |
| WPLS2 | Remove the planet from the screen | sin-verificar | `circle_wpls2` |  |
| WP1 | Reset the ball line heap | sin-verificar | `circle_wpls2` | reinicio del monton de la bola; LSY2[0] = $FF como centinela (el original no lo inicializa a la vista) |
| PL21 | Return from a planet/sun-drawing routine with a failure flag | sin-verificar | `planet` | SEC; RTS: en C el retorno de PROJ ya lo cubre |
| PLS6 | Calculate (X K) = (A P+1 P) / (z_sign z_hi z_lo) | verificada | `sm_pls6` |  |

## Drawing ships (4/5, 1 no necesarias)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| scacol | Ship colours on the scanner | sin-verificar | `scacol` | cadena con el color de cada tipo |
| DOEXP | Draw an exploding ship | sin-verificar | `doexp` | nube de la nave que explota; el color de las particulas (azul/rojo/fuzzy/otro del Apple II) se muestra como cian/rojo/amarillo/blanco |
| exlook | A table to shift X left by one place when X is 0 or 1 | no-necesaria |  | tabla de EXNO (sonido de explosion): el sonido ya esta en sound.c |
| SHPPT | Draw a distant ship as a point rather than a full wireframe | verificada | `ship_shppt` |  |
| LL9 | Draw ship: Check if ship is exploding, check if ship is in front | verificada | `ship_ll9` |  |

## Drawing suns (3/3)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| FLFLLS | Reset the sun line heap | sin-verificar | `sun_flflls` |  |
| SUN | Draw the sun: Set up all the variables needed to draw the sun | sin-verificar | `sun_draw` |  |
| WPLS | Remove the sun from the screen | sin-verificar | `sun_wpls` |  |

## Drawing the screen (2/18)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| SCTBX1 | Lookup table for converting a pixel x-coordinate to the bit number |  |  |  |
| SCTBX2 | Lookup table for converting a pixel x-coordinate to the byte |  |  |  |
| TRADEMODE | Clear the screen and set up a trading screen |  |  |  |
| DET1 | Show or hide the dashboard (for when we die) |  |  |  |
| TT66 | Clear the screen and set the current view type | propia | `gfx_fill_lines` | borrado rapido de la vista y de la pantalla |
| TTX66 | Clear the top part of the screen, draw a border box if required |  |  |  |
| cellocl | Lookup table for converting a text row number to the address of |  |  |  |
| SCTBL | Lookup table for converting a character row number to the address |  |  |  |
| SCTBH | Lookup table for converting a character row number to the address |  |  |  |
| SCTBH2 | Lookup table for converting a character row number to the address |  |  |  |
| WSCAN | Wait for 15 * 256 loop iterations |  |  |  |
| clss | Clear the screen, move the text cursor to the top-left corner and |  |  |  |
| TTX66K | Clear the whole screen or just the space view (as appropriate), |  |  |  |
| cleargrap | Clear screen memory for the top part of the graphics screen mode |  |  |  |
| CLYNS | Clear two character rows near the bottom of the screen | propia | `txt_clear_rows` | borrado de filas de texto con el relleno rapido |
| clearrow | Clear a character row of screen memory, drawing blue borders along |  |  |  |
| HGR | Switch to the high-resolution graphics screen mode |  |  |  |
| TEXT | Switch to the text screen mode |  |  |  |

## Equipment (6/6)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| EQSHP | Show the Equip Ship screen | sin-verificar | `equip_buy` | la pantalla es scr_equip (propia); equip_buy lleva las reglas, el cobro y el reembolso |
| eq | Subtract the price of equipment from the cash pot | sin-verificar | `cash_spend` | dentro de equip_buy (cobra antes de reembolsar el laser antiguo) |
| prx | Return the price of a piece of equipment | sin-verificar | `equip_price` |  |
| qv | Print a menu of the four space views, for buying lasers | propia | `scr_equip` | pide la vista (0 a 3) al comprar un laser |
| refund | Install a new laser, processing a refund if applicable | sin-verificar | `laser_refund` |  |
| PRXS | Equipment prices | sin-verificar | `equip_price` |  |

## Flight (3/29)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| DOENTRY | Dock at the space station and work out any mission progression |  |  |  |
| ESCAPE | Launch our escape pod |  |  |  |
| DOCKIT | Apply docking manoeuvres to the ship in INWK |  |  |  |
| DCS1 | Calculate the vector from the ideal docking position to the ship |  |  |  |
| SESCP | Spawn an escape pod from the current (parent) ship |  |  |  |
| dockEd | Print a message to say there is no hyperspacing allowed inside the |  |  |  |
| hyp | Start the hyperspace process |  |  |  |
| wW | Start a hyperspace countdown |  |  |  |
| TTX110 | Set the current system to the nearest system and return to hyp |  |  |  |
| Ghy | Perform a galactic hyperspace jump |  |  |  |
| ee3 | Print the hyperspace countdown in the top-left of the screen |  |  |  |
| TT147 | Print an error when a system is out of hyperspace range |  |  |  |
| MJP | Process a mis-jump into witchspace |  |  |  |
| TT18 | Try to initiate a jump into hyperspace |  |  |  |
| TT110 | Launch from a station or show the front space view |  |  |  |
| SHD | Charge a shield and drain some energy from the energy banks |  |  |  |
| DENGY | Drain some energy from the energy banks |  |  |  |
| OOPS | Take some damage |  |  |  |
| me2 | Remove an in-flight message from the space view |  |  |  |
| WARP | Perform an in-system jump |  |  |  |
| me1 | Erase an old in-flight message and display a new one |  |  |  |
| MESS | Display an in-flight message |  |  |  |
| mes9 | Print a text token, possibly followed by " DESTROYED" |  |  |  |
| OUCH | Potentially lose cargo or equipment following damage |  |  |  |
| ou2 | Display "E.C.M.SYSTEM DESTROYED" as an in-flight message |  |  |  |
| ou3 | Display "FUEL SCOOPS DESTROYED" as an in-flight message |  |  |  |
| PLUT | Flip the coordinate axes for the four different views | sin-verificar | `plut` | ejes de INWK para las vistas trasera, izquierda y derecha |
| LOOK1 | Initialise the space view | propia | `change_view` | cambio de vista (FLIP + mira) y redraw_all (NWSTARS + mira) al volver de una pantalla de texto |
| SIGHT | Draw the laser crosshairs | sin-verificar | `chart_sight` | dos cruces concentricas con TT15 de la vista espacial; color segun el laser (pulsos azul, haz rojo, otros blanco) |

## Keyboard (8/19)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| TGINT | The keys used to toggle configuration settings when the game is |  |  |  |
| FLKB | Flush the keyboard buffer | propia | `plat_flush_input` |  |
| PAUSE2 | Wait until a key is pressed, ignoring any existing key press |  |  |  |
| TT214 | Ask a question with a "Y/N?" prompt and return the response | propia | `scr_disk` |  |
| YESNO | Wait until either "Y" or "N" is pressed | propia | `scr_disk` | confirmaciones Y/N dentro del menu de disco |
| TT17 | Scan the keyboard for cursor key or joystick movement | propia | `in_held` | cursores o joystick |
| TT102 | Process number key, save key, hyperspace and chart key presses | propia | `ui_nav` | en vuelo: 1-4 vistas, 5-9 pantallas, 0 carta galactica, H y G saltos; atracado: U despega, 0 galactica, 5-9 como en vuelo |
| ZEKTRAN | Clear the key logger |  |  |  |
| RDS1 | Scan one axis of the joystick and return its position |  |  |  |
| RDKEY | Scan the keyboard for key presses and the joystick, and update the |  |  |  |
| KYTB | Lookup table for in-flight keyboard controls |  |  |  |
| DKSANYKEY | An unused routine that scans the keyboard to detect whether any |  |  |  |
| DKS2 | Read the joystick position |  |  |  |
| DKS3 | Toggle a configuration setting and emit a beep |  |  |  |
| U% | Clear the key logger |  |  |  |
| DOKEY | Scan for the seven primary flight controls and apply the docking | propia | `ctl_read` | cursores = alabeo y cabeceo, F acelera, D frena, A o espacio disparan; JSTX/JSTY los calcula space_controls |
| DK4 | Scan for pause, configuration and secondary flight keys | propia | `in_poll` | teclas secundarias por caracteres (key_getchar) |
| TT217 | Scan the keyboard until a key is pressed | propia | `in_wait` |  |
| KTRAN | An unused key logger buffer that's left over from the 6502 Second |  |  |  |

## Loader (0/5)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| ENTRY | An entry point at the start of the game binary |  |  |  |
| S% | Checksum, decrypt and unscramble the main game code, and start the |  |  |  |
| BEGIN | Initialise the configuration variables and start the game |  |  |  |
| COLD | Initialise the screen mode, clear memory and set up interrupt |  |  |  |
| NMIpissoff | Acknowledge NMI interrupts and ignore them |  |  |  |

## Main loop (0/1)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| Main | Seed the random number generator |  |  |  |

## Market (18/19, 1 no necesarias)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| tnpr1 | Work out if we have space for one tonne of cargo | no-necesaria |  | espacio para una tonelada: solo lo usa la recogida de cargas (cazar canisters, cápsulas) en vuelo, que aun no existe; se portara con MVEIT/TACTICS |
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

## Maths (Arithmetic) (34/52)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| log | Binary logarithm table (high byte) |  |  |  |
| logL | Binary logarithm table (low byte) |  |  |  |
| alogh | Binary antilogarithm table |  |  |  |
| PIX1 | Calculate (YY+1 SYL+Y) = (A P) + (S R) and draw stardust particle |  |  |  |
| MAS3 | Calculate A = x_hi^2 + y_hi^2 + z_hi^2 in the K% block | transcripcion | `sm_mas3` |  |
| VCSU1 | Calculate vector K3(8 0) = [x y z] - coordinates of the sun or | transcripcion | `sm_vcsu1` |  |
| VCSUB | Calculate vector K3(8 0) = [x y z] - coordinates in (A V) | transcripcion | `sm_vcsub` |  |
| TAS1 | Calculate K3 = (x_sign x_hi x_lo) - V(1 0) | transcripcion | `sm_tas1` |  |
| MU5 | Set K(3 2 1 0) = (A A A A) and clear the C flag |  |  |  |
| MULT3 | Calculate K(3 2 1 0) = (A P+1 P) * Q | transcripcion | `em_mult3` |  |
| MLS2 | Calculate (S R) = XX(1 0) and (A P) = A * ALP1 |  |  |  |
| MLS1 | Calculate (A P) = ALP1 * A |  |  |  |
| MU6 | Set P(1 0) = (A A) |  |  |  |
| SQUA | Clear bit 7 of A and calculate (A P) = A * A | transcripcion | `em_squa` |  |
| SQUA2 | Calculate (A P) = A * A | transcripcion | `em_squa2` |  |
| MU1 | Copy X into P and A, and clear the C flag |  |  |  |
| MLU1 | Calculate Y1 = y_hi and (A P) = |y_hi| * Q for Y-th stardust | transcripcion | `em_mlu1` |  |
| MLU2 | Calculate (A P) = |A| * Q | transcripcion | `em_mlu2` |  |
| MULTU | Calculate (A P) = P * Q | transcripcion | `em_multu` |  |
| MU11 | Calculate (A P) = P * X | transcripcion | `em_mu11` |  |
| FMLTU2 | Calculate A = K * sin(A) | transcripcion | `em_fmltu2` |  |
| FMLTU | Calculate A = A * Q / 256 | verificada | `em_fmltu` |  |
| MLTU2 | Calculate (A P+1 P) = (A ~P) * Q | transcripcion | `em_mltu2` |  |
| MUT3 | An unused routine that does the same as MUT2 |  |  |  |
| MUT2 | Calculate (S R) = XX(1 0) and (A P) = Q * A |  |  |  |
| MUT1 | Calculate R = XX and (A P) = Q * A |  |  |  |
| MULT1 | Calculate (A P) = Q * A | transcripcion | `em_mult1` |  |
| MULT12 | Calculate (S R) = Q * A | transcripcion | `em_mult12` |  |
| MAD | Calculate (A X) = Q * A + (S R) | transcripcion | `em_mad` |  |
| ADD | Calculate (A X) = (A P) + (S R) | transcripcion | `em_add` |  |
| TIS1 | Calculate (A ?) = (-X * A + (S R)) / 96 | transcripcion | `em_tis1` |  |
| DV42 | Calculate (P R) = 256 * DELTA / z_hi | transcripcion | `em_dv42` |  |
| DV41 | Calculate (P R) = 256 * DELTA / A | transcripcion | `em_dv41` |  |
| DVID4 | Calculate (P R) = 256 * A / Q | transcripcion | `em_dvid4` |  |
| DVID3B2 | Calculate K(3 2 1 0) = (A P+1 P) / (z_sign z_hi z_lo) |  |  |  |
| LCASH | Subtract an amount of cash from the cash pot | transcripcion | `sm_lcash` |  |
| MCASH | Add an amount of cash to the cash pot | transcripcion | `sm_mcash` |  |
| GCASH | Calculate (Y X) = P * Q * 4 | transcripcion | `sm_gcash` |  |
| GC2 | Calculate (Y X) = (A P) * 4 |  |  |  |
| SPS2 | Calculate (Y X) = A / 10 | transcripcion | `sm_sps2` |  |
| DORND | Generate random numbers | verificada | `em_dornd` |  |
| TIS2 | Calculate A = A / Q | transcripcion | `em_tis2` |  |
| TIS3 | Calculate -(nosev_1 * roofv_1 + nosev_2 * roofv_2) / nosev_3 | transcripcion | `sm_tis3` |  |
| DVIDT | Calculate (P+1 A) = (A P) / Q | transcripcion | `em_dvidt` |  |
| LL5 | Calculate Q = SQRT(R Q) | transcripcion | `em_ll5` |  |
| LL28 | Calculate R = 256 * A / Q | verificada | `em_ll28` |  |
| LL38 | Calculate (S A) = (S R) + (A Q) | transcripcion | `em_ll38` |  |
| LL61 | Calculate (U R) = 256 * A / Q | verificada | `em_ll61` |  |
| LL62 | Calculate 128 - (U R) |  |  |  |
| LL120 | Calculate (Y X) = (S x1_lo) * XX12+2 or (S x1_lo) / XX12+2 |  |  |  |
| LL123 | Calculate (Y X) = (S R) / XX12+2 or (S R) * XX12+2 |  |  |  |
| LL129 | Calculate Q = XX12+2, A = S EOR XX12+3 and (S R) = |S R| |  |  |  |

## Maths (Geometry) (18/19)

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
| FAROF2 | Compare x_hi, y_hi and z_hi with A |  |  |  |
| MAS4 | Calculate a cap on the maximum distance to a ship | transcripcion | `sm_mas4` |  |
| SPS1 | Calculate the vector to the planet and store it in XX15 | transcripcion | `sm_sps1` |  |
| TAS2 | Normalise the three-coordinate vector in K3 | transcripcion | `sm_tas2` |  |
| NORM | Normalise the three-coordinate vector in XX15 | transcripcion | `sm_norm` |  |
| TIDY | Orthonormalise the orientation vectors for a ship | transcripcion | `sm_tidy` |  |
| LL51 | Calculate the dot product of XX15 and XX16 | verificada | `ship_ll51` |  |

## Missions (0/16)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| TRIBDIR | The low byte of the four 16-bit directions in which Trumble |  |  |  |
| TRIBDIRH | The high byte of the four 16-bit directions in which Trumble |  |  |  |
| SPMASK | Masks for updating sprite bits in VIC+$10 for the top bit of the |  |  |  |
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
| TRIBTA | A table for converting the number of Trumbles in the hold into a |  |  |  |
| TRIBMA | A table for converting the number of Trumbles in the hold into a |  |  |  |

## Moving (7/8, 1 no necesarias)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| MVT3 | Calculate K(3 2 1) = (x_sign x_hi x_lo) + K(3 2 1) | transcripcion | `sm_mvt3` |  |
| MVS5 | Apply a 3.6 degree pitch or roll to an orientation vector | sin-verificar | `mvs5` |  |
| SFS2 | Move a ship in space along one of the coordinate axes | no-necesaria |  | mover una nave a lo largo de un eje: solo para crear naves hijas (misiles, cargas, cápsulas) |
| MVEIT | Move current ship: Tidy the orientation vectors | sin-verificar | `mveit` | partes 1 a 9; TACTICS es un gancho (move_tactics_hook) hasta el paso 3; los acarreos de MLTU2 salen del bit bajo del producto |
| MVT1 | Calculate (x_sign x_hi x_lo) = (x_sign x_hi x_lo) + (A R) | sin-verificar | `mvt1` |  |
| MVS4 | Apply pitch and roll to an orientation vector | sin-verificar | `mvs4` |  |
| MVT6 | Calculate (A P+2 P+1) = (x_sign x_hi x_lo) + (A P+2 P+1) | sin-verificar | `mvt6` |  |
| MV40 | Rotate the planet or sun's location in space by the amount of | sin-verificar | `mv40` | gira la posicion y sigue por MV45 (orientacion, contadores y radar); K2 byte 0 queda a 0 (en el original es lo que hubiera en K2) |

## Save and load (5/61)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| wtable | 6-bit to 7-bit nibble conversion table |  |  |  |
| BRKBK | Set the standard BRKV handler for the game |  |  |  |
| NA% | The data block for the last saved commander |  |  |  |
| CHK2 | Second checksum byte for the saved commander data file |  |  |  |
| CHK3 | Third checksum byte for the saved commander data file |  |  |  |
| CHK | First checksum byte for the saved commander data file |  |  |  |
| S1% | The drive and directory number used when saving or loading a |  |  |  |
| NA2% | The data block for the default commander |  |  |  |
| CHECK | Calculate the checksum for the last saved commander data block | transcripcion | `cmdr_check` |  |
| CHECK2 | Calculate the third checksum for the last saved commander data | transcripcion | `cmdr_check2` |  |
| JAMESON | Restore the default JAMESON commander | sin-verificar | `cmdr_default` |  |
| COPYNAME | Copy the last saved commander's name from INWK+5 to comnam and pad |  |  |  |
| TRNME | Copy the last saved commander's name from INWK to NA% |  |  |  |
| TR1 | Copy the last saved commander's name from NA% to INWK |  |  |  |
| GTNMEW | Fetch the name of a commander file to save or load |  |  |  |
| FILEPR | Display the currently selected media (disk or tape) |  |  |  |
| OTHERFILEPR | Display the non-selected media (disk or tape) |  |  |  |
| SVE | Display the disk access menu and process saving of commander files | propia | `scr_disk` |  |
| diskerror | Print a disk error, make a beep and wait for a key press |  |  |  |
| thislong | Contains the length of the most recently entered commander name |  |  |  |
| oldlong | Contains the length of the last saved commander name |  |  |  |
| LOD | Load a commander file | propia | `load_commander` |  |
| DERR1 | The error message for when a disk is write protected |  |  |  |
| DERR2 | The error message for when a disk is full |  |  |  |
| DERR3 | The error message for when a disk catalog is full |  |  |  |
| DERR4 | The error message for when there is a disk I/O error |  |  |  |
| DERR5 | The error message for when a file is not found |  |  |  |
| ERTAB | A lookup table for the five disk error messages |  |  |  |
| comnam | Storage for the commander filename, padded out with spaces to a |  |  |  |
| rfile | Read a commander file from a DOS disk into the buffer |  |  |  |
| wfile | Write a commander file from the buffer to a DOS disk |  |  |  |
| findf | Search the disk catalog for an existing file |  |  |  |
| finde | Search the disk catalog for an empty file entry |  |  |  |
| rentry | Search the disk catalog for an existing file or an empty file |  |  |  |
| getsct | Analyse the VTOC sector to allocate one free sector |  |  |  |
| isfull | Check the disk to ensure there are least two free sectors, one for |  |  |  |
| gettsl | Read a file's track/sector list |  |  |  |
| rvtoc | Read the VTOC sector into the buffer |  |  |  |
| rsect | Read a specific sector from disk into the buffer |  |  |  |
| wsect | Write a specific sector from the buffer to disk |  |  |  |
| rwts | Read or write a specific sector |  |  |  |
| trytrk | Try finding a specific track on the disk |  |  |  |
| rdrght | Check that this is the correct track |  |  |  |
| prterr | Return from the RWTS code with a "Disk write protected" error |  |  |  |
| rttrk | Read or write a sector on the current track |  |  |  |
| drverr | Return from the RWTS code with a "Disk I/O error" |  |  |  |
| rttrk3 | Successfully return from the RWTS code with no error reported |  |  |  |
| read | Read a sector's worth of data into the buffr2 buffer |  |  |  |
| write | Write a sector's worth of data from the buffr2 buffer to the |  |  |  |
| rdaddr | Read a track address field |  |  |  |
| seek | Fast seek routine |  |  |  |
| armwat | Implement the arm move delay |  |  |  |
| armtab | Phase-on time table in 100-usec intervals |  |  |  |
| armtb2 | Phase-off time table in 100-usec intervals |  |  |  |
| prenib | Convert 256 8-bit bytes in buffer into 342 6-bit nibbles in buffr2 |  |  |  |
| pstnib | Convert 342 6-bit nibbles in buffr2 into 256 8-bit bytes in buffer |  |  |  |
| wbyte | Write one byte to disk |  |  |  |
| scttab | Lookup table to translate logical (requested) sector number to |  |  |  |
| rtable | 64 disk nibbles of "6-and-2" Read Translate Table |  |  |  |
| MUTILATE | Encrypt the commander file in the buffer at comfil |  |  |  |
| UNMUTILATE | Decrypt the commander file in the buffer at comfil |  |  |  |

## Sound (0/13)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| BELL | Make a standard system beep |  |  |  |
| EXNO | Make the sound of a laser strike on another ship |  |  |  |
| BOOP | Make a long, low beep |  |  |  |
| SOHISS | Make the sound of a launch from the station, hyperspace or missile |  |  |  |
| EXNO3 | Make the sound of a collision, or an exploding cargo canister or |  |  |  |
| SOEXPL | Make an explosion sound |  |  |  |
| BEEP | Make a short, high beep |  |  |  |
| SOBEEP | Make a beep |  |  |  |
| SOBLIP | Make the sound of the hyperspace drive being engaged, or the sound |  |  |  |
| LASNOISE | Make the sound of our laser firing or the sound of us being hit by |  |  |  |
| LASNOISE2 | An unused routine that makes a different laser sound |  |  |  |
| SOBOMB | Make the sound of an energy bomb going off |  |  |  |
| CLICK | Toggle the state of the speaker (i.e. move it in or out) to make a |  |  |  |

## Stardust (7/7)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| FLIP | Reflect the stardust particles in the screen diagonal and redraw | sin-verificar | `stars_flip` |  |
| STARS | The main routine for processing the stardust | sin-verificar | `stars_update` | vistas 0 (STARS1), 1 (STARS6), 2 y 3 (STARS2) |
| STARS1 | Process the stardust for the front view | sin-verificar | `stars1` | los acarreos del 6502 se siguen con una variable c (LSR/ROR/SBC/ADC) |
| STARS6 | Process the stardust for the rear view | sin-verificar | `stars6` |  |
| STARS2 | Process the stardust for the left or right view | sin-verificar | `stars2` | con ST2 al entrar y al salir |
| NWSTARS | Initialise the stardust field | sin-verificar | `stars_new` | el original no crea el polvo si QQ11 != 0; aqui solo se llama en la vista espacial |
| nWq | Create a random cloud of stardust | sin-verificar | `stars_new` |  |

## Start and end (1/8)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| RESET | Reset most variables |  |  |  |
| RES2 | Reset a number of flight variables and workspaces |  |  |  |
| DEATH | Display the death screen |  |  |  |
| TT170 | Main entry point for the Elite game code |  |  |  |
| DEATH2 | Reset most of the game and restart from the title screen |  |  |  |
| BR1 | Show the "Load New Commander (Y/N)?" screen and start the game |  |  |  |
| DFAULT | Reset the current commander data block to the last saved commander | sin-verificar | `cmdr_unpack` |  |
| TITLE | Display a title screen with a rotating ship and prompt |  |  |  |

## Status (1/7)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| STATUS | Show the Status Mode screen | sin-verificar | `scr_status` | maquetacion propia; rango y estado legal en rating_name / legal_name |
| cmn | Print the commander's name |  |  |  |
| fwl | Print fuel and cash levels |  |  |  |
| csh | Print the current amount of cash |  |  |  |
| BAD | Calculate how bad we have been |  |  |  |
| BAY | Go to the docking bay (i.e. show the Status Mode screen) |  |  |  |
| EXNO2 | Process us making a kill |  |  |  |

## Tactics (0/8)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| TACTICS | Apply tactics: Process missiles, both enemy missiles and our own |  |  |  |
| HITCH | Work out if the ship in INWK is in our crosshairs |  |  |  |
| FRS1 | Launch a ship straight ahead of us, below the laser sights |  |  |  |
| FRMIS | Fire a missile from our ship |  |  |  |
| ANGRY | Make a ship or station hostile, and if this is a ship then enable |  |  |  |
| FR1 | Display the "missile jammed" message |  |  |  |
| yetanotherrts | Contains an RTS |  |  |  |
| SFRMIS | Add an enemy missile to our local bubble of universe |  |  |  |

## Text (0/77)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| MT27 | Print the captain's name during mission briefings |  |  |  |
| MT28 | Print the location hint during the mission 1 briefing |  |  |  |
| DETOK3 | Print an extended recursive token from the RUTOK token table |  |  |  |
| DETOK | Print an extended recursive token from the TKN1 token table |  |  |  |
| DETOK2 | Print an extended text token (1-255) |  |  |  |
| MT1 | Switch to ALL CAPS when printing extended tokens |  |  |  |
| MT2 | Switch to Sentence Case when printing extended tokens |  |  |  |
| MT8 | Tab to column 6 and start a new word when printing extended tokens |  |  |  |
| MT9 | Clear the screen and set the current view type to 1 |  |  |  |
| MT13 | Switch to lower case when printing extended tokens |  |  |  |
| MT6 | Switch to standard tokens in Sentence Case |  |  |  |
| MT5 | Switch to extended tokens |  |  |  |
| MT14 | Switch to justified text when printing extended tokens |  |  |  |
| MT15 | Switch to left-aligned text when printing extended tokens |  |  |  |
| MT17 | Print the selected system's adjective, e.g. Lavian for Lave |  |  |  |
| MT18 | Print a random 1-8 letter word in Sentence Case |  |  |  |
| MT19 | Capitalise the next letter |  |  |  |
| VOWEL | Test whether a character is a vowel |  |  |  |
| WHITETEXT | Switch to white text |  |  |  |
| JMTB | The extended token table for jump tokens 1-32 (DETOK) |  |  |  |
| TKN2 | The extended two-letter token lookup table |  |  |  |
| QQ16 | The two-letter token lookup table |  |  |  |
| plf2 | Print text followed by a newline and indent of 6 characters |  |  |  |
| TENS | A constant used when printing large numbers in BPRNT |  |  |  |
| pr2 | Print an 8-bit number, left-padded to 3 digits, and optional point |  |  |  |
| TT11 | Print a 16-bit number, left-padded to n digits, and optional point |  |  |  |
| BPRNT | Print a 32-bit number, left-padded to a specific number of digits, |  |  |  |
| DTW1 | A mask for applying the lower case part of Sentence Case to |  |  |  |
| DTW2 | A flag that indicates whether we are currently printing a word |  |  |  |
| DTW3 | A flag for switching between standard and extended text tokens |  |  |  |
| DTW4 | Flags that govern how justified extended text tokens are printed |  |  |  |
| DTW5 | The size of the justified text buffer at BUF |  |  |  |
| DTW6 | A flag to denote whether printing in lower case is enabled for |  |  |  |
| DTW8 | A mask for capitalising the next letter in an extended text token |  |  |  |
| FEED | Print a newline |  |  |  |
| MT16 | Print the character in variable DTW7 |  |  |  |
| TT26 | Print a character at the text cursor, with support for verified |  |  |  |
| MT23 | Move to row 9 and switch to lower case |  |  |  |
| MT29 | Move to row 5 and switch to lower case when printing extended |  |  |  |
| MTIN | Lookup table for random tokens in the extended token table (0-37) |  |  |  |
| DOXC | Move the text cursor to a specific column |  |  |  |
| DOYC | Move the text cursor to a specific row |  |  |  |
| INCYC | Move the text cursor to the next row |  |  |  |
| TT60 | Print a text token and a paragraph break |  |  |  |
| TTX69 | Print a paragraph break |  |  |  |
| TT69 | Set Sentence Case and print a newline |  |  |  |
| TT67 | Print a newline |  |  |  |
| spc | Print a text token followed by a space |  |  |  |
| pr6 | Print 16-bit number, left-padded to 5 digits, no point |  |  |  |
| pr5 | Print a 16-bit number, left-padded to 5 digits, and optional point |  |  |  |
| prq | Print a text token followed by a question mark |  |  |  |
| TT162 | Print a space |  |  |  |
| dn2 | Make a low, long beep and delay for 0.5 seconds |  |  |  |
| plf | Print a text token followed by a newline |  |  |  |
| TT68 | Print a text token followed by a colon |  |  |  |
| TT73 | Print a colon |  |  |  |
| TT27 | Print a text token |  |  |  |
| TT42 | Print a letter in lower case |  |  |  |
| TT41 | Print a letter according to Sentence Case |  |  |  |
| qw | Print a recursive token in the range 128-145 |  |  |  |
| crlf | Tab to column 21 and print a colon |  |  |  |
| TT45 | Print a letter in lower case |  |  |  |
| TT46 | Print a character and switch to capitals |  |  |  |
| TT74 | Print a character |  |  |  |
| TT43 | Print a two-letter token or recursive token 0-95 |  |  |  |
| ex | Print a recursive token |  |  |  |
| MT26 | Fetch a line of text from the keyboard |  |  |  |
| RLINE | The OSWORD configuration block used to fetch a line of text from |  |  |  |
| CHPR2 | Character print vector handler |  |  |  |
| R5 | Make a beep and jump back into the character-printing routine at |  |  |  |
| RR5 | Print a character in the text screen mode |  |  |  |
| TT67K | Print a newline |  |  |  |
| CHPR | Print a character at the text cursor by poking into screen memory |  |  |  |
| letter | Draw a character in the high-resolution screen mode using the game |  |  |  |
| letter2 | Draw a character or indicator bulb bitmap in the high-resolution |  |  |  |
| SETXC | Move the text cursor to a specific column |  |  |  |
| SETYC | Move the text cursor to a specific row |  |  |  |

## Tube (0/1)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| newosrdch | The custom OSRDCH routine for reading characters |  |  |  |

## Universe (9/34, 1 no necesarias)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| SPIN | Randomly spawn cargo from a destroyed ship |  |  |  |
| UNIV | Table of pointers to the local universe's ship data blocks |  |  |  |
| SFS1 | Spawn a child ship from the current (parent) ship |  |  |  |
| PDESC | Print the system's extended description or a mission 1 directive |  |  |  |
| GINF | Fetch the address of a ship's data block into INF | no-necesaria |  | el puntero INF no existe en C: se indexa por ranura |
| ping | Set the selected system to the current system | sin-verificar | `chart_ping` | tecla O: el cursor vuelve al sistema actual |
| TT20 | Twist the selected system's seeds four times |  |  |  |
| TT54 | Twist the selected system's seeds |  |  |  |
| TT146 | Print the distance to the selected system in light years |  |  |  |
| TT70 | Display "MAINLY " and jump to TT72 |  |  |  |
| TT25 | Show the Data on System screen | sin-verificar | `scr_data` |  |
| TT24 | Calculate system data from the system seeds |  |  |  |
| TT81 | Set the selected system's seeds to those of system 0 |  |  |  |
| TT111 | Set the current system to the nearest system to a point | sin-verificar | `uni_nearest` |  |
| jmp | Set the current system to the selected system |  |  |  |
| hyp1 | Process a jump to the system closest to (QQ9, QQ10) |  |  |  |
| GVL | Calculate the availability of market items |  |  |  |
| GTHG | Spawn a Thargoid ship and a Thargon companion |  |  |  |
| cpl | Print the selected system name |  |  |  |
| ypl | Print the current system name |  |  |  |
| tal | Print the current galaxy number |  |  |  |
| SOS1 | Update the missile indicators, set up the planet data block | sin-verificar | `make_planet` |  |
| SOLAR | Set up various aspects of arriving in a new system | sin-verificar | `ships_launch` | planeta con las formulas de SOLAR; al despegar el sol se sustituye por la estacion; ships_arrive pone planeta y sol |
| NWSPS | Add a new space station to our local bubble of universe | sin-verificar | `make_station` | la posicion de la estacion es propia |
| NWSHP | Add a new ship to our local bubble of universe | sin-verificar | `ship_new` | sin heap compartido (SLSP): un monton de 256 bytes por ranura; NEWB inicial de E% |
| NwS1 | Flip the sign and double an INWK byte |  |  |  |
| KS3 | Set the SLSP ship line heap pointer after shuffling ship slots |  |  |  |
| KS1 | Remove the current ship from our local bubble of universe |  |  |  |
| KS4 | Remove the space station and replace it with the sun |  |  |  |
| KS2 | Check the local bubble for missiles with target lock |  |  |  |
| KILLSHP | Remove a ship from our local bubble of universe | sin-verificar | `ship_kill` | sin puntos de muerte ni mensajes (paso 3) |
| ZINF | Reset the INWK workspace and orientation vectors | sin-verificar | `ship_zinf` |  |
| Ze | Initialise the INWK workspace to a fairly aggressive ship |  |  |  |
| spasto | Contains the address of the Coriolis space station's ship |  |  |  |

## Utility routines (0/18)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| DEEOR | Unscramble the main code |  |  |  |
| DEEORS | Decrypt a multi-page block of memory |  |  |  |
| G% | Denotes the start of the main game code, from ELITE A to ELITE K |  |  |  |
| DELAY | Wait for a specified time, measured in loop iterations |  |  |  |
| SWAPPZERO | An unused routine that swaps bytes in and out of zero page |  |  |  |
| brkd | A flag that indicates whether a system error has occurred |  |  |  |
| BRBR | The standard BRKV handler for the game |  |  |  |
| ZERO | Reset the local bubble of universe and ship status |  |  |  |
| ZEBC | Zero-fill pages $B and $C |  |  |  |
| ZES1 | Zero-fill the page whose number is in X |  |  |  |
| ZES2 | Zero-fill a specific page |  |  |  |
| backtonormal | Disable the keyboard, set the SVN flag to 0, and return with A = 0 |  |  |  |
| CLDELAY | Delay by iterating through 5 * 256 (1280) empty loops |  |  |  |
| ZES1k | Zero-fill the page whose number is in X |  |  |  |
| ZES2k | Zero-fill a specific page |  |  |  |
| ZESNEW | Zero-fill memory from SC(1 0) to the end of the page |  |  |  |
| mvblockK | Copy a specific number of pages in memory |  |  |  |
| F% | Denotes the end of the main game code, from ELITE A to ELITE K |  |  |  |

## Workspaces (0/7)

| Rutina | Resumen | Estado | C | Nota |
|---|---|---|---|---|
| ZP | Lots of important variables are stored in the zero page workspace |  |  |  |
| XX3 | Temporary storage space for complex calculations |  |  |  |
| K% | Ship data blocks and ship line heaps |  |  |  |
| Disk | Variables used by the disk operations and DOS 3.3 RWTS routines |  |  |  |
| UP | Configuration variables |  |  |  |
| WP | Variables |  |  |  |
| Option | Variables that are predominantly used to store the game options |  |  |  |
