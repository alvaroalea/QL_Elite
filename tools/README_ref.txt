Herramientas de verificacion contra el codigo ORIGINAL (solo para desarrollo; no forman parte del juego)
=====================================================================================================
La fuente de referencia (Mark Moxon, elite-source-code-apple-ii) no tiene licencia y NO se incluye en el proyecto.
Se clona aparte y se usa unicamente para comprobar que los puertos en C hacen lo mismo que el 6502:

    git clone https://github.com/markmoxon/elite-source-code-apple-ii ref/moxon-appleII   # en ref/ (esta en .gitignore)
    export ELITE_SRC=$PWD/ref/moxon-appleII/1-source-files/main-sources# (valor por defecto del Makefile y de las herramientas)

  tools/elitesrc.py   lee la fuente: tabla de simbolos de variables (SKIP/ORG) y utilidades
  tools/cpu6502.py    emulador de 6502 (instrucciones documentadas, sin modo decimal)
  tools/asm6502.py    ensamblador de la sintaxis de la fuente (etiquetas, EQUB/EQUW, P%, LO()/HI(), %bin, $hex)
  tools/orig.py       monta una imagen con LL9, SHPPT, LL51, PROJ, PLS6, DVID3B2, FMLTU y DORND originales;
                      LOIN/DOEXP/PLANET son "stubs" que registran la llamada
  tools/gen_ll9.py    ejecuta el original con entradas aleatorias y genera tests/vectors_ll9.txt
  tools/gen_shipdata.py  extrae los planos de las 32 naves de elite-data.asm -> src/shipdata.c/.h

Uso:  make test_orig     (LL9, CLIP/CLIP2, LL51, PROJ contra el original)
Las pruebas de aritmetica (make test, make test_state) NO necesitan la fuente: usan transcripciones literales en Python.
