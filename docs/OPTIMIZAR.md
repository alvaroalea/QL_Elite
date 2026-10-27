# Flight optimisation (analysis and measurements)

State as of 2026-10-25. To be continued. The initial analysis was done by reading the code; the measurements are the user's on a real QL with the DEBUG build (`make elite`).

## 1. What a flight frame does (`flight_loop`, `flight.c`)

1. `in_poll`: 4 `KEYROW` reads through the IPC (`MT.IPCOM`), one `IO.FBYTE` (`plat_flush_input`) and a wait for the next 50 Hz tick (`plat_present`).
2. `ctl_read`, `space_controls`, `laser_fire`.
3. `space_step` → `ships_step`: for each slot `TACTICS`/`MVEIT`, `combat_collide`, `HITCH` (`combat_hit`), `ship_ll9` (projection and lines, assembly), radar (`space_scan`); then `energy_tick`, `ships_part14`, `damage_checks`.
4. `bomb_frame`, `ecm_frame`, `laser_frame_end`, `stars_update` (12 particles, transcribed `DV42`/`MLU1/2`), messages and help.
5. `dash_update` (incremental) and `compas` (unit vector with `sm_sps1/3`), `chart_sight_update` (sight; with the station near it decides green/blue with `station_aligned`).

The 10 FPS cap (`FRAME_TICKS = 5`) only acts if the frame lasts less than 100 ms; on the real QL it has no influence.

## 2. Measurements (SHIFT-F shows FPS, SHIFT-D cycles what is skipped)

`SHIFT-D` (8 states): bit 0 = `LL9` maths (`ship_ll9`), bit 1 = ship line drawing (`default_loin`), bit 2 = star painting (calculated but not painted; `stars_dbg_nodraw`). Resolution of the figures: 0.1 FPS in 1 s windows; differences under ~10 ms are noise.

| Mode | Planet FPS (ms) | Station FPS (ms) |
|---|---|---|
| NONE | 2.7 (370) | 5.0 (200) |
| MATHS | 7.5 (133) | 5.8 (172) |
| DRAWING | 2.7 (370) | 6.1 (164) |
| MATHS+DRAWING | 7.4 (135) | 7.5 (133) |
| STARS | 2.8 (357) | 5.1 (196) |
| STARS+MATHS | 8.0 (125) | 5.8 (172) |
| STARS+DRAWING | 2.8 (357) | 5.8 (172) |
| ALL | 8.1 (123) | 6.6 (152) |

Anomalies (noise or changing scene): at the station `ALL` (6.6) is worse than `MATHS+DRAWING` (7.5) and `STARS+DRAWING` worse than `DRAWING`.

## 3. Conclusions

- **Looking at the planet, the planet costs ~235 ms of 370 (64 %).** `SKIP: MATHS` skips `ship_ll9` for all objects, planet included (in `ships.c` only the ships are repainted, not the types with bit 7); `SKIP: DRAWING` does not affect it because the planet is drawn with ellipses, not with `default_loin`. The cost is in the planet calculation and the ellipse (`sun.c`, `ql_ellrows.asm`); the current switches do not separate calculation and ellipse.
- **Stars cost 4–13 ms (2–4 %)**: not worth optimising (discarded).
- **Station:** `LL9` maths ~28 ms, ship lines ~36 ms, together ~67 ms. The `LL9` already in assembly is not the problem.
- **Floor of ~125–150 ms (7–8 FPS) with everything skipped.** It is not drawing: with `SKIP: ALL` the `mveit`/`TACTICS` of each object, `combat_collide`, `HITCH`, the radar, `energy_tick`, `damage_checks`, laser/ECM/bomb, `stars_update`, `dash_update`, `compas`, `chart_sight_update`, keyboard through the IPC and sound keep running. Proof: with `ALL` looking at the station the sight went from blue to green, because `station_aligned` reads `nosev`/`roofv` of slot 1 and `dash_comz`, which `mveit` and `compas` keep updating. Without reducing this floor ~8 FPS cannot be exceeded even if drawing were free (and it is already above the 10 FPS cap).
- With the station scene the floor is about 30 ms higher than looking at the planet (more station logic and surroundings).

## 4. Candidates and next steps

| # | Action | Expected effect | Cost |
|---|---|---|---|
| 0 | **Extend `SHIFT-D`** with: (a) planet/sun: skip only the calculation or only the ellipse/fill; (b) skip `mveit`/`TACTICS`/collisions/`HITCH` without touching drawing (careful: the frozen world must not break the XOR); (c) skip `dash_update`/`compas`/sight; (d) keyboard through the IPC. | Splits the ~125 ms floor and the planet cost | Low |
| 1 | **Planet and sun:** move to assembly whatever remains in C of `sun.c` (fill, `SUN`), or simpler ellipses when small. | High with the planet near (up to 370 → ~135 ms) | Medium-high |
| 2 | **Simulation logic** (`MVEIT`, `TACTICS`, `HITCH`, `ships_step`, `load_slot`/`store_slot`): move to assembly what shows up in the profile (verify each converted routine against the 6502); review c68 `-O` code (global carries `em_carry`, byte-wise functions). | Medium-high on the floor | High |
| 3 | **Do not repaint what does not change:** reuse the line heap of distant or still ships and erase/paint only the difference (like `gfx_ell_update`). Respect the XOR rule. | Medium with several ships | Medium |
| 4 | **Keyboard:** fewer rows per frame or `KEYROW` every 2 frames (see `docs/PENDIENTES.md`, row 22, priority A). | Small but constant | Low |
| 5 | **Mode 4:** compare FPS with and without it (long lines carry twice the physical pixels). | To be measured | Low |
| 6 | **Optional, visual:** less dust (`NOST`) or do not draw distant ships. | Medium | Low |
| — | Stars | 4–13 ms: discarded | — |

Keep fidelity with the 6502: whatever is moved to assembly must reproduce the same carries and states (see `CLAUDE.md`, section 2.1).
