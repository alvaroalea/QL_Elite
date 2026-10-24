# QL (XTC68): make elite / make disk (imagen elite.img) / make elite.mdv (microdrive)    PC: make host_game    Pruebas: make test_all
# Orden: 1) compilacion para el QL  2) generacion de assets  3) compilacion en el PC  4) pruebas
.DEFAULT_GOAL := disk
# Version que sale en la pantalla de titulo ("R <VERSION>"): make VERSION=pre0.3
VERSION ?= 0.2
# Fuente de referencia (Mark Moxon): carpeta ref/moxon-appleII/ del proyecto (y ref/moxon-bbc-a/ con la version A de BBC) (ignorada por git); ELITE_SRC del entorno tiene prioridad
export ELITE_SRC ?= $(CURDIR)/ref/moxon-appleII/1-source-files/main-sources
# Ejecutables, imagen de disco y capturas generadas van a build/
BUILD ?= build
# Funciones de depuracion (SHIFT-1/2/3 compra/venta/combustible gratis, SHIFT-C/P/H en vuelo): make DEBUG= para quitarlas
DEBUG ?= 1
DBGDEF = $(if $(DEBUG),-DDEBUG)
# Linea de Bresenham en ensamblador en el QL (src/ql_line.asm); make ASM_LINE= para usar la version en C
ASM_LINE ?= 1
ASMDEF = $(if $(ASM_LINE),-DASM_LINE)
# LL9 en ensamblador (src/ql_ll9.asm: caras, vertices y LL51): make ASM_LL9= para usar el C
ASM_LL9 ?= 1
ASMDEF += $(if $(ASM_LL9),-DASM_LL9)
# Version sin sonido: snd_ipc no hace nada (ni IPC, ni ZX8302, ni ROM): make NOSOUND=1 (por defecto con sonido; ver CLAUDE.md 2026-10-22)
NOSOUND ?=
# Sin el reloj de 50 Hz (MT.LPOLL) ni el limite de fps: make NOTICK=1 (diagnostico de cuelgues; por defecto con reloj)
SNDDEF = $(if $(NOSOUND),-DNOSOUND) $(if $(NOTICK),-DNOTICK)
VERDEF = $(DBGDEF) $(SNDDEF) -DELITE_VERSION='"$(VERSION)"'
COMMON = src/gfx.c src/text.c

# ==== 1. Compilacion para el QL ===========================================================================

# Cadena de qcc (tools/install_xtc68.sh la instala en /tmp/qlsdk): si existe, se pone sola en el PATH
QLSDK ?= /tmp/qlsdk
ifneq ($(wildcard $(QLSDK)/bin/qcc),)
export PATH := $(QLSDK)/bin:$(PATH)
export QLINC := $(QLSDK)/share/qdos/include
export QLLIB := $(QLSDK)/share/qdos/lib
endif

# Juego completo. QL: make elite (o make disk para la imagen); PC: make host_game (para pruebas)
GAME = src/main.c src/app.c src/menu.c src/chart.c src/sun.c src/dash.c src/space.c src/stars.c src/ships.c src/combat.c src/damage.c src/equip.c src/dockit.c src/tactics.c src/move.c src/flight.c src/encounters.c src/missions.c src/detok.c src/config.c src/input.c src/game.c src/cmdr.c src/save.c src/shipview.c src/ship3d.c src/shipdata.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/text.c src/universe.c src/keys.c src/sound.c

# El ejecutable QDOS "elite" (qcc anade el trailer XTcc con el dataspace; qltools lo usa para marcarlo ejecutable)
GAME_QL = $(GAME) src/keys_ql.c src/sound_ql.c src/plat_ql.c src/ql_ipc.asm src/ql_line.asm src/ql_ell.asm src/ql_ll9.asm src/ql_lltab.asm src/ql_ellrows.asm
$(BUILD)/elite: $(GAME_QL) $(wildcard src/*.h) Makefile
	mkdir -p $(BUILD)
	qcc -O $(VERDEF) $(ASMDEF) -Isrc -I/usr/local/share/qdos/include -o $@ $(GAME_QL) -lm
elite: $(BUILD)/elite

# ---- Imagen de disco DS/DD 720 KB (elite.img) -------------------------------------------------------------
# qltools (N. Dunbar) se compila desde las fuentes: git clone + make -C Unix, en tmp/qltools_src (no se incluye en el tar).
# Otro qltools ya instalado:  make elite.img QLTOOLS=/ruta/qltools
QLTOOLS_REPO ?= https://github.com/NormanDunbar/qltools
QLTOOLS_DIR  ?= tmp/qltools_src
QLTOOLS      ?= $(QLTOOLS_DIR)/Unix/qltools
DISK_LABEL   ?= ELITE
DISK_FILES    = data/boot data/elite_scr data/elite_dash

$(QLTOOLS):
	test -d $(QLTOOLS_DIR)/Unix || git clone --depth 1 $(QLTOOLS_REPO) $(QLTOOLS_DIR)
	$(MAKE) -C $(QLTOOLS_DIR)/Unix

qltools: $(QLTOOLS)

# Formatea (-fdd = DS/DD 720 KB) y escribe: elite (ejecutable, con su dataspace), boot, elite_scr y elite_dash.
# Dentro del disco los nombres son los de los ficheros sin el directorio: boot, elite, elite_scr, elite_dash.
# Se escribe desde build/ para que en el disco el ejecutable se llame elite (qltools usa el nombre sin directorio)
$(BUILD)/elite.img: $(BUILD)/elite $(DISK_FILES) $(QLTOOLS)
	rm -f $@
	$(QLTOOLS) $@ -fdd $(DISK_LABEL)
	$(QLTOOLS) $@ -W $(BUILD)/elite $(DISK_FILES)
	$(QLTOOLS) $@ -d

elite.img: $(BUILD)/elite.img
disk: elite.img

# ---- Imagen de microdrive en formato QLAY (elite.mdv) -----------------------------------------------------
# mdvtool (T. Harbaum / xXorAa): el del PATH o, si no hay, se compila en tmp/mdvtool (necesita libzip-dev).
# Otro mdvtool:  make elite.mdv MDVTOOL=/ruta/mdvtool
# No esta en el objetivo por defecto: un cartucho QLAY tiene 255 sectores de 512 B (253 libres para ficheros,
# ~126 KB; un cartucho real suele dar menos) y hoy el ejecutable ocupa ~147 KB. La regla comprueba el tamano
# antes de llamar a mdvtool, que con una imagen llena termina con violacion de segmento.
MDVTOOL_REPO ?= https://github.com/xxoraa/mdvtool
MDVTOOL_DIR  ?= tmp/mdvtool
MDVTOOL      ?= $(or $(shell command -v mdvtool 2>/dev/null),$(MDVTOOL_DIR)/mdvtool)
MDV_LABEL    ?= ELITE
MDV_SECTORS  ?= 253

$(MDVTOOL_DIR)/mdvtool:
	test -d $(MDVTOOL_DIR) || git clone --depth 1 $(MDVTOOL_REPO) $(MDVTOOL_DIR)
	$(MAKE) -C $(MDVTOOL_DIR)

mdvtool: $(MDVTOOL)

# mdvtool guarda el nombre tal como se le pasa: se copian los ficheros a build/mdv/ y se importan desde alli.
# El boot del microdrive es el del disco con dev$="mdv1_". Cada fichero lleva 64 B de cabecera QDOS.
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

# Demos para el QL (texto, teclado, sonido, ficha de sistema y proyeccion 3D).
# -Isrc necesario: qcc busca "platform.h" en el directorio actual. "ql" es la prueba de texto.
ql:
	mkdir -p $(BUILD)
	qcc -O -Isrc -I/usr/local/share/qdos/include -o $(BUILD)/elite_ql src/test_text.c $(COMMON) src/plat_ql.c src/keys.c src/keys_ql.c src/ql_ipc.asm -lm
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

# ==== 2. Generacion de assets =============================================================================

# Cabina editable en el PC: exporta el panel a PNG (asset/cabina.png, 1024 x 112) y lo vuelve a convertir en data/elite_dash
dash_png:
	python3 tools/png2dash.py --export data/elite_dash asset/cabina.png
dash_from_png:
	python3 tools/png2dash.py asset/cabina.png data/elite_dash

# Fichero de fondo del panel (data/elite_dash) a partir del volcado de pantalla del Apple II (necesita ELITE_SRC)
dash_file:
	python3 tools/gen_dashboard.py

# ==== 3. Compilacion en el PC =============================================================================

# Juego completo en el PC (pantalla en memoria)
host_game:
	mkdir -p $(BUILD)
	gcc -std=c89 -pedantic -Wall -O2 $(VERDEF) -Isrc -o $(BUILD)/elite_host $(GAME) src/keys_host.c src/sound_host.c src/plat_host.c -lm

# Demos en el PC: grafica/texto, teclado, ficha de sistema y proyeccion 3D
host:
	mkdir -p $(BUILD)
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o $(BUILD)/test_gfx src/test_gfx.c src/gfx.c src/plat_host.c -lm
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o $(BUILD)/test_text src/test_text.c $(COMMON) src/plat_host.c -lm
	./$(BUILD)/test_gfx && python3 tools/ql2png.py screen.bin $(BUILD)/test.png
	./$(BUILD)/test_text && python3 tools/ql2png.py screen.bin $(BUILD)/test_text.png

# Demo de teclado: en el PC dibuja un cuadro con una pulsacion simulada; en el QL es interactiva
host_demo_keys:
	mkdir -p $(BUILD)
	gcc -std=c89 -pedantic -Wall -O2 -DHOST_DEMO -Isrc -o $(BUILD)/demo_keys_host src/demo_keys.c src/gfx.c src/text.c src/keys.c src/keys_host.c src/plat_host.c

# Demo de ficha de sistema (datos, descripcion y mercado)
host_demo_universe:
	mkdir -p $(BUILD)
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o $(BUILD)/demo_universe src/demo_universe.c src/gfx.c src/text.c src/universe.c src/emath.c src/lines.c src/plat_host.c
	./$(BUILD)/demo_universe && python3 tools/ql2png.py screen.bin $(BUILD)/demo_universe.png

# Demo de proyeccion 3D: seis naves dibujadas con LL9 (XOR)
host_demo_ship:
	mkdir -p $(BUILD)
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o $(BUILD)/demo_ship src/demo_ship.c src/shipview.c src/ship3d.c src/shipdata.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/text.c src/plat_host.c
	./$(BUILD)/demo_ship && python3 tools/ql2png.py screen.bin $(BUILD)/demo_ship.png

clean:
	rm -f tests/test_trade tests/test_laser tests/vectors_laser.txt tests/test_damage tests/vectors_damage.txt tests/test_planet tests/vectors_planet.txt tests/test_equip tests/vectors_equip.txt tests/test_tactics tests/vectors_tactics.txt tests/test_missions tests/vectors_missions.txt tests/test_univ6502 tests/vectors_univ.txt tests/test_shipview tests/vectors_trade.txt screen.bin tests/test_emath tests/test_emath2 tests/test_state tests/test_keys tests/test_sound tests/test_ll9 tests/test_save tests/test_game tests/dump_universe tests/oracle/oracle tests/shots/*.bin tests/tmp/*
	rm -rf $(BUILD)

# ==== 4. Pruebas ==========================================================================================

test_all: test test_keys test_sound test_universe test_save test_game test_shipview test_space test_dashpng test_bres test_lineasm test_ellipse test_nativeview test_arcs test_ellasm test_dust test_ll9asm test_ellrows test_ellrowsasm

# Compara el codigo de c68 (-int=16 -O) con el del PC bajo qemu (necesita c68, gcc-m68k-linux-gnu y qemu-user; ver tools/c68_check/run.sh)
c68_check:
	tools/c68_check/run.sh

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

# Pruebas de sonido (bloques IPC y prioridades)
test_sound:
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_sound tests/test_sound.c src/sound.c src/sound_host.c -lm
	./tests/test_sound

# Universo: construye el oraculo desde TXTELITE.C 1.5, vuelca ambos y compara
test_universe:
	python3 tools/gen_universe_text.py
	python3 tools/make_oracle.py
	gcc -std=gnu89 -w -funsigned-char -o tests/oracle/oracle tests/oracle/oracle.c -lm
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/dump_universe tests/dump_universe.c src/universe.c src/emath.c
	./tests/oracle/oracle > tests/oracle/oracle.txt
	./tests/dump_universe > tests/oracle/ours.txt
	python3 tests/compare_universe.py tests/oracle/oracle.txt tests/oracle/ours.txt

# Comparacion con el codigo ORIGINAL ejecutado en un emulador de 6502 (necesita la fuente de referencia clonada:
# git clone https://github.com/markmoxon/elite-source-code-apple-ii ref/moxon-appleII  o  export ELITE_SRC=<ruta>/1-source-files/main-sources)
test_orig:
	python3 tools/gen_ll9.py 3000
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_ll9 tests/test_ll9.c src/ship3d.c src/shipdata.c src/ship_math.c src/state.c src/emath.c
	./tests/test_ll9

# Texto bajo el panel de vuelo: pintar encima de otro estado (vaciando solo el rectangulo que sobra) = pintar desde cero
test_panel:
	gcc -std=c89 -O2 -Isrc -o tests/test_panel tests/test_panel.c src/app.c src/menu.c src/chart.c src/sun.c src/dash.c src/space.c src/stars.c src/ships.c src/combat.c src/damage.c src/equip.c src/dockit.c src/tactics.c src/move.c src/encounters.c src/missions.c src/detok.c src/config.c src/input.c src/game.c src/cmdr.c src/save.c src/shipview.c src/ship3d.c src/shipdata.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/text.c src/universe.c src/keys.c src/sound.c src/keys_host.c src/sound_host.c src/plat_host.c -lm
	./tests/test_panel

# Polvo estelar (tamanos por distancia, borrado, bordes) y PIXEL original intacto
test_dust:
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_dust tests/test_dust.c src/gfx.c
	./tests/test_dust

# ql_ell.asm (elipse y tramo horizontal en ensamblador) ejecutada con tools/m68k_mini.py contra gfx_ellipse_xor y gfx_hline_xor en C
test_ellasm:
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/ell_dump tests/ell_dump.c src/gfx.c
	python3 tools/test_ellasm.py 700

# Contorno de la elipse fila a fila (gfx_ell_update, sin tabla = punto medio): misma pantalla que dibujar la vieja y la nueva con gfx_ellipse_xor
test_ellrows:
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_ellrows tests/test_ellrows.c src/gfx.c src/plat_host.c -lm
	./tests/test_ellrows

# ql_ellrows.asm ejecutada con tools/m68k_mini.py contra gfx_ell_update en C
test_ellrowsasm:
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/ellrows_dump tests/ellrows_dump.c src/gfx.c src/plat_host.c -lm
	python3 tools/test_ellrowsasm.py 200 1

# ql_ll9.asm (LL51, caras y vertices de LL9 en ensamblador) ejecutada con tools/m68k_mini.py contra ship_ll9_faces_c y ship_ll9_verts_c (naves reales, casos limite)
test_ll9asm:
	python3 tools/gen_lltab.py
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/ll9_dump tests/ll9_dump.c src/ship3d.c src/shipdata.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/plat_host.c -lm
	python3 tools/test_ll9asm.py 300 1

# Vista nativa de vuelo: sol por filas (invariante pantalla = tramos), planeta (salto de redibujado), anillos del tunel y borrado exacto
test_nativeview:
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_nativeview tests/test_nativeview.c src/ships.c src/move.c src/sun.c src/ship3d.c src/shipdata.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/combat.c src/damage.c src/equip.c src/dockit.c src/tactics.c src/space.c src/stars.c src/dash.c src/game.c src/cmdr.c src/save.c src/universe.c src/text.c src/keys.c src/keys_host.c src/sound.c src/sound_host.c src/plat_host.c src/chart.c -lm
	./tests/test_nativeview

# Meridianos y crater del planeta (vista nativa): sin restos al quitar, figura estable, crater oculto
test_arcs:
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_arcs tests/test_arcs.c src/ships.c src/move.c src/sun.c src/ship3d.c src/shipdata.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/combat.c src/damage.c src/equip.c src/dockit.c src/tactics.c src/space.c src/stars.c src/dash.c src/game.c src/cmdr.c src/save.c src/universe.c src/text.c src/keys.c src/keys_host.c src/sound.c src/sound_host.c src/plat_host.c src/chart.c -lm
	./tests/test_arcs

# Elipse de punto medio (planeta y anillos del tunel en la vista nativa): ejes, simetria sin puntos borrados por XOR, cercania a la elipse ideal, borrado exacto y recorte
test_ellipse:
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_ellipse tests/test_ellipse.c src/gfx.c -lm
	./tests/test_ellipse

# Linea de Bresenham del juego (extremos, simetria, borrado exacto, cercania a la recta) y su version en ensamblador (ql_line.asm)
# ejecutada con el interprete de 68000 de tools/m68k_mini.py contra la version en C
test_bres:
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_bres tests/test_bres.c src/lines.c src/emath.c src/gfx.c -lm
	./tests/test_bres

test_lineasm: test_bres
	python3 tools/test_lineasm.py 3000

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

# Cabina editable: PNG -> elite_dash -> PNG da el mismo fichero
test_dashpng:
	mkdir -p tests/tmp
	python3 tools/png2dash.py --export data/elite_dash tests/tmp/cabina.png
	python3 tools/png2dash.py tests/tmp/cabina.png tests/tmp/elite_dash_png
	cmp data/elite_dash tests/tmp/elite_dash_png && echo "png2dash: ida y vuelta identica"

test_shipview:
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_shipview tests/test_shipview.c src/shipview.c src/ship3d.c src/shipdata.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/plat_host.c
	./tests/test_shipview

# Comercio contra el 6502 original (tnpr, TT219, TT208, GCASH, LCASH, MCASH). Requiere ELITE_SRC (ref/, tools/README_ref.txt)
test_trade:
	python3 tools/gen_trade.py 20000 7
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_trade tests/test_trade.c src/combat.c src/damage.c src/equip.c src/dockit.c src/tactics.c src/space.c src/stars.c src/ships.c src/move.c src/sun.c src/chart.c src/ship3d.c src/shipdata.c src/dash.c src/game.c src/cmdr.c src/save.c src/universe.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/text.c src/keys.c src/keys_host.c src/sound.c src/sound_host.c src/plat_host.c
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

# Start and end: RESET, RES2 y DFAULT contra el codigo ORIGINAL en el emulador de 6502 (necesita ELITE_SRC)
test_start:
	python3 tools/gen_start.py 1000
	gcc -std=c89 -pedantic -O2 -Isrc -o tests/test_start tests/test_start.c src/app.c src/menu.c src/chart.c src/sun.c src/dash.c src/space.c src/stars.c src/ships.c src/combat.c src/damage.c src/equip.c src/dockit.c src/tactics.c src/move.c src/flight.c src/encounters.c src/missions.c src/detok.c src/config.c src/input.c src/game.c src/cmdr.c src/save.c src/shipview.c src/ship3d.c src/shipdata.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/text.c src/universe.c src/keys.c src/sound.c src/keys_host.c src/sound_host.c src/plat_host.c -lm
	./tests/test_start

# Sound: DORND gastados por cada sonido, RAND y acarreo contra el codigo ORIGINAL en el emulador (necesita ELITE_SRC)
test_soundorig:
	python3 tools/gen_sound.py 300
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_sound_orig tests/test_sound_orig.c src/sound.c src/sound_host.c src/emath.c -lm
	./tests/test_sound_orig

# Moving: MVT1, MVS4, MVS5, MVT6 y MVEIT (con MV40, MV45, TIDY, SCAN y TACTICS registrados) contra el codigo ORIGINAL (necesita ELITE_SRC)
test_move:
	python3 tools/gen_move.py 3000
	gcc -std=c89 -pedantic -O2 -Isrc -o tests/test_move tests/test_move.c src/app.c src/menu.c src/chart.c src/sun.c src/dash.c src/space.c src/stars.c src/ships.c src/combat.c src/damage.c src/equip.c src/dockit.c src/tactics.c src/move.c src/flight.c src/encounters.c src/missions.c src/detok.c src/config.c src/input.c src/game.c src/cmdr.c src/save.c src/shipview.c src/ship3d.c src/shipdata.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/text.c src/universe.c src/keys.c src/sound.c src/keys_host.c src/sound_host.c src/plat_host.c -lm
	./tests/test_move

# Equipment: EQSHP (equip_buy, equip_price) contra el codigo ORIGINAL en el emulador de 6502 (necesita ELITE_SRC)
test_eqshp:
	python3 tools/gen_eqshp.py 3000
	gcc -std=c89 -pedantic -O2 -Isrc -o tests/test_eqshp tests/test_eqshp.c src/app.c src/menu.c src/chart.c src/sun.c src/dash.c src/space.c src/stars.c src/ships.c src/combat.c src/damage.c src/equip.c src/dockit.c src/tactics.c src/move.c src/flight.c src/encounters.c src/missions.c src/detok.c src/config.c src/input.c src/game.c src/cmdr.c src/save.c src/shipview.c src/ship3d.c src/shipdata.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/text.c src/universe.c src/keys.c src/sound.c src/keys_host.c src/sound_host.c src/plat_host.c -lm
	./tests/test_eqshp

# Keyboard: DK4 (FREEZE) y DKS3 contra el codigo ORIGINAL en el emulador de 6502 (necesita ELITE_SRC)
test_dk4:
	python3 tools/gen_keys.py 2000
	gcc -std=c89 -pedantic -O2 -Isrc -o tests/test_dk4 tests/test_dk4.c src/app.c src/menu.c src/chart.c src/sun.c src/dash.c src/space.c src/stars.c src/ships.c src/combat.c src/damage.c src/equip.c src/dockit.c src/tactics.c src/move.c src/flight.c src/encounters.c src/missions.c src/detok.c src/config.c src/input.c src/game.c src/cmdr.c src/save.c src/shipview.c src/ship3d.c src/shipdata.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/text.c src/universe.c src/keys.c src/sound.c src/keys_host.c src/sound_host.c src/plat_host.c -lm
	./tests/test_dk4

# Save and load: el comandante JAMESON de fabrica (NA2%) contra la fuente original (necesita ELITE_SRC)
test_jameson:
	python3 tools/gen_jameson.py
	gcc -std=c89 -pedantic -O2 -Isrc -o tests/test_jameson tests/test_jameson.c src/app.c src/menu.c src/chart.c src/sun.c src/dash.c src/space.c src/stars.c src/ships.c src/combat.c src/damage.c src/equip.c src/dockit.c src/tactics.c src/move.c src/flight.c src/encounters.c src/missions.c src/detok.c src/config.c src/input.c src/game.c src/cmdr.c src/save.c src/shipview.c src/ship3d.c src/shipdata.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/text.c src/universe.c src/keys.c src/sound.c src/keys_host.c src/sound_host.c src/plat_host.c -lm
	./tests/test_jameson

# Dashboard: DIALS, COMPAS, MSBAR, msblob, SCAN, cntr, BUMP2 y REDU2 contra el codigo ORIGINAL (necesita ELITE_SRC)
test_dash:
	python3 tools/gen_dash.py 2000
	gcc -std=c89 -pedantic -O2 -Isrc -o tests/test_dash tests/test_dash.c src/app.c src/menu.c src/chart.c src/sun.c src/dash.c src/space.c src/stars.c src/ships.c src/combat.c src/damage.c src/equip.c src/dockit.c src/tactics.c src/move.c src/flight.c src/encounters.c src/missions.c src/detok.c src/config.c src/input.c src/game.c src/cmdr.c src/save.c src/shipview.c src/ship3d.c src/shipdata.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/text.c src/universe.c src/keys.c src/sound.c src/keys_host.c src/sound_host.c src/plat_host.c -lm
	./tests/test_dash

# Flight: PLUT, ESCAPE, MJP y TT110 contra el codigo ORIGINAL en el emulador de 6502 (necesita ELITE_SRC)
test_flight:
	python3 tools/gen_flight.py 1000
	gcc -std=c89 -pedantic -O2 -Isrc -o tests/test_flight tests/test_flight.c src/app.c src/menu.c src/chart.c src/sun.c src/dash.c src/space.c src/stars.c src/ships.c src/combat.c src/damage.c src/equip.c src/dockit.c src/tactics.c src/move.c src/flight.c src/encounters.c src/missions.c src/detok.c src/config.c src/input.c src/game.c src/cmdr.c src/save.c src/shipview.c src/ship3d.c src/shipdata.c src/ship_math.c src/state.c src/emath.c src/lines.c src/gfx.c src/text.c src/universe.c src/keys.c src/sound.c src/keys_host.c src/sound_host.c src/plat_host.c -lm
	./tests/test_flight

# Maths (Arithmetic) contra el codigo ORIGINAL en el emulador de 6502 (necesita ELITE_SRC)
test_arith:
	python3 tools/gen_arith.py 2000
	gcc -std=c89 -pedantic -O2 -Isrc -o tests/test_arith tests/test_arith.c src/emath.c
	./tests/test_arith


.PHONY: elite elite.img elite.mdv disk qltools mdvtool ql ql_keys ql_sound ql_universe ql_ship dash_png dash_from_png dash_file host host_demo_keys host_demo_universe host_demo_ship host_game clean test_all c68_check test_bres test_lineasm test_ellipse test_nativeview test_arcs test_ellasm test_dust test_panel
