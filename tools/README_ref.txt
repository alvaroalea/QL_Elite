Tools for verification against the ORIGINAL code (for development only; they are not part of the game)
=====================================================================================================
The reference source (Mark Moxon, elite-source-code-apple-ii) is unlicensed and is NOT included in the project.
It is cloned separately and used only to check that the C ports do the same as the 6502:

    git clone https://github.com/markmoxon/elite-source-code-apple-ii ref/moxon-appleII   # in ref/ (it is in .gitignore)
    export ELITE_SRC=$PWD/ref/moxon-appleII/1-source-files/main-sources# (default value of the Makefile and the tools)

  tools/elitesrc.py   reads the source: table of variable symbols (SKIP/ORG) and utilities
  tools/cpu6502.py    6502 emulator (documented instructions, no decimal mode)
  tools/asm6502.py    assembler for the source's syntax (labels, EQUB/EQUW, P%, LO()/HI(), %bin, $hex)
  tools/orig.py       builds an image with the original LL9, SHPPT, LL51, PROJ, PLS6, DVID3B2, FMLTU and DORND;
                      LOIN/DOEXP/PLANET are "stubs" that log the call
  tools/gen_ll9.py    runs the original with random inputs and generates tests/vectors_ll9.txt
  tools/gen_shipdata.py  extracts the blueprints of the 32 ships from elite-data.asm -> src/shipdata.c/.h

Usage:  make test_orig     (LL9, CLIP/CLIP2, LL51, PROJ against the original)
The arithmetic tests (make test, make test_state) do NOT need the source: they use literal transcriptions in Python.
