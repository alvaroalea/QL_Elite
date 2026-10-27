# Experiment: c68 4.24f libc.a with the _envsetup module fixed (hang on Minerva without Toolkit II).
#   make          -> fixed lib/libc.a (copy of orig/libc.a with _envsetup replaced)
#   make check    -> 1) the faithful transcription (envsetup_orig.asm) gives an executable identical to the original libc's
#                    2) in lib/libc.a the _envsetup module is the fixed one and the number of modules does not change
# Usage: qcc ... -L<path>/lib/ (the folder goes before the system one). An _envsetup defined in the program
# itself (like the one in src/plat_ql.c) takes priority over that of any library.
# Notes: as68 sets the SROFF module name to the output file name as is (with directory and
# extension), and slb -r only replaces a module with the same name (otherwise it adds it or does nothing). That is why
# it is assembled inside the destination directory to a file called "_envsetup" and then copied to .o.

# starting libc.a: the copy saved in orig/ (the untouched /usr/local/share/qdos/lib/libc.a), so that the
# result does not depend on what is installed. Another: make SYSLIB=/path/libc.a
SYSLIB ?= orig/libc.a
QLINC  ?= /usr/local/share/qdos/include
B      := build
TOP    := $(CURDIR)

all: lib/libc.a

# fixed module
$(B)/fix/_envsetup.o: envsetup.asm
	mkdir -p $(B)/fix
	cd $(B)/fix && as68 $(TOP)/envsetup.asm _envsetup && cp _envsetup _envsetup.o

# faithfully transcribed module (only for the check)
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
