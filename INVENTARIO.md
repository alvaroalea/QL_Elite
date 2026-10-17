# Inventario de rutinas (Elite Apple II -> Elite QL)

Generado por `tools/inventory.py` a partir de la fuente de referencia y de `tools/ported.txt`.

Rutinas: 528. Portadas: 76 (verificadas con el emulador: 9)

Estados: **verificada** = comparada con el 6502 original (emulador); **transcripcion** = comparada con una transcripcion literal propia del 6502 (falta el emulador); **sin-verificar** = portada, en PENDIENTES.md; **propia** = sustituto no basado en la rutina; vacio = sin portar.

## Charts (2/9)

| Rutina | Resumen | Estado | C |
|---|---|---|---|
| HME2 | Search the galaxy for a system |  |  |
| TT22 | Show the Long-range Chart | sin-verificar | `scr_chart` |
| TT16 | Move the crosshairs on a chart |  |  |
| TT103 | Draw a small set of crosshairs on a chart |  |  |
| TT123 | Move galactic coordinates by a signed delta |  |  |
| TT105 | Draw crosshairs on the Short-range Chart, with clipping |  |  |
| TT23 | Show the Short-range Chart | sin-verificar | `scr_chart` |
| TT114 | Display either the Long-range or Short-range Chart |  |  |
| hm | Select the closest system and redraw the chart crosshairs |  |  |

## Dashboard (0/29)

| Rutina | Resumen | Estado | C |
|---|---|---|---|
| DIALS | Update the dashboard: speed indicator |  |  |
| DIS1 | Update a bar-based indicator on the dashboard |  |  |
| DIS5 | Update the roll or pitch indicator on the dashboard |  |  |
| dialY | The screen y-coordinate of the top-left corner of each indicator |  |  |
| dialle | The threshold value for each indicator that defines low and high |  |  |
| dialc1 | The colour for each indicator for values that are below the |  |  |
| dialc2 | The colour for each indicator for values that are on or above the |  |  |
| cntr | Apply damping to the pitch or roll dashboard indicator |  |  |
| BUMP2 | Bump up the value of the pitch or roll dashboard indicator |  |  |
| REDU2 | Reduce the value of the pitch or roll dashboard indicator |  |  |
| WPSHPS | Clear the scanner, reset the ball line and sun line heaps |  |  |
| COMPAS | Update the compass |  |  |
| SP1 | Draw the space station on the compass |  |  |
| SP2 | Draw a dot on the compass, given the planet/station vector |  |  |
| DOT | Draw a dash on the compass |  |  |
| ABORT | Unarm missiles and update the dashboard indicators |  |  |
| ABORT2 | Set/unset the lock target for a missile and update the dashboard |  |  |
| msblob | Display the dashboard's missile indicators in green |  |  |
| ECMOF | Switch off the E.C.M. and turn off the dashboard bulb |  |  |
| MSBARS | Draw two horizontal lines as part of an indicator bar, from |  |  |
| ECBLB2 | Start up the E.C.M. (light up the indicator, start the countdown |  |  |
| ECBLB | Light up the E.C.M. indicator bulb ("E") on the dashboard |  |  |
| SPBLB | Light up the space station indicator ("S") on the dashboard |  |  |
| BULB | Draw an indicator bulb on the dashboard |  |  |
| ECBT | The character bitmap for the E.C.M. indicator bulb |  |  |
| SPBT | The bitmap definition for the space station indicator bulb |  |  |
| MSBAR | Draw a specific indicator in the dashboard's missile bar |  |  |
| msloc | The screen x-coordinates of the four missile indicators on the |  |  |
| SCAN | Display the current ship on the scanner |  |  |

## Drawing circles (0/9)

| Rutina | Resumen | Estado | C |
|---|---|---|---|
| BLINE | Draw a circle segment and add it to the ball line heap |  |  |
| LL164 | Make the hyperspace sound and draw the hyperspace tunnel |  |  |
| LAUN | Make the launch sound and draw the launch tunnel |  |  |
| HFS2 | Draw the launch or hyperspace tunnel |  |  |
| TT14 | Draw a circle with crosshairs on a chart |  |  |
| TT128 | Draw a circle on a chart |  |  |
| CIRCLE | Draw a circle for the planet |  |  |
| CIRCLE2 | Draw a circle (for the planet or chart) |  |  |
| CHKON | Check whether any part of a circle appears on the extended screen |  |  |

## Drawing lines (0/27)

| Rutina | Resumen | Estado | C |
|---|---|---|---|
| BOMBOFF | Draw the zig-zag lightning bolt for the energy bomb |  |  |
| BOMBEFF2 | Erase the energy bomb zig-zag lightning bolt, make the sound of |  |  |
| BOMBON | Randomise and draw the energy bomb's zig-zag lightning bolt lines |  |  |
| BOMBPOS | A set of x-coordinates that are used as the basis for the energy |  |  |
| BOMBTBX | This is where we store the x-coordinates for the energy bomb's |  |  |
| BOMBTBY | This is where we store the y-coordinates for the energy bomb's |  |  |
| NLI4 | Draw a line of dashes underneath a title on the text screen |  |  |
| NLIN3 | Print a title and draw a horizontal line at row 19 to box it in |  |  |
| NLIN4 | Draw a horizontal line at pixel row 19 to box in a title |  |  |
| NLIN | Draw a horizontal line at pixel row 23 to box in a title |  |  |
| NLIN2 | Draw a screen-wide horizontal line at the pixel row in A |  |  |
| HLOIN2 | Remove a line from the sun line heap and draw it on-screen |  |  |
| LASLI | Draw the laser lines for when we fire our lasers |  |  |
| DVLOIN | Draw a vertical line from (A, GCYT) to (A, GCYB) |  |  |
| TT15 | Draw a set of crosshairs |  |  |
| EDGES | Draw a horizontal line given a centre and a half-width |  |  |
| LL118 | Move a point along a line until it is on-screen |  |  |
| LL145 | Clip line: Work out which end-points are on-screen, if any |  |  |
| LSPUT | Draw a ship line using flicker-free animation |  |  |
| sightcol | Colours for the crosshair sights on the different laser types |  |  |
| beamcol | An unused table of laser colours |  |  |
| TWFL | Ready-made pixel bytes for the left end of a horizontal line |  |  |
| TWFR | Ready-made character rows for the right end of a horizontal line |  |  |
| LOIN | Draw a line: Calculate the line gradient in the form of deltas |  |  |
| HLOIN | Draw a horizontal line from (X1, Y1) to (X2, Y1) |  |  |
| MASKT | High-resolution pixel bytes for drawing continuous lines of solid |  |  |
| VLOIN | Draw a vertical line from (X1, Y1) to (X1, Y2) |  |  |

## Drawing pixels (0/6)

| Rutina | Resumen | Estado | C |
|---|---|---|---|
| PIXEL2 | Draw a stardust particle relative to the screen centre |  |  |
| PIXEL | Draw a two-pixel dash, three-pixel dash or double-height |  |  |
| TWOS3 | Ready-made two-pixel and three-pixel bytes in white, with an extra |  |  |
| TWOS | Ready-made bytes for drawing one-pixel dots in the space view |  |  |
| TWOS2 | Ready-made two-bit pixel bytes for plotting colour pixels |  |  |
| CPIX | Draw a colour pixel |  |  |

## Drawing planets (1/7)

| Rutina | Resumen | Estado | C |
|---|---|---|---|
| PL2 | Remove the planet or sun from the screen |  |  |
| PLANET | Draw the planet or sun |  |  |
| PL9 | Draw the planet as a plain circle |  |  |
| WPLS2 | Remove the planet from the screen |  |  |
| WP1 | Reset the ball line heap |  |  |
| PL21 | Return from a planet/sun-drawing routine with a failure flag |  |  |
| PLS6 | Calculate (X K) = (A P+1 P) / (z_sign z_hi z_lo) | verificada | `sm_pls6` |

## Drawing ships (2/5)

| Rutina | Resumen | Estado | C |
|---|---|---|---|
| scacol | Ship colours on the scanner |  |  |
| DOEXP | Draw an exploding ship |  |  |
| exlook | A table to shift X left by one place when X is 0 or 1 |  |  |
| SHPPT | Draw a distant ship as a point rather than a full wireframe | verificada | `ship_shppt` |
| LL9 | Draw ship: Check if ship is exploding, check if ship is in front | verificada | `ship_ll9` |

## Drawing suns (0/3)

| Rutina | Resumen | Estado | C |
|---|---|---|---|
| FLFLLS | Reset the sun line heap |  |  |
| SUN | Draw the sun: Set up all the variables needed to draw the sun |  |  |
| WPLS | Remove the sun from the screen |  |  |

## Drawing the screen (0/18)

| Rutina | Resumen | Estado | C |
|---|---|---|---|
| SCTBX1 | Lookup table for converting a pixel x-coordinate to the bit number |  |  |
| SCTBX2 | Lookup table for converting a pixel x-coordinate to the byte |  |  |
| TRADEMODE | Clear the screen and set up a trading screen |  |  |
| DET1 | Show or hide the dashboard (for when we die) |  |  |
| TT66 | Clear the screen and set the current view type |  |  |
| TTX66 | Clear the top part of the screen, draw a border box if required |  |  |
| cellocl | Lookup table for converting a text row number to the address of |  |  |
| SCTBL | Lookup table for converting a character row number to the address |  |  |
| SCTBH | Lookup table for converting a character row number to the address |  |  |
| SCTBH2 | Lookup table for converting a character row number to the address |  |  |
| WSCAN | Wait for 15 * 256 loop iterations |  |  |
| clss | Clear the screen, move the text cursor to the top-left corner and |  |  |
| TTX66K | Clear the whole screen or just the space view (as appropriate), |  |  |
| cleargrap | Clear screen memory for the top part of the graphics screen mode |  |  |
| CLYNS | Clear two character rows near the bottom of the screen |  |  |
| clearrow | Clear a character row of screen memory, drawing blue borders along |  |  |
| HGR | Switch to the high-resolution graphics screen mode |  |  |
| TEXT | Switch to the text screen mode |  |  |

## Equipment (3/6)

| Rutina | Resumen | Estado | C |
|---|---|---|---|
| EQSHP | Show the Equip Ship screen | sin-verificar | `equip_buy` |
| eq | Subtract the price of equipment from the cash pot |  |  |
| prx | Return the price of a piece of equipment |  |  |
| qv | Print a menu of the four space views, for buying lasers |  |  |
| refund | Install a new laser, processing a refund if applicable | sin-verificar | `laser_refund` |
| PRXS | Equipment prices | sin-verificar | `equip_price` |

## Flight (0/29)

| Rutina | Resumen | Estado | C |
|---|---|---|---|
| DOENTRY | Dock at the space station and work out any mission progression |  |  |
| ESCAPE | Launch our escape pod |  |  |
| DOCKIT | Apply docking manoeuvres to the ship in INWK |  |  |
| DCS1 | Calculate the vector from the ideal docking position to the ship |  |  |
| SESCP | Spawn an escape pod from the current (parent) ship |  |  |
| dockEd | Print a message to say there is no hyperspacing allowed inside the |  |  |
| hyp | Start the hyperspace process |  |  |
| wW | Start a hyperspace countdown |  |  |
| TTX110 | Set the current system to the nearest system and return to hyp |  |  |
| Ghy | Perform a galactic hyperspace jump |  |  |
| ee3 | Print the hyperspace countdown in the top-left of the screen |  |  |
| TT147 | Print an error when a system is out of hyperspace range |  |  |
| MJP | Process a mis-jump into witchspace |  |  |
| TT18 | Try to initiate a jump into hyperspace |  |  |
| TT110 | Launch from a station or show the front space view |  |  |
| SHD | Charge a shield and drain some energy from the energy banks |  |  |
| DENGY | Drain some energy from the energy banks |  |  |
| OOPS | Take some damage |  |  |
| me2 | Remove an in-flight message from the space view |  |  |
| WARP | Perform an in-system jump |  |  |
| me1 | Erase an old in-flight message and display a new one |  |  |
| MESS | Display an in-flight message |  |  |
| mes9 | Print a text token, possibly followed by " DESTROYED" |  |  |
| OUCH | Potentially lose cargo or equipment following damage |  |  |
| ou2 | Display "E.C.M.SYSTEM DESTROYED" as an in-flight message |  |  |
| ou3 | Display "FUEL SCOOPS DESTROYED" as an in-flight message |  |  |
| PLUT | Flip the coordinate axes for the four different views |  |  |
| LOOK1 | Initialise the space view |  |  |
| SIGHT | Draw the laser crosshairs |  |  |

## Keyboard (1/19)

| Rutina | Resumen | Estado | C |
|---|---|---|---|
| TGINT | The keys used to toggle configuration settings when the game is |  |  |
| FLKB | Flush the keyboard buffer |  |  |
| PAUSE2 | Wait until a key is pressed, ignoring any existing key press |  |  |
| TT214 | Ask a question with a "Y/N?" prompt and return the response |  |  |
| YESNO | Wait until either "Y" or "N" is pressed |  |  |
| TT17 | Scan the keyboard for cursor key or joystick movement |  |  |
| TT102 | Process number key, save key, hyperspace and chart key presses | propia | `ui_nav` |
| ZEKTRAN | Clear the key logger |  |  |
| RDS1 | Scan one axis of the joystick and return its position |  |  |
| RDKEY | Scan the keyboard for key presses and the joystick, and update the |  |  |
| KYTB | Lookup table for in-flight keyboard controls |  |  |
| DKSANYKEY | An unused routine that scans the keyboard to detect whether any |  |  |
| DKS2 | Read the joystick position |  |  |
| DKS3 | Toggle a configuration setting and emit a beep |  |  |
| U% | Clear the key logger |  |  |
| DOKEY | Scan for the seven primary flight controls and apply the docking |  |  |
| DK4 | Scan for pause, configuration and secondary flight keys |  |  |
| TT217 | Scan the keyboard until a key is pressed |  |  |
| KTRAN | An unused key logger buffer that's left over from the 6502 Second |  |  |

## Loader (0/5)

| Rutina | Resumen | Estado | C |
|---|---|---|---|
| ENTRY | An entry point at the start of the game binary |  |  |
| S% | Checksum, decrypt and unscramble the main game code, and start the |  |  |
| BEGIN | Initialise the configuration variables and start the game |  |  |
| COLD | Initialise the screen mode, clear memory and set up interrupt |  |  |
| NMIpissoff | Acknowledge NMI interrupts and ignore them |  |  |

## Main loop (0/1)

| Rutina | Resumen | Estado | C |
|---|---|---|---|
| Main | Seed the random number generator |  |  |

## Market (6/19)

| Rutina | Resumen | Estado | C |
|---|---|---|---|
| tnpr1 | Work out if we have space for one tonne of cargo |  |  |
| tnpr | Work out if we have space for a specific amount of cargo | sin-verificar | `trade_room` |
| TT219 | Show the Buy Cargo screen | sin-verificar | `trade_buy` |
| gnum | Get a number from the keyboard |  |  |
| NWDAV4 | Print an "ITEM?" error, make a beep and rejoin the TT210 routine |  |  |
| TT208 | Show the Sell Cargo screen | sin-verificar | `trade_sell` |
| TT210 | Show a list of current cargo in our hold, optionally to sell |  |  |
| TT213 | Show the Inventory screen | sin-verificar | `scr_inventory` |
| TT151 | Print the name, price and availability of a market item | sin-verificar | `trade_price` |
| TT152 | Print the unit ("t", "kg" or "g") for a market item |  |  |
| TT160 | Print "t" (for tonne) and a space |  |  |
| TT161 | Print "kg" (for kilograms) |  |  |
| TT16a | Print "g" (for grams) |  |  |
| TT163 | Print the headers for the table of market prices |  |  |
| TT167 | Show the Market Price screen | sin-verificar | `scr_market` |
| var | Calculate QQ19+3 = economy * |economic_factor| |  |  |
| dn | Print the amount of money we have left in the cash pot, then make |  |  |
| ITEM | Macro definition for the market prices table |  |  |
| QQ23 | Market prices table |  |  |

## Maths (Arithmetic) (34/52)

| Rutina | Resumen | Estado | C |
|---|---|---|---|
| log | Binary logarithm table (high byte) |  |  |
| logL | Binary logarithm table (low byte) |  |  |
| alogh | Binary antilogarithm table |  |  |
| PIX1 | Calculate (YY+1 SYL+Y) = (A P) + (S R) and draw stardust particle |  |  |
| MAS3 | Calculate A = x_hi^2 + y_hi^2 + z_hi^2 in the K% block | transcripcion | `sm_mas3` |
| VCSU1 | Calculate vector K3(8 0) = [x y z] - coordinates of the sun or | transcripcion | `sm_vcsu1` |
| VCSUB | Calculate vector K3(8 0) = [x y z] - coordinates in (A V) | transcripcion | `sm_vcsub` |
| TAS1 | Calculate K3 = (x_sign x_hi x_lo) - V(1 0) | transcripcion | `sm_tas1` |
| MU5 | Set K(3 2 1 0) = (A A A A) and clear the C flag |  |  |
| MULT3 | Calculate K(3 2 1 0) = (A P+1 P) * Q | transcripcion | `em_mult3` |
| MLS2 | Calculate (S R) = XX(1 0) and (A P) = A * ALP1 |  |  |
| MLS1 | Calculate (A P) = ALP1 * A |  |  |
| MU6 | Set P(1 0) = (A A) |  |  |
| SQUA | Clear bit 7 of A and calculate (A P) = A * A | transcripcion | `em_squa` |
| SQUA2 | Calculate (A P) = A * A | transcripcion | `em_squa2` |
| MU1 | Copy X into P and A, and clear the C flag |  |  |
| MLU1 | Calculate Y1 = y_hi and (A P) = |y_hi| * Q for Y-th stardust | transcripcion | `em_mlu1` |
| MLU2 | Calculate (A P) = |A| * Q | transcripcion | `em_mlu2` |
| MULTU | Calculate (A P) = P * Q | transcripcion | `em_multu` |
| MU11 | Calculate (A P) = P * X | transcripcion | `em_mu11` |
| FMLTU2 | Calculate A = K * sin(A) | transcripcion | `em_fmltu2` |
| FMLTU | Calculate A = A * Q / 256 | verificada | `em_fmltu` |
| MLTU2 | Calculate (A P+1 P) = (A ~P) * Q | transcripcion | `em_mltu2` |
| MUT3 | An unused routine that does the same as MUT2 |  |  |
| MUT2 | Calculate (S R) = XX(1 0) and (A P) = Q * A |  |  |
| MUT1 | Calculate R = XX and (A P) = Q * A |  |  |
| MULT1 | Calculate (A P) = Q * A | transcripcion | `em_mult1` |
| MULT12 | Calculate (S R) = Q * A | transcripcion | `em_mult12` |
| MAD | Calculate (A X) = Q * A + (S R) | transcripcion | `em_mad` |
| ADD | Calculate (A X) = (A P) + (S R) | transcripcion | `em_add` |
| TIS1 | Calculate (A ?) = (-X * A + (S R)) / 96 | transcripcion | `em_tis1` |
| DV42 | Calculate (P R) = 256 * DELTA / z_hi | transcripcion | `em_dv42` |
| DV41 | Calculate (P R) = 256 * DELTA / A | transcripcion | `em_dv41` |
| DVID4 | Calculate (P R) = 256 * A / Q | transcripcion | `em_dvid4` |
| DVID3B2 | Calculate K(3 2 1 0) = (A P+1 P) / (z_sign z_hi z_lo) |  |  |
| LCASH | Subtract an amount of cash from the cash pot | transcripcion | `sm_lcash` |
| MCASH | Add an amount of cash to the cash pot | transcripcion | `sm_mcash` |
| GCASH | Calculate (Y X) = P * Q * 4 | transcripcion | `sm_gcash` |
| GC2 | Calculate (Y X) = (A P) * 4 |  |  |
| SPS2 | Calculate (Y X) = A / 10 | transcripcion | `sm_sps2` |
| DORND | Generate random numbers | verificada | `em_dornd` |
| TIS2 | Calculate A = A / Q | transcripcion | `em_tis2` |
| TIS3 | Calculate -(nosev_1 * roofv_1 + nosev_2 * roofv_2) / nosev_3 | transcripcion | `sm_tis3` |
| DVIDT | Calculate (P+1 A) = (A P) / Q | transcripcion | `em_dvidt` |
| LL5 | Calculate Q = SQRT(R Q) | transcripcion | `em_ll5` |
| LL28 | Calculate R = 256 * A / Q | verificada | `em_ll28` |
| LL38 | Calculate (S A) = (S R) + (A Q) | transcripcion | `em_ll38` |
| LL61 | Calculate (U R) = 256 * A / Q | verificada | `em_ll61` |
| LL62 | Calculate 128 - (U R) |  |  |
| LL120 | Calculate (Y X) = (S x1_lo) * XX12+2 or (S x1_lo) / XX12+2 |  |  |
| LL123 | Calculate (Y X) = (S R) / XX12+2 or (S R) * XX12+2 |  |  |
| LL129 | Calculate Q = XX12+2, A = S EOR XX12+3 and (S R) = |S R| |  |  |

## Maths (Geometry) (17/19)

| Rutina | Resumen | Estado | C |
|---|---|---|---|
| MAS1 | Add an orientation vector coordinate to an INWK coordinate | transcripcion | `sm_mas1` |
| MAS2 | Calculate a cap on the maximum distance to the planet or sun | transcripcion | `sm_mas2` |
| TAS4 | Calculate the dot product of XX15 and one of the space station's | transcripcion | `sm_tas4` |
| TAS6 | Negate the vector in XX15 so it points in the opposite direction | transcripcion | `sm_tas6` |
| TAS3 | Calculate the dot product of XX15 and an orientation vector | transcripcion | `sm_tas3` |
| SCALEY | Scale the y-coordinate in A to 0.375 * A | transcripcion | `sm_scaley` |
| SCALEY2 | Scale the y-coordinate in A to 0.75 * A | transcripcion | `sm_scaley2` |
| SCALEX | Scale the x-coordinate in A to 32 + 0.75 * A | transcripcion | `sm_scalex` |
| SPS4 | Calculate the vector to the space station | transcripcion | `sm_sps4` |
| SPS3 | Copy a space coordinate from the K% block into K3 | transcripcion | `sm_sps3` |
| PROJ | Project the current ship or planet onto the screen | verificada | `sm_proj` |
| FAROF | Compare x_hi, y_hi and z_hi with 224 |  |  |
| FAROF2 | Compare x_hi, y_hi and z_hi with A |  |  |
| MAS4 | Calculate a cap on the maximum distance to a ship | transcripcion | `sm_mas4` |
| SPS1 | Calculate the vector to the planet and store it in XX15 | transcripcion | `sm_sps1` |
| TAS2 | Normalise the three-coordinate vector in K3 | transcripcion | `sm_tas2` |
| NORM | Normalise the three-coordinate vector in XX15 | transcripcion | `sm_norm` |
| TIDY | Orthonormalise the orientation vectors for a ship | transcripcion | `sm_tidy` |
| LL51 | Calculate the dot product of XX15 and XX16 | verificada | `ship_ll51` |

## Missions (0/16)

| Rutina | Resumen | Estado | C |
|---|---|---|---|
| TRIBDIR | The low byte of the four 16-bit directions in which Trumble |  |  |
| TRIBDIRH | The high byte of the four 16-bit directions in which Trumble |  |  |
| SPMASK | Masks for updating sprite bits in VIC+$10 for the top bit of the |  |  |
| BRIEF2 | Start mission 2 |  |  |
| BRP | Print an extended token and show the Status Mode screen |  |  |
| BRIEF3 | Receive the briefing and plans for mission 2 |  |  |
| DEBRIEF2 | Finish mission 2 |  |  |
| DEBRIEF | Finish mission 1 |  |  |
| TBRIEF | Start mission 3 |  |  |
| BRIEF | Start mission 1 and show the mission briefing |  |  |
| BRIS | Clear the screen, display "INCOMING MESSAGE" and wait for 2 |  |  |
| PAUSE | Wait until a key is pressed for the Constrictor mission briefing |  |  |
| PAS1 | Change to the text view for the Constrictor mission briefing |  |  |
| THERE | Check whether we are in the Constrictor's system in mission 1 |  |  |
| TRIBTA | A table for converting the number of Trumbles in the hold into a |  |  |
| TRIBMA | A table for converting the number of Trumbles in the hold into a |  |  |

## Moving (1/8)

| Rutina | Resumen | Estado | C |
|---|---|---|---|
| MVT3 | Calculate K(3 2 1) = (x_sign x_hi x_lo) + K(3 2 1) | transcripcion | `sm_mvt3` |
| MVS5 | Apply a 3.6 degree pitch or roll to an orientation vector |  |  |
| SFS2 | Move a ship in space along one of the coordinate axes |  |  |
| MVEIT | Move current ship: Tidy the orientation vectors |  |  |
| MVT1 | Calculate (x_sign x_hi x_lo) = (x_sign x_hi x_lo) + (A R) |  |  |
| MVS4 | Apply pitch and roll to an orientation vector |  |  |
| MVT6 | Calculate (A P+2 P+1) = (x_sign x_hi x_lo) + (A P+2 P+1) |  |  |
| MV40 | Rotate the planet or sun's location in space by the amount of |  |  |

## Save and load (5/61)

| Rutina | Resumen | Estado | C |
|---|---|---|---|
| wtable | 6-bit to 7-bit nibble conversion table |  |  |
| BRKBK | Set the standard BRKV handler for the game |  |  |
| NA% | The data block for the last saved commander |  |  |
| CHK2 | Second checksum byte for the saved commander data file |  |  |
| CHK3 | Third checksum byte for the saved commander data file |  |  |
| CHK | First checksum byte for the saved commander data file |  |  |
| S1% | The drive and directory number used when saving or loading a |  |  |
| NA2% | The data block for the default commander |  |  |
| CHECK | Calculate the checksum for the last saved commander data block | transcripcion | `cmdr_check` |
| CHECK2 | Calculate the third checksum for the last saved commander data | transcripcion | `cmdr_check2` |
| JAMESON | Restore the default JAMESON commander | sin-verificar | `cmdr_default` |
| COPYNAME | Copy the last saved commander's name from INWK+5 to comnam and pad |  |  |
| TRNME | Copy the last saved commander's name from INWK to NA% |  |  |
| TR1 | Copy the last saved commander's name from NA% to INWK |  |  |
| GTNMEW | Fetch the name of a commander file to save or load |  |  |
| FILEPR | Display the currently selected media (disk or tape) |  |  |
| OTHERFILEPR | Display the non-selected media (disk or tape) |  |  |
| SVE | Display the disk access menu and process saving of commander files | propia | `scr_disk` |
| diskerror | Print a disk error, make a beep and wait for a key press |  |  |
| thislong | Contains the length of the most recently entered commander name |  |  |
| oldlong | Contains the length of the last saved commander name |  |  |
| LOD | Load a commander file | propia | `load_commander` |
| DERR1 | The error message for when a disk is write protected |  |  |
| DERR2 | The error message for when a disk is full |  |  |
| DERR3 | The error message for when a disk catalog is full |  |  |
| DERR4 | The error message for when there is a disk I/O error |  |  |
| DERR5 | The error message for when a file is not found |  |  |
| ERTAB | A lookup table for the five disk error messages |  |  |
| comnam | Storage for the commander filename, padded out with spaces to a |  |  |
| rfile | Read a commander file from a DOS disk into the buffer |  |  |
| wfile | Write a commander file from the buffer to a DOS disk |  |  |
| findf | Search the disk catalog for an existing file |  |  |
| finde | Search the disk catalog for an empty file entry |  |  |
| rentry | Search the disk catalog for an existing file or an empty file |  |  |
| getsct | Analyse the VTOC sector to allocate one free sector |  |  |
| isfull | Check the disk to ensure there are least two free sectors, one for |  |  |
| gettsl | Read a file's track/sector list |  |  |
| rvtoc | Read the VTOC sector into the buffer |  |  |
| rsect | Read a specific sector from disk into the buffer |  |  |
| wsect | Write a specific sector from the buffer to disk |  |  |
| rwts | Read or write a specific sector |  |  |
| trytrk | Try finding a specific track on the disk |  |  |
| rdrght | Check that this is the correct track |  |  |
| prterr | Return from the RWTS code with a "Disk write protected" error |  |  |
| rttrk | Read or write a sector on the current track |  |  |
| drverr | Return from the RWTS code with a "Disk I/O error" |  |  |
| rttrk3 | Successfully return from the RWTS code with no error reported |  |  |
| read | Read a sector's worth of data into the buffr2 buffer |  |  |
| write | Write a sector's worth of data from the buffr2 buffer to the |  |  |
| rdaddr | Read a track address field |  |  |
| seek | Fast seek routine |  |  |
| armwat | Implement the arm move delay |  |  |
| armtab | Phase-on time table in 100-usec intervals |  |  |
| armtb2 | Phase-off time table in 100-usec intervals |  |  |
| prenib | Convert 256 8-bit bytes in buffer into 342 6-bit nibbles in buffr2 |  |  |
| pstnib | Convert 342 6-bit nibbles in buffr2 into 256 8-bit bytes in buffer |  |  |
| wbyte | Write one byte to disk |  |  |
| scttab | Lookup table to translate logical (requested) sector number to |  |  |
| rtable | 64 disk nibbles of "6-and-2" Read Translate Table |  |  |
| MUTILATE | Encrypt the commander file in the buffer at comfil |  |  |
| UNMUTILATE | Decrypt the commander file in the buffer at comfil |  |  |

## Sound (0/13)

| Rutina | Resumen | Estado | C |
|---|---|---|---|
| BELL | Make a standard system beep |  |  |
| EXNO | Make the sound of a laser strike on another ship |  |  |
| BOOP | Make a long, low beep |  |  |
| SOHISS | Make the sound of a launch from the station, hyperspace or missile |  |  |
| EXNO3 | Make the sound of a collision, or an exploding cargo canister or |  |  |
| SOEXPL | Make an explosion sound |  |  |
| BEEP | Make a short, high beep |  |  |
| SOBEEP | Make a beep |  |  |
| SOBLIP | Make the sound of the hyperspace drive being engaged, or the sound |  |  |
| LASNOISE | Make the sound of our laser firing or the sound of us being hit by |  |  |
| LASNOISE2 | An unused routine that makes a different laser sound |  |  |
| SOBOMB | Make the sound of an energy bomb going off |  |  |
| CLICK | Toggle the state of the speaker (i.e. move it in or out) to make a |  |  |

## Stardust (0/7)

| Rutina | Resumen | Estado | C |
|---|---|---|---|
| FLIP | Reflect the stardust particles in the screen diagonal and redraw |  |  |
| STARS | The main routine for processing the stardust |  |  |
| STARS1 | Process the stardust for the front view |  |  |
| STARS6 | Process the stardust for the rear view |  |  |
| STARS2 | Process the stardust for the left or right view |  |  |
| NWSTARS | Initialise the stardust field |  |  |
| nWq | Create a random cloud of stardust |  |  |

## Start and end (1/8)

| Rutina | Resumen | Estado | C |
|---|---|---|---|
| RESET | Reset most variables |  |  |
| RES2 | Reset a number of flight variables and workspaces |  |  |
| DEATH | Display the death screen |  |  |
| TT170 | Main entry point for the Elite game code |  |  |
| DEATH2 | Reset most of the game and restart from the title screen |  |  |
| BR1 | Show the "Load New Commander (Y/N)?" screen and start the game |  |  |
| DFAULT | Reset the current commander data block to the last saved commander | sin-verificar | `cmdr_unpack` |
| TITLE | Display a title screen with a rotating ship and prompt |  |  |

## Status (1/7)

| Rutina | Resumen | Estado | C |
|---|---|---|---|
| STATUS | Show the Status Mode screen | sin-verificar | `scr_status` |
| cmn | Print the commander's name |  |  |
| fwl | Print fuel and cash levels |  |  |
| csh | Print the current amount of cash |  |  |
| BAD | Calculate how bad we have been |  |  |
| BAY | Go to the docking bay (i.e. show the Status Mode screen) |  |  |
| EXNO2 | Process us making a kill |  |  |

## Tactics (0/8)

| Rutina | Resumen | Estado | C |
|---|---|---|---|
| TACTICS | Apply tactics: Process missiles, both enemy missiles and our own |  |  |
| HITCH | Work out if the ship in INWK is in our crosshairs |  |  |
| FRS1 | Launch a ship straight ahead of us, below the laser sights |  |  |
| FRMIS | Fire a missile from our ship |  |  |
| ANGRY | Make a ship or station hostile, and if this is a ship then enable |  |  |
| FR1 | Display the "missile jammed" message |  |  |
| yetanotherrts | Contains an RTS |  |  |
| SFRMIS | Add an enemy missile to our local bubble of universe |  |  |

## Text (0/77)

| Rutina | Resumen | Estado | C |
|---|---|---|---|
| MT27 | Print the captain's name during mission briefings |  |  |
| MT28 | Print the location hint during the mission 1 briefing |  |  |
| DETOK3 | Print an extended recursive token from the RUTOK token table |  |  |
| DETOK | Print an extended recursive token from the TKN1 token table |  |  |
| DETOK2 | Print an extended text token (1-255) |  |  |
| MT1 | Switch to ALL CAPS when printing extended tokens |  |  |
| MT2 | Switch to Sentence Case when printing extended tokens |  |  |
| MT8 | Tab to column 6 and start a new word when printing extended tokens |  |  |
| MT9 | Clear the screen and set the current view type to 1 |  |  |
| MT13 | Switch to lower case when printing extended tokens |  |  |
| MT6 | Switch to standard tokens in Sentence Case |  |  |
| MT5 | Switch to extended tokens |  |  |
| MT14 | Switch to justified text when printing extended tokens |  |  |
| MT15 | Switch to left-aligned text when printing extended tokens |  |  |
| MT17 | Print the selected system's adjective, e.g. Lavian for Lave |  |  |
| MT18 | Print a random 1-8 letter word in Sentence Case |  |  |
| MT19 | Capitalise the next letter |  |  |
| VOWEL | Test whether a character is a vowel |  |  |
| WHITETEXT | Switch to white text |  |  |
| JMTB | The extended token table for jump tokens 1-32 (DETOK) |  |  |
| TKN2 | The extended two-letter token lookup table |  |  |
| QQ16 | The two-letter token lookup table |  |  |
| plf2 | Print text followed by a newline and indent of 6 characters |  |  |
| TENS | A constant used when printing large numbers in BPRNT |  |  |
| pr2 | Print an 8-bit number, left-padded to 3 digits, and optional point |  |  |
| TT11 | Print a 16-bit number, left-padded to n digits, and optional point |  |  |
| BPRNT | Print a 32-bit number, left-padded to a specific number of digits, |  |  |
| DTW1 | A mask for applying the lower case part of Sentence Case to |  |  |
| DTW2 | A flag that indicates whether we are currently printing a word |  |  |
| DTW3 | A flag for switching between standard and extended text tokens |  |  |
| DTW4 | Flags that govern how justified extended text tokens are printed |  |  |
| DTW5 | The size of the justified text buffer at BUF |  |  |
| DTW6 | A flag to denote whether printing in lower case is enabled for |  |  |
| DTW8 | A mask for capitalising the next letter in an extended text token |  |  |
| FEED | Print a newline |  |  |
| MT16 | Print the character in variable DTW7 |  |  |
| TT26 | Print a character at the text cursor, with support for verified |  |  |
| MT23 | Move to row 9 and switch to lower case |  |  |
| MT29 | Move to row 5 and switch to lower case when printing extended |  |  |
| MTIN | Lookup table for random tokens in the extended token table (0-37) |  |  |
| DOXC | Move the text cursor to a specific column |  |  |
| DOYC | Move the text cursor to a specific row |  |  |
| INCYC | Move the text cursor to the next row |  |  |
| TT60 | Print a text token and a paragraph break |  |  |
| TTX69 | Print a paragraph break |  |  |
| TT69 | Set Sentence Case and print a newline |  |  |
| TT67 | Print a newline |  |  |
| spc | Print a text token followed by a space |  |  |
| pr6 | Print 16-bit number, left-padded to 5 digits, no point |  |  |
| pr5 | Print a 16-bit number, left-padded to 5 digits, and optional point |  |  |
| prq | Print a text token followed by a question mark |  |  |
| TT162 | Print a space |  |  |
| dn2 | Make a low, long beep and delay for 0.5 seconds |  |  |
| plf | Print a text token followed by a newline |  |  |
| TT68 | Print a text token followed by a colon |  |  |
| TT73 | Print a colon |  |  |
| TT27 | Print a text token |  |  |
| TT42 | Print a letter in lower case |  |  |
| TT41 | Print a letter according to Sentence Case |  |  |
| qw | Print a recursive token in the range 128-145 |  |  |
| crlf | Tab to column 21 and print a colon |  |  |
| TT45 | Print a letter in lower case |  |  |
| TT46 | Print a character and switch to capitals |  |  |
| TT74 | Print a character |  |  |
| TT43 | Print a two-letter token or recursive token 0-95 |  |  |
| ex | Print a recursive token |  |  |
| MT26 | Fetch a line of text from the keyboard |  |  |
| RLINE | The OSWORD configuration block used to fetch a line of text from |  |  |
| CHPR2 | Character print vector handler |  |  |
| R5 | Make a beep and jump back into the character-printing routine at |  |  |
| RR5 | Print a character in the text screen mode |  |  |
| TT67K | Print a newline |  |  |
| CHPR | Print a character at the text cursor by poking into screen memory |  |  |
| letter | Draw a character in the high-resolution screen mode using the game |  |  |
| letter2 | Draw a character or indicator bulb bitmap in the high-resolution |  |  |
| SETXC | Move the text cursor to a specific column |  |  |
| SETYC | Move the text cursor to a specific row |  |  |

## Tube (0/1)

| Rutina | Resumen | Estado | C |
|---|---|---|---|
| newosrdch | The custom OSRDCH routine for reading characters |  |  |

## Universe (2/34)

| Rutina | Resumen | Estado | C |
|---|---|---|---|
| SPIN | Randomly spawn cargo from a destroyed ship |  |  |
| UNIV | Table of pointers to the local universe's ship data blocks |  |  |
| SFS1 | Spawn a child ship from the current (parent) ship |  |  |
| PDESC | Print the system's extended description or a mission 1 directive |  |  |
| GINF | Fetch the address of a ship's data block into INF |  |  |
| ping | Set the selected system to the current system |  |  |
| TT20 | Twist the selected system's seeds four times |  |  |
| TT54 | Twist the selected system's seeds |  |  |
| TT146 | Print the distance to the selected system in light years |  |  |
| TT70 | Display "MAINLY " and jump to TT72 |  |  |
| TT25 | Show the Data on System screen | sin-verificar | `scr_data` |
| TT24 | Calculate system data from the system seeds |  |  |
| TT81 | Set the selected system's seeds to those of system 0 |  |  |
| TT111 | Set the current system to the nearest system to a point | sin-verificar | `uni_nearest` |
| jmp | Set the current system to the selected system |  |  |
| hyp1 | Process a jump to the system closest to (QQ9, QQ10) |  |  |
| GVL | Calculate the availability of market items |  |  |
| GTHG | Spawn a Thargoid ship and a Thargon companion |  |  |
| cpl | Print the selected system name |  |  |
| ypl | Print the current system name |  |  |
| tal | Print the current galaxy number |  |  |
| SOS1 | Update the missile indicators, set up the planet data block |  |  |
| SOLAR | Set up various aspects of arriving in a new system |  |  |
| NWSPS | Add a new space station to our local bubble of universe |  |  |
| NWSHP | Add a new ship to our local bubble of universe |  |  |
| NwS1 | Flip the sign and double an INWK byte |  |  |
| KS3 | Set the SLSP ship line heap pointer after shuffling ship slots |  |  |
| KS1 | Remove the current ship from our local bubble of universe |  |  |
| KS4 | Remove the space station and replace it with the sun |  |  |
| KS2 | Check the local bubble for missiles with target lock |  |  |
| KILLSHP | Remove a ship from our local bubble of universe |  |  |
| ZINF | Reset the INWK workspace and orientation vectors |  |  |
| Ze | Initialise the INWK workspace to a fairly aggressive ship |  |  |
| spasto | Contains the address of the Coriolis space station's ship |  |  |

## Utility routines (0/18)

| Rutina | Resumen | Estado | C |
|---|---|---|---|
| DEEOR | Unscramble the main code |  |  |
| DEEORS | Decrypt a multi-page block of memory |  |  |
| G% | Denotes the start of the main game code, from ELITE A to ELITE K |  |  |
| DELAY | Wait for a specified time, measured in loop iterations |  |  |
| SWAPPZERO | An unused routine that swaps bytes in and out of zero page |  |  |
| brkd | A flag that indicates whether a system error has occurred |  |  |
| BRBR | The standard BRKV handler for the game |  |  |
| ZERO | Reset the local bubble of universe and ship status |  |  |
| ZEBC | Zero-fill pages $B and $C |  |  |
| ZES1 | Zero-fill the page whose number is in X |  |  |
| ZES2 | Zero-fill a specific page |  |  |
| backtonormal | Disable the keyboard, set the SVN flag to 0, and return with A = 0 |  |  |
| CLDELAY | Delay by iterating through 5 * 256 (1280) empty loops |  |  |
| ZES1k | Zero-fill the page whose number is in X |  |  |
| ZES2k | Zero-fill a specific page |  |  |
| ZESNEW | Zero-fill memory from SC(1 0) to the end of the page |  |  |
| mvblockK | Copy a specific number of pages in memory |  |  |
| F% | Denotes the end of the main game code, from ELITE A to ELITE K |  |  |

## Workspaces (0/7)

| Rutina | Resumen | Estado | C |
|---|---|---|---|
| ZP | Lots of important variables are stored in the zero page workspace |  |  |
| XX3 | Temporary storage space for complex calculations |  |  |
| K% | Ship data blocks and ship line heaps |  |  |
| Disk | Variables used by the disk operations and DOS 3.3 RWTS routines |  |  |
| UP | Configuration variables |  |  |
| WP | Variables |  |  |
| Option | Variables that are predominantly used to store the game options |  |  |
