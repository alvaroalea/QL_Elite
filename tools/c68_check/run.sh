#!/bin/sh
# Compara el codigo que genera c68 (con -int=16 -O, como qcc) con el del PC, ejecutandolo bajo qemu.
# Para cada sonda de tests/c68/*.c: se compila con gcc (PC) y con c68 -> ensamblador -> GNU as -> qemu-m68k; ambas dejan
# hashes en la "pantalla" y se comparan byte a byte. Encontro un fallo real de c68 -O (ver la nota en src/ship3d.c).
# Requisitos: c68 compilado en el PC (git clone https://github.com/stronnag/xtc68; meson setup build; ninja -C build c68),
#             gcc-m68k-linux-gnu (as, ld) y qemu-user. Variables: XTC68=<dir con c68> (por defecto /tmp/xtc68/build)
set -e
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
XTC68=${XTC68:-$( [ -x /tmp/qlsdk/bin/c68 ] && echo /tmp/qlsdk/bin || echo /tmp/xtc68/build )}
W=${W:-/tmp/c68_work}
mkdir -p $W; cd $W
S=$ROOT/src
cp $ROOT/tools/c68_check/conv.py $ROOT/tools/c68_check/rt.s .
m68k-linux-gnu-as -m68020 --register-prefix-optional -o rt.o rt.s
buildc68() {          # salida fuentes...
  out=$1; shift; objs=""
  for f in "$@"; do
    b=$(basename $f .c)
    gcc -E -P -U__GNUC__ -nostdinc -I$ROOT/tools/c68_check/inc -I$S $f > $b.i 2>/dev/null
    $XTC68/c68 -int=16 -O -warn=0 $b.i $b.s 2>&1 | grep -iv "^!" | head -3 || true
    python3 conv.py $b.s ${b}_g.s
    m68k-linux-gnu-as -m68020 --register-prefix-optional -o $b.o ${b}_g.s
    objs="$objs $b.o"
  done
  m68k-linux-gnu-ld -o $out rt.o $objs
}
GFX="$S/gfx.c $S/text.c"
SHIP="$S/shipview.c $S/ship3d.c $S/shipdata.c $S/ship_math.c $S/state.c $S/emath.c $GFX"
DASHM="$S/dash.c $S/space.c $S/stars.c $S/ships.c $S/combat.c $S/damage.c $S/equip.c $S/dockit.c $S/tactics.c $S/move.c $S/sun.c $S/chart.c $S/ship3d.c $S/shipdata.c $S/ship_math.c $S/universe.c $S/game.c $S/cmdr.c $S/save.c $S/state.c $S/emath.c $S/sound.c $S/sound_host.c $GFX"
CHARTS="$S/chart.c $S/sun.c $S/ship3d.c $S/shipdata.c $S/ship_math.c $S/universe.c $S/game.c $S/cmdr.c $S/save.c $S/state.c $S/emath.c $GFX"
GAME="$S/universe.c $S/game.c $S/cmdr.c $S/save.c $S/state.c $S/emath.c $GFX"
FULL="$S/app.c $S/menu.c $S/chart.c $S/sun.c $S/dash.c $S/space.c $S/stars.c $S/ships.c $S/combat.c $S/damage.c $S/equip.c $S/dockit.c $S/tactics.c $S/move.c $S/flight.c $S/encounters.c $S/missions.c $S/detok.c $S/input.c $S/keys.c $S/keys_host.c $S/sound.c $S/sound_host.c $S/game.c $S/cmdr.c $S/save.c $S/shipview.c $S/ship3d.c $S/shipdata.c $S/ship_math.c $S/state.c $S/emath.c $S/gfx.c $S/text.c $S/universe.c"
fail=0
MISS="$S/detok.c $S/missions.c $S/encounters.c $S/menu.c $S/shipview.c $S/input.c $S/keys.c $S/keys_host.c $DASHM"
for p in gfxfast:"$GFX" emath:"$S/emath.c $S/state.c" keys:"$S/keys.c $S/keys_host.c $GFX" ships:"$SHIP" game:"$GAME" charts:"$CHARTS" dash:"$DASHM" flight:"$DASHM" session:"$FULL" missions:"$MISS"; do
  name=${p%%:*}; mods=${p#*:}; src=$ROOT/tests/c68/probe_$name.c
  if [ $# -gt 0 ]; then case " $* " in *" $name "*) ;; *) continue ;; esac; fi        # make c68_check: solo las sondas indicadas
  gcc -std=gnu89 -O1 -I$S -o host_$name $src $mods $S/plat_host.c -lm 2>/dev/null
  rm -f screen.bin; ./host_$name; cp screen.bin host_$name.bin
  buildc68 c68_$name $src $mods
  rm -f screen.bin; timeout 600 qemu-m68k -cpu m68020 ./c68_$name; cp screen.bin c68_$name.bin
  if cmp -s host_$name.bin c68_$name.bin; then echo "sonda $name: IGUAL (PC = c68)"; else echo "sonda $name: DIFERENTE"; fail=1; fi
done
exit $fail
