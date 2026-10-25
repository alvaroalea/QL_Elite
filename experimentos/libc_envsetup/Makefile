# Experimento: libc.a de c68 4.24f con el modulo _envsetup corregido (cuelgue en Minerva sin Toolkit II).
#   make          -> lib/libc.a corregida (copia de orig/libc.a con _envsetup sustituido)
#   make check    -> 1) la transcripcion fiel (envsetup_orig.asm) da un ejecutable identico al de la libc original
#                    2) en lib/libc.a el modulo _envsetup es el corregido y el numero de modulos no cambia
# Uso: qcc ... -L<ruta>/lib/ (la carpeta va delante de la del sistema). Un _envsetup definido en el propio
# programa (como el de src/plat_ql.c) tiene prioridad sobre el de cualquier biblioteca.
# Notas: as68 pone como nombre del modulo SROFF el nombre del fichero de salida tal cual (con directorio y
# extension), y slb -r solo sustituye un modulo con el mismo nombre (si no, lo anade o no hace nada). Por eso
# se ensambla dentro del directorio de destino a un fichero llamado "_envsetup" y despues se copia a .o.

# libc.a de partida: la copia guardada en orig/ (la de /usr/local/share/qdos/lib/libc.a sin tocar), para que el
# resultado no dependa de lo instalado. Otra: make SYSLIB=/ruta/libc.a
SYSLIB ?= orig/libc.a
QLINC  ?= /usr/local/share/qdos/include
B      := build
TOP    := $(CURDIR)

all: lib/libc.a

# modulo corregido
$(B)/fix/_envsetup.o: envsetup.asm
	mkdir -p $(B)/fix
	cd $(B)/fix && as68 $(TOP)/envsetup.asm _envsetup && cp _envsetup _envsetup.o

# modulo transcrito fiel (solo para la comprobacion)
$(B)/orig/_envsetup.o: envsetup_orig.asm
	mkdir -p $(B)/orig
	cd $(B)/orig && as68 $(TOP)/envsetup_orig.asm _envsetup && cp _envsetup _envsetup.o

lib/libc.a: $(B)/fix/_envsetup.o $(SYSLIB)
	mkdir -p lib
	cp $(SYSLIB) lib/libc.a
	cd $(B)/fix && slb -r $(TOP)/lib/libc.a _envsetup.o

$(B)/libtra/libc.a: $(B)/orig/_envsetup.o $(SYSLIB)
	mkdir -p $(B)/libtra
	cp $(SYSLIB) $(B)/libtra/libc.a
	cd $(B)/orig && slb -r $(TOP)/$(B)/libtra/libc.a _envsetup.o

$(B)/prueba.o: prueba.c
	mkdir -p $(B)
	qcc -c -O -I$(QLINC) -o $@ prueba.c

check: $(B)/prueba.o $(B)/libtra/libc.a lib/libc.a
	cd $(B) && qld -ms -L$(dir $(abspath $(SYSLIB))) -o ref prueba.o >/dev/null
	cd $(B) && qld -ms -L$(TOP)/$(B)/libtra/ -o tra prueba.o >/dev/null
	cmp $(B)/ref $(B)/tra && echo "OK: la transcripcion da un ejecutable identico"
	mkdir -p $(B)/ext_tra $(B)/ext_fix
	cd $(B)/ext_tra && slb -x -m- $(TOP)/$(B)/libtra/libc.a _envsetup >/dev/null
	cmp $(B)/ext_tra/_envsetup.o $(B)/orig/_envsetup.o && echo "OK: libtra lleva la transcripcion"
	cd $(B)/ext_fix && slb -x -m- $(TOP)/lib/libc.a _envsetup >/dev/null
	cmp $(B)/ext_fix/_envsetup.o $(B)/fix/_envsetup.o && echo "OK: lib/libc.a lleva el modulo corregido"
	test `slb -t - lib/libc.a | wc -l` -eq `slb -t - $(SYSLIB) | wc -l` && echo "OK: mismo numero de modulos"

.PHONY: all check
