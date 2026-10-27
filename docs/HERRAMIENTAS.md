# Tools and environment

Moved from `CLAUDE.md` (sections 3.2–3.6 and 5.3).

## 3.2 C compiler for the QL (XTC68) — installation if it is not available.
`tools/install_xtc68.sh` automates what follows:
1. `git clone https://github.com/stronnag/xtc68` (migrated to Codeberg, but the GitHub clone works) and `git clone https://github.com/xXorAa/c68-support`.
2. `tools/install_xtc68.sh` installs XTC68 + the c68 runtime in `/tmp/qlsdk` (see 3.2). Then `export PATH=/tmp/qlsdk/bin:$PATH QLINC=/tmp/qlsdk/share/qdos/include QLLIB=/tmp/qlsdk/share/qdos/lib` and
3. modern gcc fails with `-Werror` on the old sources (e.g. "ignoring return value of write"): `sed -i "s/'-Werror', *//g" src/*/meson.build`.
4. `meson setup build --prefix=/tmp/qlsdk --strip && ninja -C build install` → `bin/{qcc,c68,as68,qld,qcpp,qdos-ar,...}`.
5. The c68 runtime (`INCLUDE_*` headers and `LIB_*` libraries, from the c68 runtime 4.24 "424frun1") is in a **`support/424frun1`** folder of the xtc68 repository, and is installed with `./sdk-install.sh /tmp/qlsdk` run **from that folder** → `share/qdos/include` and `share/qdos/lib`.
6. Usage: `export PATH=/tmp/qlsdk/bin:$PATH QLINC=/tmp/qlsdk/share/qdos/include QLLIB=/tmp/qlsdk/share/qdos/lib`; `qcc -O -Isrc -o elite ...` (the `Makefile` already does it with `make elite`). `-Isrc` is mandatory because qcc looks for the `#include "..."` in the current directory. `qcc` prints `elite: dataspace 18114 (46c2)` when it finishes correctly.
7. `as68` is from the Sozobon family: `.globl`/`.text` directives, comments with `;`, labels with colons, `.asm` files (qcc does not recognise `.s`); `xdef`, `section` and comments with `*` are not valid.

## 3.3 Verification of c68 code (`make c68_check`)
`tools/c68_check/run.sh [probe ...]` compiles each probe in `tests/c68/` with gcc (PC) and with the real `c68` (`-int=16 -O -warn=0`, preprocessing with `gcc -E -P -U__GNUC__ -nostdinc`), converts c68's output (ACK syntax) to GNU as with `conv.py`, links it with a mini runtime (`rt.s`: `.Xlmul`, `.Xldiv`, ..., screen output) and runs it under `qemu-m68k`. Each probe leaves hashes on the "screen" and they are compared byte by byte. `XTC68=<dir with c68>` (by default `/tmp/qlsdk/bin` if it exists, otherwise `/tmp/xtc68/build`). With `-U__GNUC__` the "QL" paths are compiled (for example the long words of `gfx_fill_lines`). Probes: `gfxfast, emath, keys, ships, game, charts, dash, flight, session`. **It does not replace the real QL** (it covers neither the runtime nor the hardware), but it detects compiler bugs (it found the `-O` one).

## 3.4 Python tools (`tools/`)
| File | Purpose |
|---|---|
| `cpu6502.py`, `asm6502.py` | 6502 emulator and minimal assembler for the source's syntax (labels, EQUB/EQUW, `P%`, `LO()/HI()`, `%bin`, `$hex`, `IF/ELIF`) |
| `orig.py`, `elitesrc.py` | builds an image with original routines (LL9, SHPPT, LL51, PROJ, PLS6, DVID3B2, FMLTU, DORND; LOIN/DOEXP/PLANET are stubs that log the call) and reads the source (symbols) |
| `frag6502.py`, `gen_laser.py`, `gen_damage.py`, `gen_planet.py`, `gen_equip.py` | assemble loose stretches of the source (by label) with logging stubs, and generate `tests/vectors_{laser,damage,planet,equip}.txt` by running the original (`gen_planet.py` runs the part of `LOIN` that matters —up to `LI3`/`LI15`— and logs the already sorted line) |
| `gen_ll9.py` | generates `tests/vectors_ll9.txt` by running the original with random inputs |
| `gen_geom.py`, `gen_lines.py`, `gen_draw.py` | (round 16) Maths Geometry with `test_state`'s inputs; `LOIN`/`HLOIN`/`VLOIN` on the emulator's hires screen decoded to game coordinates; `TT15`, `SIGHT`, `NLIN2/4`, `BOMBEFF2`, `DOEXP`, `scacol` with logging primitives → `tests/vectors_{geom,lines,draw}.txt` |
| `ref6502.py`, `ref6502b.py`, `ref6502_state.py` | **literal transcriptions** of the 6502 in Python (with explicit carries) of the arithmetic and of the stateful routines |
| `gen_vectors*.py`, `gen_trade.py` | generate the vectors for `make test`, `test_state`, `test_save` and `test_trade` |
| `gen_shipdata.py` | extracts the 32 ship blueprints from `elite-data.asm` → `src/shipdata.c/.h` |
| `gen_dashboard.py` | `A.SCREEN.bin` (Apple II hires dump) → `data/elite_dash` |
| `png2dash.py` | editable cockpit: 8-colour PNG (any multiple of 256 × 56) → `data/elite_dash`, and `--export` of the panel to PNG (by default 1024 × 112, 4 × 2 pixels with the QL's proportion); `make dash_png`, `make dash_from_png` (with `asset/cabina.png`), `make test_dashpng` (round trip) |
| `gen_font.py`, `gen_tables.py`, `gen_sne.py`, `gen_universe_text.py`, `make_oracle.py` | 8×8 font, log/sine tables, ship sine table, universe texts, Text Elite oracle |
| `ql2png.py` | converts a QL screen capture (32 KB) to PNG (`ql2png.py capture.bin output.png [4]`: the 4 = mode 4, 1024 × 768; `test_game` mode 4 captures have an `NN.m4` next to them) |
| `inventory.py`, `ported.txt` | generate `INVENTARIO.md` |
| `test_mode4asm.py` | **mode 4** assembler (`ql_line4`, `ql_ellipse` with `hr`, `ql_ellrows4` and dot/span/column/rectangle with the mode 4 colour codes) run with `m68k_mini.py` against the C versions with `gfx_m4 = 1` (`tests/m4_dump.c`); `make test_mode4asm` |
| `m68k_mini.py`, `test_lineasm.py`, `test_ellasm.py`, `test_ll9asm.py`, `test_putsasm.py`, `gen_lltab.py` | minimal 68000 interpreter and tests of our own assembler (`ql_line`, `ql_ell`, `ql_ll9`, `ql_puts`) against the C; `gen_lltab.py` generates `src/ql_lltab.asm` |
| `install_xtc68.sh`, `c68_check/` | qcc toolchain and c68 verification |
| `tmp/qltools_src/` (created automatically) | clone and build of N. Dunbar's **qltools** (`make qltools`); formats and writes the `elite.img` image (3.5). Not included in the tar |

## 3.5 `elite.img` disk image (qltools)
`make elite.img` (alias `make disk`) does everything at once and is repeatable:
- **`elite`** (the real target; the `ql_game` alias was removed on 2026-10-24): builds with qcc if any `src/*.c`, `src/*.h` or `.asm` changed. qcc leaves at the end of the file the `XTcc` trailer + dataspace (big-endian; today 16,840 bytes); qltools detects it on writing and marks the file as executable with that dataspace (`E` in the directory). qltools' `-W` is needed; do not copy the file by hand.
- **`$(QLTOOLS)`** if qltools does not exist locally it can be installed and built in `tmp/qltools_src/` with `git clone --depth 1 https://github.com/NormanDunbar/qltools tmp/qltools_src` and `make -C tmp/qltools_src/Unix` (done by `make qltools`).
- Everything generated goes to `build/` (`build/elite`, `build/elite.img`). `rm -f build/elite.img`; `qltools build/elite.img -fdd ELITE` (formats DS/DD: 2 sides, 80 cylinders, 9 sectors per track = 1,440 sectors = 737,280 bytes; ID `QL5A`); `qltools build/elite.img -W build/elite data/boot data/elite_scr data/elite_dash` (`-W` overwrites without asking; the names on the disk are the file's without the directory); `qltools build/elite.img -d` lists the result.
- **qltools notes:** it translates `.` into `_` in names (without `-t`); no name on the disk has a dot. It shows the year of the files' date as 126 (presentation-only error). The `-fdd` and `-W` arguments go **before** the file list; to check a file with an untranslated name use `-t -n name`.
- In an emulator (sQLux, QPC2, uQLx): mount `build/elite.img` as `flp1_` and start with `LRUN flp1_boot` (or let the disk boot run it).

## 3.6 `elite.mdv` microdrive image (mdvtool)
- **`$(MDVTOOL)`** if mdvtool does not exist locally it can be installed and built in `tmp/` from the repository https://github.com/xxoraa/mdvtool (`make mdvtool`; uses the one on the `PATH` if there is one; needs libzip).
- `make elite.mdv` generates `build/elite.mdv` (QLAY format, label `ELITE`) with `boot`, `elite`, `elite_scr` and `elite_dash`. It copies the files to `build/mdv/` and imports from there (mdvtool stores the name **as given**, directory included). The microdrive `boot` is the disk one with `dev$="mdv1_"` (sed). mdvtool detects the `XTcc` trailer and marks `elite` as EXEC with its dataspace.
- **Capacity:** 255 sectors of 512 B; removing the map and the directory leaves **253** for files, and each file carries a 64 B header (`MDV_SECTORS`). With a full image mdvtool ends with a segmentation fault, so the rule checks the sectors first and fails with a message. **Today the executable (~147 KB, 146 KB without `DEBUG`) does not fit** (369 sectors are needed): that is why `elite.mdv` is not in the default target. A real cartridge usually gives fewer sectors than the QLAY image.




## 5.3 How to verify a new routine against the original
`tools/orig.py` builds a 6502 image with the original routines requested and stubs for those that draw; `tools/gen_laser.py` and `tools/gen_damage.py` are the most convenient examples; they use `tools/frag6502.py` (its `build(lines, None, presets)` turns into stubs that log (name, A, X, Y) everything that is not resolved, and puts a local RTS when a branch does not reach the stub; it assembles loose stretches of the main loop by label: `.MA68`..`.MA3`, `.MA47`..`.MA8`...; `presets` fixes addresses of tables such as `log`; if a branch does not reach a label outside, a local `RTS` must be put in front of it, e.g. `['.out', 'RTS'] + OUCH`). Careful: a stretch that "falls" into the next routine (like `Ze` into `DORND2`) needs that routine right after it in the list; `tools/gen_trade.py` (preprocesses `IF/ELIF`, replaces the screen with stubs, runs `tnpr` and the `TT219`/`TT208` stretches). Steps: dump the random initial state into the emulator and into C, run, compare INWK/variables/drawn lines, repeat with boundary values, mutate an instruction of the C and check that the test fails.

## Text timing (`make ql_bench`)

`src/bench_text.c` prints 2000 characters (opaque paper, transparent, `txt_puts` and numbers) and shows the 20 ms ticks and the µs per character. Run `build/bench_ql` on the QL. Measured before `ql_text.asm` (pure C, `fm` table): opaque 73 t (730 µs/char.); before the table, 189 t. `ASM_TEXT` (in the `Makefile`, 1 by default) uses `ql_puts` (`src/ql_text.asm`: strip of cells with opaque paper, unrolled) in `txt_puts` and `txt_putc`; transparent paper is still in C. Without `ASM_TEXT` the same formula is used in C.
