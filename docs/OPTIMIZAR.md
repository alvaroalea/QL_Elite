# Optimización del vuelo (análisis y mediciones)

Estado a 2026-10-25. Pendiente de continuar. El análisis inicial se hizo leyendo el código; las mediciones son del usuario en el QL real con la compilación DEBUG (`make elite`).

## 1. Qué hace un fotograma de vuelo (`flight_loop`, `flight.c`)

1. `in_poll`: 4 lecturas `KEYROW` por IPC (`MT.IPCOM`), un `IO.FBYTE` (`plat_flush_input`) y espera al siguiente cuadro de 50 Hz (`plat_present`).
2. `ctl_read`, `space_controls`, `laser_fire`.
3. `space_step` → `ships_step`: por cada ranura `TACTICS`/`MVEIT`, `combat_collide`, `HITCH` (`combat_hit`), `ship_ll9` (proyección y líneas, ensamblador), radar (`space_scan`); luego `energy_tick`, `ships_part14`, `damage_checks`.
4. `bomb_frame`, `ecm_frame`, `laser_frame_end`, `stars_update` (12 partículas, `DV42`/`MLU1/2` transcritos), mensajes y ayuda.
5. `dash_update` (incremental) y `compas` (vector unitario con `sm_sps1/3`), `chart_sight_update` (mira; con la estación cerca decide verde/azul con `station_aligned`).

El tope de 10 FPS (`FRAME_TICKS = 5`) solo actúa si el fotograma dura menos de 100 ms; en el QL real no influye.

## 2. Mediciones (SHIFT-F muestra FPS, SHIFT-D cicla lo que se salta)

`SHIFT-D` (8 estados): bit 0 = matemáticas de `LL9` (`ship_ll9`), bit 1 = dibujo de líneas de las naves (`default_loin`), bit 2 = pintado de las estrellas (se calculan pero no se pintan; `stars_dbg_nodraw`). Resolución de las cifras: 0,1 FPS en ventanas de 1 s; diferencias de menos de ~10 ms son ruido.

| Modo | Planeta FPS (ms) | Estación FPS (ms) |
|---|---|---|
| NONE | 2,7 (370) | 5,0 (200) |
| MATHS | 7,5 (133) | 5,8 (172) |
| DRAWING | 2,7 (370) | 6,1 (164) |
| MATHS+DRAWING | 7,4 (135) | 7,5 (133) |
| STARS | 2,8 (357) | 5,1 (196) |
| STARS+MATHS | 8,0 (125) | 5,8 (172) |
| STARS+DRAWING | 2,8 (357) | 5,8 (172) |
| ALL | 8,1 (123) | 6,6 (152) |

Anomalías (ruido o escena cambiante): en la estación `ALL` (6,6) sale peor que `MATHS+DRAWING` (7,5) y `STARS+DRAWING` peor que `DRAWING`.

## 3. Conclusiones

- **Mirando al planeta, el planeta cuesta ~235 ms de 370 (64 %).** `SKIP: MATHS` salta `ship_ll9` para todos los objetos, planeta incluido (en `ships.c` solo se repintan las naves, no los tipos con bit 7); `SKIP: DRAWING` no lo afecta porque el planeta se dibuja con elipses, no con `default_loin`. El coste está en el cálculo del planeta y en la elipse (`sun.c`, `ql_ellrows.asm`); los interruptores actuales no separan cálculo y elipse.
- **Las estrellas cuestan 4–13 ms (2–4 %)**: no merecen optimizarse (descartado).
- **Estación:** matemáticas de `LL9` ~28 ms, líneas de las naves ~36 ms, juntas ~67 ms. El `LL9` en ensamblador ya hecho no es el problema.
- **Suelo de ~125–150 ms (7–8 FPS) con todo saltado.** No es dibujo: con `SKIP: ALL` siguen ejecutándose `mveit`/`TACTICS` de cada objeto, `combat_collide`, `HITCH`, el radar, `energy_tick`, `damage_checks`, láser/ECM/bomba, `stars_update`, `dash_update`, `compas`, `chart_sight_update`, el teclado por IPC y el sonido. Prueba: con `ALL` mirando a la estación la mira pasaba de azul a verde, porque `station_aligned` lee `nosev`/`roofv` de la ranura 1 y `dash_comz`, que `mveit` y `compas` siguen actualizando. Sin reducir este suelo no se pasa de ~8 FPS aunque el dibujo fuera gratis (y ya está por encima del tope de 10 FPS).
- Con la escena de la estación el suelo es unos 30 ms mayor que mirando al planeta (más lógica de la estación y su entorno).

## 4. Candidatos y siguientes pasos

| # | Acción | Efecto esperado | Coste |
|---|---|---|---|
| 0 | **Ampliar `SHIFT-D`** con: (a) planeta/sol: saltar solo el cálculo o solo la elipse/relleno; (b) saltar `mveit`/`TACTICS`/colisiones/`HITCH` sin tocar el dibujo (cuidado: el mundo congelado no debe romper el XOR); (c) saltar `dash_update`/`compas`/mira; (d) teclado por IPC. | Reparte el suelo de ~125 ms y el coste del planeta | Bajo |
| 1 | **Planeta y sol:** pasar a ensamblador lo que siga en C de `sun.c` (relleno, `SUN`), o elipses más simples cuando es pequeño. | Alto con el planeta cerca (hasta 370 → ~135 ms) | Medio-alto |
| 2 | **Lógica de simulación** (`MVEIT`, `TACTICS`, `HITCH`, `ships_step`, `load_slot`/`store_slot`): pasar a ensamblador lo que salga en el perfil (verificar contra el 6502 cada rutina convertida); revisar el código de c68 `-O` (acarreos globales `em_carry`, funciones por byte). | Medio-alto sobre el suelo | Alto |
| 3 | **No repintar lo que no cambia:** reutilizar el montón de líneas de naves lejanas o quietas y borrar/pintar solo la diferencia (como `gfx_ell_update`). Respetar la regla del XOR. | Medio con varias naves | Medio |
| 4 | **Teclado:** menos filas por fotograma o `KEYROW` cada 2 fotogramas (ver `docs/PENDIENTES.md`, fila 22, prioridad A). | Pequeño pero constante | Bajo |
| 5 | **Modo 4:** comparar FPS con y sin él (las líneas largas llevan el doble de píxeles físicos). | Por medir | Bajo |
| 6 | **Opcional visual:** menos polvo (`NOST`) o no dibujar naves lejanas. | Medio | Bajo |
| — | Estrellas | 4–13 ms: descartado | — |

Mantener la fidelidad con el 6502: lo que se pase a ensamblador debe reproducir los mismos acarreos y estados (ver `CLAUDE.md`, sección 2.1).
