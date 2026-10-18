# Pruebas en PC:  make host        QL con XTC68:  make ql
COMMON = src/gfx.c src/text.c

host:
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o test_gfx src/test_gfx.c src/gfx.c src/plat_host.c -lm
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o test_text src/test_text.c $(COMMON) src/plat_host.c -lm
	./test_gfx && python3 tools/ql2png.py screen.bin test.png
	./test_text && python3 tools/ql2png.py screen.bin test_text.png

# -Isrc necesario: qcc busca "platform.h" en el directorio actual. El programa del QL es la prueba de texto.
ql:
	qcc -O -Isrc -I/usr/local/share/qdos/include -o elite_ql src/test_text.c $(COMMON) src/plat_ql.c src/keys.c src/keys_ql.c src/ql_ipc.asm -lm

# Pruebas de aritmetica: regenera tablas y vectores (referencia literal del 6502) y compara con emath.c
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

# Estado del juego y rutinas dependientes (ship_math.c) contra la transcripcion literal del 6502
test_state:
	python3 tools/gen_vectors3.py
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_state tests/test_state.c src/ship_math.c src/state.c src/emath.c
	./tests/test_state

# Maths (Geometry), MVT3, PLS6 y dinero contra el codigo ORIGINAL en el emulador de 6502 (mismas entradas que test_state)
test_geom:
	python3 tools/gen_geom.py 3000
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_state tests/test_state.c src/ship_math.c src/state.c src/emath.c
	./tests/test_state tests/vectors_geom.txt

# Drawing lines: LOIN, HLOIN y VLOIN punto a punto contra el codigo ORIGINAL en el emulador (pantalla hires del Apple II)
test_lines:
	python3 tools/gen_lines.py 3000
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_lines tests/test_lines.c src/lines.c src/gfx.c src/emath.c
	./tests/test_lines

# TT15, SIGHT, NLIN2/NLIN4, BOMBEFF2 (con los DORND de SOBOMB) y DOEXP contra el codigo ORIGINAL en el emulador
test_draw:
	python3 tools/gen_draw.py 2000
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_draw tests/test_draw.c src/combat.c src/damage.c src/equip.c src/dockit.c src/tactics.c src/space.c src/stars.c src/ships.c src/move.c src/sun.c src/chart.c src/ship3d.c src/shipdata.c src/dash.c src/game.c src/cmdr.c src/save.c src/universe.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/text.c src/keys.c src/keys_host.c src/sound.c src/sound_host.c src/plat_host.c
	./tests/test_draw

# Stardust: STARS (las 4 vistas), nWq y FLIP con PIXEL2 y PIXEL reales contra el codigo ORIGINAL en el emulador
test_stars:
	python3 tools/gen_stars.py 3000
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_stars tests/test_stars.c src/combat.c src/damage.c src/equip.c src/dockit.c src/tactics.c src/space.c src/stars.c src/ships.c src/move.c src/sun.c src/chart.c src/ship3d.c src/shipdata.c src/dash.c src/game.c src/cmdr.c src/save.c src/universe.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/text.c src/keys.c src/keys_host.c src/sound.c src/sound_host.c src/plat_host.c
	./tests/test_stars

# Charts: TT22, TT23, TT16, hm y HME2 (busqueda, con el motor de texto real) contra el codigo ORIGINAL en el emulador
test_charts:
	python3 tools/gen_charts.py 300
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_charts tests/test_charts.c src/combat.c src/damage.c src/equip.c src/dockit.c src/tactics.c src/space.c src/stars.c src/ships.c src/move.c src/sun.c src/chart.c src/ship3d.c src/shipdata.c src/dash.c src/game.c src/cmdr.c src/save.c src/universe.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/text.c src/keys.c src/keys_host.c src/sound.c src/sound_host.c src/plat_host.c
	./tests/test_charts

# Drawing the screen: TT66/TTX66/TTX66K/BOX y CLYNS contra el codigo ORIGINAL en el emulador
test_screen:
	python3 tools/gen_screen.py 2000
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_screen tests/test_screen.c src/app.c src/menu.c src/chart.c src/sun.c src/dash.c src/space.c src/stars.c src/ships.c src/combat.c src/damage.c src/equip.c src/dockit.c src/tactics.c src/move.c src/flight.c src/encounters.c src/missions.c src/detok.c src/config.c src/input.c src/game.c src/cmdr.c src/save.c src/shipview.c src/ship3d.c src/shipdata.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/text.c src/universe.c src/keys.c src/sound.c src/keys_host.c src/sound_host.c src/plat_host.c -lm
	./tests/test_screen

# Prueba de teclado/joystick con matriz simulada
test_keys:
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_keys tests/test_keys.c src/keys.c src/keys_host.c
	./tests/test_keys

# Demo de teclado: en el PC dibuja un cuadro con una pulsacion simulada; en el QL es interactiva
host_demo_keys:
	gcc -std=c89 -pedantic -Wall -O2 -DHOST_DEMO -Isrc -o demo_keys_host src/demo_keys.c src/gfx.c src/text.c src/keys.c src/keys_host.c src/plat_host.c
ql_keys:
	qcc -O -Isrc -I/usr/local/share/qdos/include -o keys_ql src/demo_keys.c src/gfx.c src/text.c src/keys.c src/keys_ql.c src/plat_ql.c src/ql_ipc.asm

# Pruebas de sonido (bloques IPC y prioridades)
test_sound:
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_sound tests/test_sound.c src/sound.c src/sound_host.c -lm
	./tests/test_sound

# Demo de sonido para el QL
ql_sound:
	qcc -O -Isrc -I/usr/local/share/qdos/include -o sound_ql src/demo_sound.c src/gfx.c src/text.c src/keys.c src/keys_ql.c src/sound.c src/sound_ql.c src/plat_ql.c src/ql_ipc.asm

# Universo: construye el oraculo desde TXTELITE.C 1.5, vuelca ambos y compara
test_universe:
	python3 tools/gen_universe_text.py
	python3 tools/make_oracle.py
	gcc -w -funsigned-char -o tests/oracle/oracle tests/oracle/oracle.c -lm
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/dump_universe tests/dump_universe.c src/universe.c src/emath.c
	./tests/oracle/oracle > tests/oracle/oracle.txt
	./tests/dump_universe > tests/oracle/ours.txt
	python3 tests/compare_universe.py tests/oracle/oracle.txt tests/oracle/ours.txt

# Demo de ficha de sistema (datos, descripcion y mercado)
host_demo_universe:
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o demo_universe src/demo_universe.c src/gfx.c src/text.c src/universe.c src/emath.c src/lines.c src/plat_host.c
	./demo_universe && python3 tools/ql2png.py screen.bin demo_universe.png
ql_universe:
	qcc -O -Isrc -I/usr/local/share/qdos/include -o universe_ql src/demo_universe.c src/gfx.c src/text.c src/universe.c src/emath.c src/lines.c src/plat_ql.c src/ql_ipc.asm

# Demo de proyeccion 3D: seis naves dibujadas con LL9 (XOR)
host_demo_ship:
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o demo_ship src/demo_ship.c src/shipview.c src/ship3d.c src/shipdata.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/text.c src/plat_host.c
	./demo_ship && python3 tools/ql2png.py screen.bin demo_ship.png
ql_ship:
	qcc -O -Isrc -I/usr/local/share/qdos/include -o ship_ql src/demo_ship.c src/shipview.c src/ship3d.c src/shipdata.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/text.c src/plat_ql.c src/ql_ipc.asm

# Comparacion con el codigo ORIGINAL ejecutado en un emulador de 6502 (necesita la fuente de referencia clonada:
# git clone https://github.com/markmoxon/elite-source-code-apple-ii /tmp/moxon  o  export ELITE_SRC=<ruta>/1-source-files/main-sources)
test_orig:
	python3 tools/gen_ll9.py 3000
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_ll9 tests/test_ll9.c src/ship3d.c src/shipdata.c src/ship_math.c src/state.c src/emath.c
	./tests/test_ll9

# Juego completo (provisional: vuelo sin MVEIT/TACTICS). PC: make host_game (para pruebas); QL: make ql_game
GAME = src/main.c src/app.c src/menu.c src/chart.c src/sun.c src/dash.c src/space.c src/stars.c src/ships.c src/combat.c src/damage.c src/equip.c src/dockit.c src/tactics.c src/move.c src/flight.c src/encounters.c src/missions.c src/detok.c src/config.c src/input.c src/game.c src/cmdr.c src/save.c src/shipview.c src/ship3d.c src/shipdata.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/text.c src/universe.c src/keys.c src/sound.c
host_game:
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o elite_host $(GAME) src/keys_host.c src/sound_host.c src/plat_host.c -lm
# Cadena de qcc (tools/install_xtc68.sh la instala en /tmp/qlsdk): si existe, se pone sola en el PATH
QLSDK ?= /tmp/qlsdk
ifneq ($(wildcard $(QLSDK)/bin/qcc),)
export PATH := $(QLSDK)/bin:$(PATH)
export QLINC := $(QLSDK)/share/qdos/include
export QLLIB := $(QLSDK)/share/qdos/lib
endif

# El ejecutable QDOS "elite" (qcc anade el trailer XTcc con el dataspace; qltools lo usa para marcarlo ejecutable)
GAME_QL = $(GAME) src/keys_ql.c src/sound_ql.c src/plat_ql.c src/ql_ipc.asm
elite: $(GAME_QL) $(wildcard src/*.h)
	qcc -O -Isrc -I/usr/local/share/qdos/include -o elite $(GAME_QL) -lm
ql_game: elite

# ---- Imagen de disco DS/DD 720 KB (elite.img) -------------------------------------------------------------
# qltools (N. Dunbar) se compila desde las fuentes: git clone + make -C Unix, en tools/qltools_src (no se incluye en el tar).
# Otro qltools ya instalado:  make elite.img QLTOOLS=/ruta/qltools
QLTOOLS_REPO ?= https://github.com/NormanDunbar/qltools
QLTOOLS_DIR  ?= tools/qltools_src
QLTOOLS      ?= $(QLTOOLS_DIR)/Unix/qltools
DISK_LABEL   ?= ELITE
DISK_FILES    = data/boot data/elite_scr data/elite_dash

$(QLTOOLS):
	test -d $(QLTOOLS_DIR)/Unix || git clone --depth 1 $(QLTOOLS_REPO) $(QLTOOLS_DIR)
	$(MAKE) -C $(QLTOOLS_DIR)/Unix

qltools: $(QLTOOLS)

# Formatea (-fdd = DS/DD 720 KB) y escribe: elite (ejecutable, con su dataspace), boot, elite_scr y elite_dash.
# Dentro del disco los nombres son los de los ficheros sin el directorio: boot, elite, elite_scr, elite_dash.
elite.img: elite $(DISK_FILES) $(QLTOOLS)
	rm -f $@
	$(QLTOOLS) $@ -fdd $(DISK_LABEL)
	$(QLTOOLS) $@ -W elite $(DISK_FILES)
	$(QLTOOLS) $@ -d

disk: elite.img

.PHONY: ql_game qltools disk

# Logica de juego y sesion guiada por guion (PC): guarda capturas en tests/shots/
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

clean:
	rm -f tests/test_trade tests/test_laser tests/vectors_laser.txt tests/test_damage tests/vectors_damage.txt tests/test_planet tests/vectors_planet.txt tests/test_equip tests/vectors_equip.txt tests/test_tactics tests/vectors_tactics.txt tests/test_missions tests/vectors_missions.txt tests/test_univ6502 tests/vectors_univ.txt tests/test_shipview tests/vectors_trade.txt elite_host demo_ship demo_universe demo_keys_host test_gfx test_text screen.bin *.png tests/test_emath tests/test_emath2 tests/test_state tests/test_keys tests/test_sound tests/test_ll9 tests/test_save tests/test_game tests/dump_universe tests/oracle/oracle tests/shots/*.bin tests/tmp/* elite elite.img

test_shipview:
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_shipview tests/test_shipview.c src/shipview.c src/ship3d.c src/shipdata.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/plat_host.c
	./tests/test_shipview

# Comercio contra el 6502 original (tnpr, TT219, TT208, GCASH, LCASH, MCASH). Requiere ELITE_SRC (tools/README_ref.txt)
test_trade:
	python3 tools/gen_trade.py 20000 7
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_trade tests/test_trade.c src/game.c src/cmdr.c src/state.c src/universe.c src/emath.c
	./tests/test_trade

test_space:
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_space tests/test_space.c src/space.c src/stars.c src/ships.c src/combat.c src/damage.c src/equip.c src/dockit.c src/tactics.c src/move.c src/sun.c src/chart.c src/ship3d.c src/shipdata.c src/dash.c src/game.c src/cmdr.c src/save.c src/universe.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/text.c src/keys.c src/keys_host.c src/sound.c src/sound_host.c src/plat_host.c
	./tests/test_space

# Laser y danos contra el 6502 original (HITCH, ANGRY, LASLI, MA68, MLOOP, MA77/SHD, MA47, MA15, SFS1). Requiere ELITE_SRC
test_laser:
	python3 tools/gen_laser.py 3000 7
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_laser tests/test_laser.c src/combat.c src/damage.c src/equip.c src/dockit.c src/tactics.c src/space.c src/stars.c src/ships.c src/move.c src/sun.c src/chart.c src/ship3d.c src/shipdata.c src/dash.c src/game.c src/cmdr.c src/save.c src/universe.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/text.c src/keys.c src/keys_host.c src/sound.c src/sound_host.c src/plat_host.c
	./tests/test_laser

# Danos a nuestra nave contra el 6502 original (OOPS, OUCH, partes 9, 10 y 15, D1 de DEATH). Requiere ELITE_SRC
test_damage:
	python3 tools/gen_damage.py 3000 7
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_damage tests/test_damage.c src/combat.c src/damage.c src/equip.c src/dockit.c src/tactics.c src/space.c src/stars.c src/ships.c src/move.c src/sun.c src/chart.c src/ship3d.c src/shipdata.c src/dash.c src/game.c src/cmdr.c src/save.c src/universe.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/text.c src/keys.c src/keys_host.c src/sound.c src/sound_host.c src/plat_host.c
	./tests/test_damage

# Planetas y soles contra el 6502 original (PLANET, SUN, CIRCLE, BLINE, WPLS2, LL145...). Requiere ELITE_SRC
test_planet:
	python3 tools/gen_planet.py 400 7
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_planet tests/test_planet.c src/combat.c src/damage.c src/equip.c src/dockit.c src/tactics.c src/space.c src/stars.c src/ships.c src/move.c src/sun.c src/chart.c src/ship3d.c src/shipdata.c src/dash.c src/game.c src/cmdr.c src/save.c src/universe.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/text.c src/keys.c src/keys_host.c src/sound.c src/sound_host.c src/plat_host.c
	./tests/test_planet

# Equipo contra el 6502 original (bomba, WARP, DOCKIT, DOKEY con ordenador de atraque, GTHG, FRS1). Requiere ELITE_SRC
test_equip:
	python3 tools/gen_equip.py 3000 7
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_equip tests/test_equip.c src/combat.c src/damage.c src/equip.c src/dockit.c src/tactics.c src/space.c src/stars.c src/ships.c src/move.c src/sun.c src/chart.c src/ship3d.c src/shipdata.c src/dash.c src/game.c src/cmdr.c src/save.c src/universe.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/text.c src/keys.c src/keys_host.c src/sound.c src/sound_host.c src/plat_host.c
	./tests/test_equip

# TACTICS contra el 6502 original. Requiere ELITE_SRC
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

# Cabina editable en el PC: exporta el panel a PNG (data/cabina.png, 1024 x 112) y lo vuelve a convertir en data/elite_dash
dash_png:
	python3 tools/png2dash.py --export data/elite_dash data/cabina.png
dash_from_png:
	python3 tools/png2dash.py data/cabina.png data/elite_dash
test_dashpng:
	mkdir -p tests/tmp
	python3 tools/png2dash.py --export data/elite_dash tests/tmp/cabina.png
	python3 tools/png2dash.py tests/tmp/cabina.png tests/tmp/elite_dash_png
	cmp data/elite_dash tests/tmp/elite_dash_png && echo "png2dash: ida y vuelta identica"

# Fichero de fondo del panel (data/elite_dash) a partir del volcado de pantalla del Apple II (necesita ELITE_SRC)
dash_file:
	python3 tools/gen_dashboard.py

test_all: test test_keys test_sound test_universe test_save test_game test_shipview test_space test_dashpng

# Compara el codigo de c68 (-int=16 -O) con el del PC bajo qemu (necesita c68, gcc-m68k-linux-gnu y qemu-user; ver tools/c68_check/run.sh)
c68_check:
	tools/c68_check/run.sh

# Start and end: RESET, RES2 y DFAULT contra el codigo ORIGINAL en el emulador de 6502 (necesita ELITE_SRC)
test_start:
	python3 tools/gen_start.py 1000
	gcc -std=c89 -pedantic -O2 -Isrc -o tests/test_start tests/test_start.c src/app.c src/menu.c src/chart.c src/sun.c src/dash.c src/space.c src/stars.c src/ships.c src/combat.c src/damage.c src/equip.c src/dockit.c src/tactics.c src/move.c src/flight.c src/encounters.c src/missions.c src/detok.c src/config.c src/input.c src/game.c src/cmdr.c src/save.c src/shipview.c src/ship3d.c src/shipdata.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/text.c src/universe.c src/keys.c src/sound.c src/keys_host.c src/sound_host.c src/plat_host.c -lm
	./tests/test_start
