# Pruebas en PC:  make host        QL con XTC68:  make ql
SRC = src/test_gfx.c src/gfx.c

host:
	gcc -std=c89 -pedantic -Wall -O2 -Isrc -o test_gfx $(SRC) src/plat_host.c -lm
	./test_gfx && python3 tools/ql2png.py screen.bin test.png

# -I va ANTES de los ficheros. -Isrc es necesario porque qcc busca "platform.h" en el directorio actual.
ql:
	qcc -O -Isrc -I/usr/local/share/qdos/include -o elite_ql $(SRC) src/plat_ql.c -lm
