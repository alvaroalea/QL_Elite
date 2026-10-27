# QL (XTC68): make elite / make disk (elite.img image) / make elite.mdv (microdrive)    PC: make host_game    Tests: make test_all
# Order: 1) build for the QL  2) asset generation  3) build on the PC  4) tests
.DEFAULT_GOAL := disk
# Version shown on the title screen ("R <VERSION>"): make VERSION=0.3
VERSION ?= pre0.3
# Reference source (Mark Moxon): the project's ref/moxon-appleII/ folder (and ref/moxon-bbc-a/ with BBC version A) (ignored by git); the environment's ELITE_SRC takes priority
export ELITE_SRC ?= $(CURDIR)/ref/moxon-appleII/1-source-files/main-sources
# Executables, disk image and generated screenshots go to build/
BUILD ?= build
# Debug functions (SHIFT-1/2/3 free buy/sell/fuel, SHIFT-C/P/H in flight): make DEBUG= to remove them
DEBUG ?= 1
DBGDEF = $(if $(DEBUG),-DDEBUG)
# Bresenham line in assembly on the QL (src/ql_line.asm); make ASM_LINE= to use the C version
ASM_LINE ?= 1
ASMDEF = $(if $(ASM_LINE),-DASM_LINE)
# LL9 in assembly (src/ql_ll9.asm: faces, vertices and LL51): make ASM_LL9= to use the C
ASM_LL9 ?= 1
ASMDEF += $(if $(ASM_LL9),-DASM_LL9)
# Opaque text cell in assembly (src/ql_text.asm): make ASM_TEXT= to use the C
ASM_TEXT ?= 1
ASMDEF += $(if $(ASM_TEXT),-DASM_TEXT)
# Version without sound: snd_ipc does nothing (no IPC, no ZX8302, no ROM): make NOSOUND=1 (sound by default; it was to detect a bug when using MT.IPCOM)
NOSOUND ?=
# Without the 50 Hz clock (MT.LPOLL) or the fps limit: make NOTICK=1 (hang diagnosis; clock by default)
SNDDEF = $(if $(NOSOUND),-DNOSOUND) $(if $(NOTICK),-DNOTICK)
VERDEF = $(DBGDEF) $(SNDDEF) -DELITE_VERSION='"$(VERSION)"'
COMMON = src/gfx.c src/text.c

# ==== 1. Build for the QL =================================================================================

# qcc toolchain (tools/install_xtc68.sh installs it in /tmp/qlsdk): if it exists, it is put on the PATH automatically
QLSDK ?= /tmp/qlsdk
ifneq ($(wildcard $(QLSDK)/bin/qcc),)
export PATH := $(QLSDK)/bin:$(PATH)
export QLINC := $(QLSDK)/share/qdos/include
export QLLIB := $(QLSDK)/share/qdos/lib
endif

# Complete game. QL: make elite (or make disk for the image); PC: make host_game (for tests)
GAME = src/main.c src/app.c src/menu.c src/chart.c src/sun.c src/dash.c src/space.c src/stars.c src/ships.c src/combat.c src/damage.c src/equip.c src/dockit.c src/tactics.c src/move.c src/flight.c src/encounters.c src/missions.c src/detok.c src/config.c src/input.c src/game.c src/cmdr.c src/save.c src/shipview.c src/ship3d.c src/shipdata.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/text.c src/universe.c src/keys.c src/sound.c

# The QDOS executable "elite" (qcc adds the XTcc trailer with the dataspace; qltools uses it to mark it executable)
GAME_QL = $(GAME) src/keys_ql.c src/sound_ql.c src/plat_ql.c src/ql_ipc.asm src/ql_line.asm src/ql_ell.asm src/ql_ll9.asm src/ql_lltab.asm src/ql_ellrows.asm src/ql_text.asm
$(BUILD)/elite: $(GAME_QL) $(wildcard src/*.h) Makefile
	mkdir -p $(BUILD)
	qcc -O $(VERDEF) $(ASMDEF) -Isrc -I/usr/local/share/qdos/include -o $@ $(GAME_QL) -lm
elite: $(BUILD)/elite

# ---- DS/DD 720 KB disk image (elite.img) ------------------------------------------------------------------
# qltools (N. Dunbar) is built from source: git clone + make -C Unix, in tmp/qltools_src (not included in the tar).
# Another qltools already installed:  make elite.img QLTOOLS=/path/qltools
QLTOOLS_REPO ?= https://github.com/NormanDunbar/qltools
QLTOOLS_DIR  ?= tmp/qltools_src
QLTOOLS      ?= $(QLTOOLS_DIR)/Unix/qltools
DISK_LABEL   ?= ELITE
DISK_FILES    = data/boot data/elite_scr data/elite_dash

$(QLTOOLS):
	test -d $(QLTOOLS_DIR)/Unix || git clone --depth 1 $(QLTOOLS_REPO) $(QLTOOLS_DIR)
	$(MAKE) -C $(QLTOOLS_DIR)/Unix

qltools: $(QLTOOLS)

# Formats (-fdd = DS/DD 720 KB) and writes: elite (executable, with its dataspace), boot, elite_scr and elite_dash.
# Inside the disk the names are those of the files without the directory: boot, elite, elite_scr, elite_dash.
# It is written from build/ so that on the disk the executable is called elite (qltools uses the name without directory)
$(BUILD)/elite.img: $(BUILD)/elite $(DISK_FILES) $(QLTOOLS)
	rm -f $@
	$(QLTOOLS) $@ -fdd $(DISK_LABEL)
	$(QLTOOLS) $@ -W $(BUILD)/elite $(DISK_FILES)
	$(QLTOOLS) $@ -d

elite.img: $(BUILD)/elite.img
disk: elite.img

# ---- Microdrive image in QLAY format (elite.mdv) ----------------------------------------------------------
# mdvtool (T. Harbaum / xXorAa): the one on the PATH or, if there is none, built in tmp/mdvtool (needs libzip-dev).
# Another mdvtool:  make elite.mdv MDVTOOL=/path/mdvtool
# It is not in the default target: a QLAY cartridge has 255 sectors of 512 B (253 free for files,
# ~126 KB; a real cartridge usually gives less) and today the executable takes ~147 KB. The rule checks the size
# before calling mdvtool, which with a full image ends with a segmentation fault.
MDVTOOL_REPO ?= https://github.com/xxoraa/mdvtool
MDVTOOL_DIR  ?= tmp/mdvtool
MDVTOOL      ?= $(or $(shell command -v mdvtool 2>/dev/null),$(MDVTOOL_DIR)/mdvtool)
MDV_LABEL    ?= ELITE
MDV_SECTORS  ?= 253

$(MDVTOOL_DIR)/mdvtool:
	test -d $(MDVTOOL_DIR) || git clone --depth 1 $(MDVTOOL_REPO) $(MDVTOOL_DIR)
	$(MAKE) -C $(MDVTOOL_DIR)

mdvtool: $(MDVTOOL)

# mdvtool stores the name as given: the files are copied to build/mdv/ and imported from there.
# The microdrive boot is the disk one with dev$="mdv1_". Each file carries a 64 B QDOS header.
$(BUILD)/elite.mdv: $(BUILD)/elite $(DISK_FILES) $(MDVTOOL)
	rm -rf $(BUILD)/mdv $@
	mkdir -p $(BUILD)/mdv
	cp $(BUILD)/elite data/elite_scr data/elite_dash $(BUILD)/mdv/
	sed 's/"flp1_"/"mdv1_"/' data/boot > $(BUILD)/mdv/boot
	@n=0; for f in $(BUILD)/mdv/*; do s=$$(wc -c < $$f); n=$$((n + (s + 64 + 511) / 512)); done; \
	if [ $$n -gt $(MDV_SECTORS) ]; then \
	  echo "elite.mdv: los ficheros necesitan $$n sectores y el microdrive tiene $(MDV_SECTORS)"; ls -l $(BUILD)/mdv; exit 1; fi
	cd $(BUILD)/mdv && $(abspath $(MDVTOOL)) create name $(MDV_LABEL) import boot import elite import elite_scr import elite_dash write ../elite.mdv
	$(MDVTOOL) $@ dir

elite.mdv: $(BUILD)/elite.mdv

# Demos for the QL (text, keyboard, sound, system sheet and 3D projection).
# -Isrc needed: qcc looks for "platform.h" in the current directory. "ql" is the text test.
ql:
	mkdir -p $(BUILD)
	qcc -O -Isrc -I/usr/local/share/qdos/include -o $(BUILD)/elite_ql src/test_text.c $(COMMON) src/plat_ql.c src/keys.c src/keys_ql.c src/ql_ipc.asm -lm
ql_bench:
	mkdir -p $(BUILD)
	qcc -O -Isrc -I/usr/local/share/qdos/include -DASM_TEXT -o $(BUILD)/bench_ql src/bench_text.c $(COMMON) src/plat_ql.c src/keys.c src/keys_ql.c src/ql_ipc.asm src/ql_text.asm -lm
ql_keys:
	mkdir -p $(BUILD)
	qcc -O -Isrc -I/usr/local/share/qdos/include -o $(BUILD)/keys_ql src/demo_keys.c src/gfx.c src/text.c src/keys.c src/keys_ql.c src/plat_ql.c src/ql_ipc.asm
ql_sound:
	mkdir -p $(BUILD)
	qcc -O -Isrc -I/usr/local/share/qdos/include -o $(BUILD)/sound_ql src/demo_sound.c src/gfx.c src/text.c src/keys.c src/keys_ql.c src/sound.c src/sound_ql.c src/plat_ql.c src/ql_ipc.asm
ql_universe:
	mkdir -p $(BUILD)
	qcc -O -Isrc -I/usr/local/share/qdos/include -o $(BUILD)/universe_ql src/demo_universe.c src/gfx.c src/text.c src/universe.c src/emath.c src/lines.c src/plat_ql.c src/ql_ipc.asm
ql_ship:
	mkdir -p $(BUILD)
	qcc -O -Isrc -I/usr/local/share/qdos/include -o $(BUILD)/ship_ql src/demo_ship.c src/shipview.c src/ship3d.c src/shipdata.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/text.c src/plat_ql.c src/ql_ipc.asm

# ==== 2. Asset generation =================================================================================

# Editable cockpit on the PC: exports the panel to PNG (asset/cabina.png, 1024 x 112) and converts it back into data/elite_dash
dash_png:
	python3 tools/png2dash.py --export data/elite_dash asset/cabina.png
dash_from_png:
	python3 tools/png2dash.py asset/cabina.png data/elite_dash

# Panel background file (data/elite_dash) from the Apple II screen dump (needs ELITE_SRC)
dash_file:
	python3 tools/gen_dashboard.py

# ==== 3. Build on the PC ==================================================================================

# Complete game on the PC (screen in memory)
host_game:
	mkdir -p $(BUILD)
	gcc -std=c89 -pedantic -Wall -O2 $(VERDEF) -Isrc -o $(BUILD)/elite_host $(GAME) src/keys_host.c src/sound_host.c src/plat_host.c -lm

# Demos on the PC: graphics/text, keyboard, system sheet and 3D projection
host:
	mkdir -p $(BUILD)
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o $(BUILD)/test_gfx src/test_gfx.c src/gfx.c src/plat_host.c -lm
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o $(BUILD)/test_text src/test_text.c $(COMMON) src/plat_host.c -lm
	./$(BUILD)/test_gfx && python3 tools/ql2png.py screen.bin $(BUILD)/test.png
	./$(BUILD)/test_text && python3 tools/ql2png.py screen.bin $(BUILD)/test_text.png

# Keyboard demo: on the PC it draws a frame with a simulated key press; on the QL it is interactive
host_demo_keys:
	mkdir -p $(BUILD)
	gcc -std=c89 -pedantic -Wall -O2 -DHOST_DEMO -Isrc -o $(BUILD)/demo_keys_host src/demo_keys.c src/gfx.c src/text.c src/keys.c src/keys_host.c src/plat_host.c

# System sheet demo (data, description and market)
host_demo_universe:
	mkdir -p $(BUILD)
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o $(BUILD)/demo_universe src/demo_universe.c src/gfx.c src/text.c src/universe.c src/emath.c src/lines.c src/plat_host.c
	./$(BUILD)/demo_universe && python3 tools/ql2png.py screen.bin $(BUILD)/demo_universe.png

# 3D projection demo: six ships drawn with LL9 (XOR)
host_demo_ship:
	mkdir -p $(BUILD)
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o $(BUILD)/demo_ship src/demo_ship.c src/shipview.c src/ship3d.c src/shipdata.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/text.c src/plat_host.c
	./$(BUILD)/demo_ship && python3 tools/ql2png.py screen.bin $(BUILD)/demo_ship.png

clean:
	rm -f tests/test_trade tests/test_laser tests/vectors_laser.txt tests/test_damage tests/vectors_damage.txt tests/test_planet tests/vectors_planet.txt tests/test_equip tests/vectors_equip.txt tests/test_tactics tests/vectors_tactics.txt tests/test_missions tests/vectors_missions.txt tests/test_univ6502 tests/vectors_univ.txt tests/test_shipview tests/vectors_trade.txt screen.bin tests/test_emath tests/test_emath2 tests/test_state tests/test_keys tests/test_sound tests/test_ll9 tests/test_save tests/test_game tests/dump_universe tests/oracle/oracle tests/shots/*.bin tests/tmp/*
	rm -rf $(BUILD)

# ==== 4. Tests ============================================================================================

test_all: test test_keys test_sound test_universe test_save test_game test_shipview test_space test_dashpng test_bres test_lineasm test_ellipse test_nativeview test_arcs test_ellasm test_dust test_ll9asm test_ellrows test_ellrowsasm test_putsasm test_mode4asm

# Compares c68's code (-int=16 -O) with the PC's under qemu (needs c68, gcc-m68k-linux-gnu and qemu-user; see tools/c68_check/run.sh)
c68_check:
	tools/c68_check/run.sh

# Arithmetic tests: regenerates tables and vectors (literal 6502 reference) and compares with emath.c
test:
	python3 tools/gen_tables.py
	python3 tools/gen_sne.py
	python3 tools/gen_vectors.py
	python3 tools/gen_vectors2.py
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_emath tests/test_emath.c src/emath.c
	gcc -O2 -Isrc -o tests/test_emath2 tests/test_emath2.c src/emath.c
	./tests/test_emath
	./tests/test_emath2
	$(MAKE) test_state

# Game state and dependent routines (ship_math.c) against the literal 6502 transcription
test_state:
	python3 tools/gen_vectors3.py
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_state tests/test_state.c src/ship_math.c src/state.c src/emath.c
	./tests/test_state

# Maths (Geometry), MVT3, PLS6 and money against the ORIGINAL code in the 6502 emulator (same inputs as test_state)
test_geom:
	python3 tools/gen_geom.py 3000
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_state tests/test_state.c src/ship_math.c src/state.c src/emath.c
	./tests/test_state tests/vectors_geom.txt

# Drawing lines: LOIN, HLOIN and VLOIN point by point against the ORIGINAL code in the emulator (Apple II hires screen)
test_lines:
	python3 tools/gen_lines.py 3000
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_lines tests/test_lines.c src/lines.c src/gfx.c src/emath.c
	./tests/test_lines

# TT15, SIGHT, NLIN2/NLIN4, BOMBEFF2 (with the DORNDs of SOBOMB) and DOEXP against the ORIGINAL code in the emulator
test_draw:
	python3 tools/gen_draw.py 2000
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_draw tests/test_draw.c src/combat.c src/damage.c src/equip.c src/dockit.c src/tactics.c src/space.c src/stars.c src/ships.c src/move.c src/sun.c src/chart.c src/ship3d.c src/shipdata.c src/dash.c src/game.c src/cmdr.c src/save.c src/universe.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/text.c src/keys.c src/keys_host.c src/sound.c src/sound_host.c src/plat_host.c
	./tests/test_draw

# Stardust: STARS (the 4 views), nWq and FLIP with real PIXEL2 and PIXEL against the ORIGINAL code in the emulator
test_stars:
	python3 tools/gen_stars.py 3000
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_stars tests/test_stars.c src/combat.c src/damage.c src/equip.c src/dockit.c src/tactics.c src/space.c src/stars.c src/ships.c src/move.c src/sun.c src/chart.c src/ship3d.c src/shipdata.c src/dash.c src/game.c src/cmdr.c src/save.c src/universe.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/text.c src/keys.c src/keys_host.c src/sound.c src/sound_host.c src/plat_host.c
	./tests/test_stars

# Charts: TT22, TT23, TT16, hm and HME2 (search, with the real text engine) against the ORIGINAL code in the emulator
test_charts:
	python3 tools/gen_charts.py 300
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_charts tests/test_charts.c src/combat.c src/damage.c src/equip.c src/dockit.c src/tactics.c src/space.c src/stars.c src/ships.c src/move.c src/sun.c src/chart.c src/ship3d.c src/shipdata.c src/dash.c src/game.c src/cmdr.c src/save.c src/universe.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/text.c src/keys.c src/keys_host.c src/sound.c src/sound_host.c src/plat_host.c
	./tests/test_charts

# Drawing the screen: TT66/TTX66/TTX66K/BOX and CLYNS against the ORIGINAL code in the emulator
test_screen:
	python3 tools/gen_screen.py 2000
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_screen tests/test_screen.c src/app.c src/menu.c src/chart.c src/sun.c src/dash.c src/space.c src/stars.c src/ships.c src/combat.c src/damage.c src/equip.c src/dockit.c src/tactics.c src/move.c src/flight.c src/encounters.c src/missions.c src/detok.c src/config.c src/input.c src/game.c src/cmdr.c src/save.c src/shipview.c src/ship3d.c src/shipdata.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/text.c src/universe.c src/keys.c src/sound.c src/keys_host.c src/sound_host.c src/plat_host.c -lm
	./tests/test_screen

# Keyboard/joystick test with a simulated matrix
test_keys:
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_keys tests/test_keys.c src/keys.c src/keys_host.c
	./tests/test_keys

# Sound tests (IPC blocks and priorities)
test_sound:
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_sound tests/test_sound.c src/sound.c src/sound_host.c -lm
	./tests/test_sound

# Universe: builds the oracle from TXTELITE.C 1.5, dumps both and compares
test_universe:
	python3 tools/gen_universe_text.py
	python3 tools/make_oracle.py
	gcc -std=gnu89 -w -funsigned-char -o tests/oracle/oracle tests/oracle/oracle.c -lm
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/dump_universe tests/dump_universe.c src/universe.c src/emath.c
	./tests/oracle/oracle > tests/oracle/oracle.txt
	./tests/dump_universe > tests/oracle/ours.txt
	python3 tests/compare_universe.py tests/oracle/oracle.txt tests/oracle/ours.txt

# Comparison with the ORIGINAL code run in a 6502 emulator (needs the cloned reference source:
# git clone https://github.com/markmoxon/elite-source-code-apple-ii ref/moxon-appleII  or  export ELITE_SRC=<path>/1-source-files/main-sources)
test_orig:
	python3 tools/gen_ll9.py 3000
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_ll9 tests/test_ll9.c src/ship3d.c src/shipdata.c src/ship_math.c src/state.c src/emath.c
	./tests/test_ll9

# Text under the flight panel: painting over another state (emptying only the leftover rectangle) = painting from scratch
test_panel:
	gcc -std=c89 -O2 -Isrc -o tests/test_panel tests/test_panel.c src/app.c src/menu.c src/chart.c src/sun.c src/dash.c src/space.c src/stars.c src/ships.c src/combat.c src/damage.c src/equip.c src/dockit.c src/tactics.c src/move.c src/encounters.c src/missions.c src/detok.c src/config.c src/input.c src/game.c src/cmdr.c src/save.c src/shipview.c src/ship3d.c src/shipdata.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/text.c src/universe.c src/keys.c src/sound.c src/keys_host.c src/sound_host.c src/plat_host.c -lm
	./tests/test_panel

# Stardust (sizes by distance, erasing, edges) and original PIXEL intact
test_dust:
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_dust tests/test_dust.c src/gfx.c
	./tests/test_dust

# ql_ell.asm (ellipse and horizontal span in assembly) run with tools/m68k_mini.py against gfx_ellipse_xor and gfx_hline_xor in C
test_ellasm:
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/ell_dump tests/ell_dump.c src/gfx.c
	python3 tools/test_ellasm.py 700

# Row-by-row ellipse outline (gfx_ell_update, no table = midpoint): same screen as drawing the old and the new with gfx_ellipse_xor
test_ellrows:
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_ellrows tests/test_ellrows.c src/gfx.c src/plat_host.c -lm
	./tests/test_ellrows

# ql_text.asm (ql_puts) run with tools/m68k_mini.py against its C version (text.c's formula)
test_putsasm:
	python3 tools/test_putsasm.py 2000

# Mode 4 (512 x 256): ql_line4, ql_ellipse (hr), ql_ellrows4 and the dot/span/rectangle routines with the mode 4 colour codes,
# run with tools/m68k_mini.py against the C versions with gfx_m4 = 1 (tests/m4_dump.c)
test_mode4asm:
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/m4_dump tests/m4_dump.c src/gfx.c src/lines.c src/emath.c -lm
	python3 tools/test_mode4asm.py 400

# ql_ellrows.asm run with tools/m68k_mini.py against gfx_ell_update in C
test_ellrowsasm:
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/ellrows_dump tests/ellrows_dump.c src/gfx.c src/plat_host.c -lm
	python3 tools/test_ellrowsasm.py 200 1

# ql_ll9.asm (LL51, faces and vertices of LL9 in assembly) run with tools/m68k_mini.py against ship_ll9_faces_c and ship_ll9_verts_c (real ships, edge cases)
test_ll9asm:
	python3 tools/gen_lltab.py
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/ll9_dump tests/ll9_dump.c src/ship3d.c src/shipdata.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/plat_host.c -lm
	python3 tools/test_ll9asm.py 300 1

# Native flight view: sun by rows (invariant screen = spans), planet (redraw jump), tunnel rings and exact erasing
test_nativeview:
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_nativeview tests/test_nativeview.c src/ships.c src/move.c src/sun.c src/ship3d.c src/shipdata.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/combat.c src/damage.c src/equip.c src/dockit.c src/tactics.c src/space.c src/stars.c src/dash.c src/game.c src/cmdr.c src/save.c src/universe.c src/text.c src/keys.c src/keys_host.c src/sound.c src/sound_host.c src/plat_host.c src/chart.c -lm
	./tests/test_nativeview

# Meridians and crater of the planet (native view): no leftovers on removal, stable figure, hidden crater
test_arcs:
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_arcs tests/test_arcs.c src/ships.c src/move.c src/sun.c src/ship3d.c src/shipdata.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/combat.c src/damage.c src/equip.c src/dockit.c src/tactics.c src/space.c src/stars.c src/dash.c src/game.c src/cmdr.c src/save.c src/universe.c src/text.c src/keys.c src/keys_host.c src/sound.c src/sound_host.c src/plat_host.c src/chart.c -lm
	./tests/test_arcs

# Midpoint ellipse (planet and tunnel rings in the native view): axes, symmetry with no points erased by XOR, closeness to the ideal ellipse, exact erasing and clipping
test_ellipse:
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_ellipse tests/test_ellipse.c src/gfx.c -lm
	./tests/test_ellipse

# Game Bresenham line (endpoints, symmetry, exact erasing, closeness to the line) and its assembly version (ql_line.asm)
# run with the 68000 interpreter of tools/m68k_mini.py against the C version
test_bres:
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_bres tests/test_bres.c src/lines.c src/emath.c src/gfx.c -lm
	./tests/test_bres

test_lineasm: test_bres
	python3 tools/test_lineasm.py 3000

# Game logic and script-driven session (PC): saves screenshots in tests/shots/
test_game:
	mkdir -p tests/shots tests/tmp
	cp data/elite_dash tests/tmp/elite_dash
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_game tests/test_game.c src/app.c src/menu.c src/chart.c src/sun.c src/dash.c src/space.c src/stars.c src/ships.c src/combat.c src/damage.c src/equip.c src/dockit.c src/tactics.c src/move.c src/flight.c src/encounters.c src/missions.c src/detok.c src/config.c src/input.c src/game.c src/cmdr.c src/save.c src/shipview.c src/ship3d.c src/shipdata.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/text.c src/universe.c src/keys.c src/sound.c src/keys_host.c src/sound_host.c src/plat_host.c -lm
	./tests/test_game

test_save:
	python3 tools/gen_vectors_chk.py
	mkdir -p tests/tmp
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_save tests/test_save.c src/cmdr.c src/save.c src/state.c
	./tests/test_save

# Editable cockpit: PNG -> elite_dash -> PNG gives the same file
test_dashpng:
	mkdir -p tests/tmp
	python3 tools/png2dash.py --export data/elite_dash tests/tmp/cabina.png
	python3 tools/png2dash.py tests/tmp/cabina.png tests/tmp/elite_dash_png
	cmp data/elite_dash tests/tmp/elite_dash_png && echo "png2dash: ida y vuelta identica"

test_shipview:
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_shipview tests/test_shipview.c src/shipview.c src/ship3d.c src/shipdata.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/plat_host.c
	./tests/test_shipview

# Trade against the original 6502 (tnpr, TT219, TT208, GCASH, LCASH, MCASH). Requires ELITE_SRC (ref/, tools/README_ref.txt)
test_trade:
	python3 tools/gen_trade.py 20000 7
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_trade tests/test_trade.c src/combat.c src/damage.c src/equip.c src/dockit.c src/tactics.c src/space.c src/stars.c src/ships.c src/move.c src/sun.c src/chart.c src/ship3d.c src/shipdata.c src/dash.c src/game.c src/cmdr.c src/save.c src/universe.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/text.c src/keys.c src/keys_host.c src/sound.c src/sound_host.c src/plat_host.c
	./tests/test_trade

test_space:
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_space tests/test_space.c src/space.c src/stars.c src/ships.c src/combat.c src/damage.c src/equip.c src/dockit.c src/tactics.c src/move.c src/sun.c src/chart.c src/ship3d.c src/shipdata.c src/dash.c src/game.c src/cmdr.c src/save.c src/universe.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/text.c src/keys.c src/keys_host.c src/sound.c src/sound_host.c src/plat_host.c
	./tests/test_space

# Laser and damage against the original 6502 (HITCH, ANGRY, LASLI, MA68, MLOOP, MA77/SHD, MA47, MA15, SFS1). Requires ELITE_SRC
test_laser:
	python3 tools/gen_laser.py 3000 7
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_laser tests/test_laser.c src/combat.c src/damage.c src/equip.c src/dockit.c src/tactics.c src/space.c src/stars.c src/ships.c src/move.c src/sun.c src/chart.c src/ship3d.c src/shipdata.c src/dash.c src/game.c src/cmdr.c src/save.c src/universe.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/text.c src/keys.c src/keys_host.c src/sound.c src/sound_host.c src/plat_host.c
	./tests/test_laser

# Damage to our ship against the original 6502 (OOPS, OUCH, parts 9, 10 and 15, D1 of DEATH). Requires ELITE_SRC
test_damage:
	python3 tools/gen_damage.py 3000 7
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_damage tests/test_damage.c src/combat.c src/damage.c src/equip.c src/dockit.c src/tactics.c src/space.c src/stars.c src/ships.c src/move.c src/sun.c src/chart.c src/ship3d.c src/shipdata.c src/dash.c src/game.c src/cmdr.c src/save.c src/universe.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/text.c src/keys.c src/keys_host.c src/sound.c src/sound_host.c src/plat_host.c
	./tests/test_damage

# Planets and suns against the original 6502 (PLANET, SUN, CIRCLE, BLINE, WPLS2, LL145...). Requires ELITE_SRC
test_planet:
	python3 tools/gen_planet.py 400 7
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_planet tests/test_planet.c src/combat.c src/damage.c src/equip.c src/dockit.c src/tactics.c src/space.c src/stars.c src/ships.c src/move.c src/sun.c src/chart.c src/ship3d.c src/shipdata.c src/dash.c src/game.c src/cmdr.c src/save.c src/universe.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/text.c src/keys.c src/keys_host.c src/sound.c src/sound_host.c src/plat_host.c
	./tests/test_planet

# Equipment against the original 6502 (bomb, WARP, DOCKIT, DOKEY with docking computer, GTHG, FRS1). Requires ELITE_SRC
test_equip:
	python3 tools/gen_equip.py 3000 7
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_equip tests/test_equip.c src/combat.c src/damage.c src/equip.c src/dockit.c src/tactics.c src/space.c src/stars.c src/ships.c src/move.c src/sun.c src/chart.c src/ship3d.c src/shipdata.c src/dash.c src/game.c src/cmdr.c src/save.c src/universe.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/text.c src/keys.c src/keys_host.c src/sound.c src/sound_host.c src/plat_host.c
	./tests/test_equip

# TACTICS against the original 6502. Requires ELITE_SRC
test_univ6502:
	python3 tools/gen_universe.py 2000 7
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_univ6502 tests/test_univ6502.c $(filter-out src/main.c,$(GAME)) src/keys_host.c src/sound_host.c src/plat_host.c -lm
	./tests/test_univ6502

test_missions:
	python3 tools/gen_tokens.py
	python3 tools/gen_missions.py 800 11
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_missions tests/test_missions.c $(filter-out src/main.c,$(GAME)) src/keys_host.c src/sound_host.c src/plat_host.c -lm
	./tests/test_missions

test_tactics:
	python3 tools/gen_tactics.py 20000 7
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_tactics tests/test_tactics.c src/combat.c src/damage.c src/equip.c src/dockit.c src/tactics.c src/space.c src/stars.c src/ships.c src/move.c src/sun.c src/chart.c src/ship3d.c src/shipdata.c src/dash.c src/game.c src/cmdr.c src/save.c src/universe.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/text.c src/keys.c src/keys_host.c src/sound.c src/sound_host.c src/plat_host.c
	./tests/test_tactics

# Start and end: RESET, RES2 and DFAULT against the ORIGINAL code in the 6502 emulator (needs ELITE_SRC)
test_start:
	python3 tools/gen_start.py 1000
	gcc -std=c89 -pedantic -O2 -Isrc -o tests/test_start tests/test_start.c src/app.c src/menu.c src/chart.c src/sun.c src/dash.c src/space.c src/stars.c src/ships.c src/combat.c src/damage.c src/equip.c src/dockit.c src/tactics.c src/move.c src/flight.c src/encounters.c src/missions.c src/detok.c src/config.c src/input.c src/game.c src/cmdr.c src/save.c src/shipview.c src/ship3d.c src/shipdata.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/text.c src/universe.c src/keys.c src/sound.c src/keys_host.c src/sound_host.c src/plat_host.c -lm
	./tests/test_start

# Sound: DORNDs consumed by each sound, RAND and carry against the ORIGINAL code in the emulator (needs ELITE_SRC)
test_soundorig:
	python3 tools/gen_sound.py 300
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_sound_orig tests/test_sound_orig.c src/sound.c src/sound_host.c src/emath.c -lm
	./tests/test_sound_orig

# Moving: MVT1, MVS4, MVS5, MVT6 and MVEIT (with MV40, MV45, TIDY, SCAN and TACTICS logged) against the ORIGINAL code (needs ELITE_SRC)
test_move:
	python3 tools/gen_move.py 3000
	gcc -std=c89 -pedantic -O2 -Isrc -o tests/test_move tests/test_move.c src/app.c src/menu.c src/chart.c src/sun.c src/dash.c src/space.c src/stars.c src/ships.c src/combat.c src/damage.c src/equip.c src/dockit.c src/tactics.c src/move.c src/flight.c src/encounters.c src/missions.c src/detok.c src/config.c src/input.c src/game.c src/cmdr.c src/save.c src/shipview.c src/ship3d.c src/shipdata.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/text.c src/universe.c src/keys.c src/sound.c src/keys_host.c src/sound_host.c src/plat_host.c -lm
	./tests/test_move

# Equipment: EQSHP (equip_buy, equip_price) against the ORIGINAL code in the 6502 emulator (needs ELITE_SRC)
test_eqshp:
	python3 tools/gen_eqshp.py 3000
	gcc -std=c89 -pedantic -O2 -Isrc -o tests/test_eqshp tests/test_eqshp.c src/app.c src/menu.c src/chart.c src/sun.c src/dash.c src/space.c src/stars.c src/ships.c src/combat.c src/damage.c src/equip.c src/dockit.c src/tactics.c src/move.c src/flight.c src/encounters.c src/missions.c src/detok.c src/config.c src/input.c src/game.c src/cmdr.c src/save.c src/shipview.c src/ship3d.c src/shipdata.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/text.c src/universe.c src/keys.c src/sound.c src/keys_host.c src/sound_host.c src/plat_host.c -lm
	./tests/test_eqshp

# Keyboard: DK4 (FREEZE) and DKS3 against the ORIGINAL code in the 6502 emulator (needs ELITE_SRC)
test_dk4:
	python3 tools/gen_keys.py 2000
	gcc -std=c89 -pedantic -O2 -Isrc -o tests/test_dk4 tests/test_dk4.c src/app.c src/menu.c src/chart.c src/sun.c src/dash.c src/space.c src/stars.c src/ships.c src/combat.c src/damage.c src/equip.c src/dockit.c src/tactics.c src/move.c src/flight.c src/encounters.c src/missions.c src/detok.c src/config.c src/input.c src/game.c src/cmdr.c src/save.c src/shipview.c src/ship3d.c src/shipdata.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/text.c src/universe.c src/keys.c src/sound.c src/keys_host.c src/sound_host.c src/plat_host.c -lm
	./tests/test_dk4

# Save and load: the factory JAMESON commander (NA2%) against the original source (needs ELITE_SRC)
test_jameson:
	python3 tools/gen_jameson.py
	gcc -std=c89 -pedantic -O2 -Isrc -o tests/test_jameson tests/test_jameson.c src/app.c src/menu.c src/chart.c src/sun.c src/dash.c src/space.c src/stars.c src/ships.c src/combat.c src/damage.c src/equip.c src/dockit.c src/tactics.c src/move.c src/flight.c src/encounters.c src/missions.c src/detok.c src/config.c src/input.c src/game.c src/cmdr.c src/save.c src/shipview.c src/ship3d.c src/shipdata.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/text.c src/universe.c src/keys.c src/sound.c src/keys_host.c src/sound_host.c src/plat_host.c -lm
	./tests/test_jameson

# Dashboard: DIALS, COMPAS, MSBAR, msblob, SCAN, cntr, BUMP2 and REDU2 against the ORIGINAL code (needs ELITE_SRC)
test_dash:
	python3 tools/gen_dash.py 2000
	gcc -std=c89 -pedantic -O2 -Isrc -o tests/test_dash tests/test_dash.c src/app.c src/menu.c src/chart.c src/sun.c src/dash.c src/space.c src/stars.c src/ships.c src/combat.c src/damage.c src/equip.c src/dockit.c src/tactics.c src/move.c src/flight.c src/encounters.c src/missions.c src/detok.c src/config.c src/input.c src/game.c src/cmdr.c src/save.c src/shipview.c src/ship3d.c src/shipdata.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/text.c src/universe.c src/keys.c src/sound.c src/keys_host.c src/sound_host.c src/plat_host.c -lm
	./tests/test_dash

# Flight: PLUT, ESCAPE, MJP and TT110 against the ORIGINAL code in the 6502 emulator (needs ELITE_SRC)
test_flight:
	python3 tools/gen_flight.py 1000
	gcc -std=c89 -pedantic -O2 -Isrc -o tests/test_flight tests/test_flight.c src/app.c src/menu.c src/chart.c src/sun.c src/dash.c src/space.c src/stars.c src/ships.c src/combat.c src/damage.c src/equip.c src/dockit.c src/tactics.c src/move.c src/flight.c src/encounters.c src/missions.c src/detok.c src/config.c src/input.c src/game.c src/cmdr.c src/save.c src/shipview.c src/ship3d.c src/shipdata.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/text.c src/universe.c src/keys.c src/sound.c src/keys_host.c src/sound_host.c src/plat_host.c -lm
	./tests/test_flight

# Maths (Arithmetic) against the ORIGINAL code in the 6502 emulator (needs ELITE_SRC)
test_arith:
	python3 tools/gen_arith.py 2000
	gcc -std=c89 -pedantic -O2 -Isrc -o tests/test_arith tests/test_arith.c src/emath.c
	./tests/test_arith


.PHONY: elite elite.img elite.mdv disk qltools mdvtool ql ql_keys ql_bench test_putsasm ql_sound ql_universe ql_ship dash_png dash_from_png dash_file host host_demo_keys host_demo_universe host_demo_ship host_game clean test_all c68_check test_bres test_lineasm test_ellipse test_nativeview test_arcs test_ellasm test_dust test_panel
