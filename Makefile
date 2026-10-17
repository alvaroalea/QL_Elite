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
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o demo_universe src/demo_universe.c src/gfx.c src/text.c src/universe.c src/emath.c src/plat_host.c
	./demo_universe && python3 tools/ql2png.py screen.bin demo_universe.png
ql_universe:
	qcc -O -Isrc -I/usr/local/share/qdos/include -o universe_ql src/demo_universe.c src/gfx.c src/text.c src/universe.c src/emath.c src/plat_ql.c src/ql_ipc.asm

# Demo de proyeccion 3D: seis naves dibujadas con LL9 (XOR)
host_demo_ship:
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o demo_ship src/demo_ship.c src/shipview.c src/ship3d.c src/shipdata.c src/ship_math.c src/state.c src/emath.c src/gfx.c src/text.c src/plat_host.c
	./demo_ship && python3 tools/ql2png.py screen.bin demo_ship.png
ql_ship:
	qcc -O -Isrc -I/usr/local/share/qdos/include -o ship_ql src/demo_ship.c src/shipview.c src/ship3d.c src/shipdata.c src/ship_math.c src/state.c src/emath.c src/gfx.c src/text.c src/plat_ql.c src/ql_ipc.asm

# Comparacion con el codigo ORIGINAL ejecutado en un emulador de 6502 (necesita la fuente de referencia clonada:
# git clone https://github.com/markmoxon/elite-source-code-apple-ii /tmp/moxon  o  export ELITE_SRC=<ruta>/1-source-files/main-sources)
test_orig:
	python3 tools/gen_ll9.py 3000
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_ll9 tests/test_ll9.c src/ship3d.c src/shipdata.c src/ship_math.c src/state.c src/emath.c
	./tests/test_ll9

# Juego completo (provisional: vuelo sin MVEIT/TACTICS). PC: make host_game (para pruebas); QL: make ql_game
GAME = src/main.c src/app.c src/menu.c src/chart.c src/sun.c src/dash.c src/space.c src/stars.c src/flight.c src/input.c src/game.c src/cmdr.c src/save.c src/shipview.c src/ship3d.c src/shipdata.c src/ship_math.c src/state.c src/emath.c src/gfx.c src/text.c src/universe.c src/keys.c src/sound.c
host_game:
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o elite_host $(GAME) src/keys_host.c src/sound_host.c src/plat_host.c -lm
ql_game:
	qcc -O -Isrc -I/usr/local/share/qdos/include -o elite $(GAME) src/keys_ql.c src/sound_ql.c src/plat_ql.c src/ql_ipc.asm -lm

# Logica de juego y sesion guiada por guion (PC): guarda capturas en tests/shots/
test_game:
	mkdir -p tests/shots tests/tmp
	cp data/elite_dash tests/tmp/elite_dash
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_game tests/test_game.c src/app.c src/menu.c src/chart.c src/sun.c src/dash.c src/space.c src/stars.c src/flight.c src/input.c src/game.c src/cmdr.c src/save.c src/shipview.c src/ship3d.c src/shipdata.c src/ship_math.c src/state.c src/emath.c src/gfx.c src/text.c src/universe.c src/keys.c src/sound.c src/keys_host.c src/sound_host.c src/plat_host.c -lm
	./tests/test_game

test_save:
	python3 tools/gen_vectors_chk.py
	mkdir -p tests/tmp
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_save tests/test_save.c src/cmdr.c src/save.c src/state.c
	./tests/test_save

clean:
	rm -f tests/test_trade tests/test_shipview tests/vectors_trade.txt elite_host demo_ship demo_universe demo_keys_host test_gfx test_text screen.bin *.png tests/test_emath tests/test_emath2 tests/test_state tests/test_keys tests/test_sound tests/test_ll9 tests/test_save tests/test_game tests/dump_universe tests/oracle/oracle tests/shots/*.bin tests/tmp/*

test_shipview:
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_shipview tests/test_shipview.c src/shipview.c src/ship3d.c src/shipdata.c src/ship_math.c src/state.c src/emath.c src/gfx.c src/plat_host.c
	./tests/test_shipview

# Comercio contra el 6502 original (tnpr, TT219, TT208, GCASH, LCASH, MCASH). Requiere ELITE_SRC (tools/README_ref.txt)
test_trade:
	python3 tools/gen_trade.py 20000 7
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_trade tests/test_trade.c src/game.c src/cmdr.c src/state.c src/universe.c src/emath.c
	./tests/test_trade

test_space:
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o tests/test_space tests/test_space.c src/space.c src/stars.c src/sun.c src/chart.c src/ship3d.c src/shipdata.c src/dash.c src/game.c src/cmdr.c src/save.c src/universe.c src/ship_math.c src/state.c src/emath.c src/gfx.c src/text.c src/keys.c src/keys_host.c src/plat_host.c
	./tests/test_space

# Fichero de fondo del panel (data/elite_dash) a partir del volcado de pantalla del Apple II (necesita ELITE_SRC)
dash_file:
	python3 tools/gen_dashboard.py

test_all: test test_keys test_sound test_universe test_save test_game test_shipview test_space

# Compara el codigo de c68 (-int=16 -O) con el del PC bajo qemu (necesita c68, gcc-m68k-linux-gnu y qemu-user; ver tools/c68_check/run.sh)
c68_check:
	tools/c68_check/run.sh
