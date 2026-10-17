# Pruebas en PC:  make host     |  QL con c68:  make ql  (ajustar CC68/LD68 a tu toolchain)
host:
	gcc -O2 -Wall -o test_gfx src/test_gfx.c src/gfx.c src/plat_host.c -lm
	./test_gfx && python3 tools/ql2png.py screen.bin test.png
ql:
	cc68 -O -o elite_ql src/test_gfx.c src/gfx.c src/plat_ql.c
