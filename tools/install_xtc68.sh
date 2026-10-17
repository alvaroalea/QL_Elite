#!/usr/bin/env bash
# Instala en el sandbox la cadena de compilacion de C para el Sinclair QL (qcc = XTC68 + runtime c68).
# El sandbox es efimero: hay que repetir esto en cada sesion nueva. Tarda un par de minutos y necesita red a github.com.
# Resultado: /tmp/qlsdk/bin/{qcc,c68,as68,qld,...} y /tmp/qlsdk/share/qdos/{include,lib}
#   export PATH=/tmp/qlsdk/bin:$PATH QLINC=/tmp/qlsdk/share/qdos/include QLLIB=/tmp/qlsdk/share/qdos/lib
#   make ql_game          # produce el ejecutable QDOS "elite"
# Requisitos: git, gcc, meson, ninja (pip install meson ninja si faltan).
set -e
PREFIX=${PREFIX:-/tmp/qlsdk}
W=${W:-/tmp/xtc68_src}
rm -rf "$W" /tmp/c68-support
git clone --depth 1 https://github.com/stronnag/xtc68 "$W"
git clone --depth 1 https://github.com/xXorAa/c68-support /tmp/c68-support
# gcc moderno rechaza las fuentes antiguas con -Werror (p. ej. ignorar el retorno de write): se quita solo en la copia local
sed -i "s/'-Werror', *//g" "$W"/src/*/meson.build
( cd "$W" && meson setup build --prefix="$PREFIX" --strip && ninja -C build install )
# el runtime de c68 (cabeceras INCLUDE_* y librerias LIB_*) va en support/424frun1, como pide el README de xtc68
mkdir -p "$W/support/424frun1"
cp -r /tmp/c68-support/. "$W/support/424frun1/"
( cd "$W/support/424frun1" && bash "$W/sdk-install.sh" "$PREFIX" )
echo "listo: export PATH=$PREFIX/bin:\$PATH QLINC=$PREFIX/share/qdos/include QLLIB=$PREFIX/share/qdos/lib"
