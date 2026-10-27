#!/usr/bin/env bash
# Installs in the sandbox the C build chain for the Sinclair QL (qcc = XTC68 + c68 runtime).
# The sandbox is ephemeral: this has to be repeated in every new session. It takes a couple of minutes and needs network access to github.com.
# Result: /tmp/qlsdk/bin/{qcc,c68,as68,qld,...} and /tmp/qlsdk/share/qdos/{include,lib}
#   export PATH=/tmp/qlsdk/bin:$PATH QLINC=/tmp/qlsdk/share/qdos/include QLLIB=/tmp/qlsdk/share/qdos/lib
#   make elite            # produces the QDOS executable "build/elite"
# Requirements: git, gcc, meson, ninja (pip install meson ninja if missing).
# The sources are cloned into the project's tmp/ (ignored by git); the installation goes to $PREFIX.
set -e
ROOT=$(cd "$(dirname "$0")/.." && pwd)
PREFIX=${PREFIX:-/tmp/qlsdk}
W=${W:-$ROOT/tmp/xtc68_src}
SUP=${SUP:-$ROOT/tmp/c68-support}
rm -rf "$W" "$SUP"
git clone --depth 1 https://github.com/stronnag/xtc68 "$W"
git clone --depth 1 https://github.com/xXorAa/c68-support "$SUP"
# modern gcc rejects the old sources with -Werror (e.g. ignoring write's return value): it is removed only in the local copy
sed -i "s/'-Werror', *//g" "$W"/src/*/meson.build
( cd "$W" && meson setup build --prefix="$PREFIX" --strip && ninja -C build install )
# the c68 runtime (INCLUDE_* headers and LIB_* libraries) goes in support/424frun1, as the xtc68 README asks
mkdir -p "$W/support/424frun1"
cp -r "$SUP"/. "$W/support/424frun1/"
( cd "$W/support/424frun1" && bash "$W/sdk-install.sh" "$PREFIX" )
echo "listo: export PATH=$PREFIX/bin:\$PATH QLINC=$PREFIX/share/qdos/include QLLIB=$PREFIX/share/qdos/lib"
