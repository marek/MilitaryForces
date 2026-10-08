#!/bin/bash
# build-deps.sh <x86_64|arm64>
# Builds SDL2 and FreeType from source as dynamic libraries for one macOS architecture, into deps/<arch>. The game
# build copies them next to its executable, so it needs nothing from Homebrew at run time. Sources are downloaded to a temporary folder
# and removed afterwards.
set -e
ARCH=${1:?usage: build-deps.sh <x86_64|arm64>}
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
OUT=$ROOT/deps/$ARCH
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT
SDL=2.32.8
FT=2.13.3
COMMON="-DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=$ARCH -DCMAKE_OSX_DEPLOYMENT_TARGET=13.3 -DCMAKE_INSTALL_PREFIX=$OUT -DBUILD_SHARED_LIBS=ON"
cd "$WORK"
curl -fsSL -o sdl.tar.gz https://github.com/libsdl-org/SDL/releases/download/release-$SDL/SDL2-$SDL.tar.gz
curl -fsSL -o ft.tar.gz https://download.savannah.gnu.org/releases/freetype/freetype-$FT.tar.gz
tar xzf sdl.tar.gz; tar xzf ft.tar.gz
cmake -S SDL2-$SDL -B sdl-build $COMMON -DSDL_SHARED=ON -DSDL_STATIC=OFF -DSDL_TEST=OFF > sdl.log 2>&1
cmake --build sdl-build -j8 >> sdl.log 2>&1
cmake --install sdl-build >> sdl.log 2>&1
# FreeType on its own: no PNG, bzip2, HarfBuzz or Brotli, and its own copy of zlib
cmake -S freetype-$FT -B ft-build $COMMON -DFT_DISABLE_PNG=ON -DFT_DISABLE_BZIP2=ON -DFT_DISABLE_HARFBUZZ=ON -DFT_DISABLE_BROTLI=ON -DFT_DISABLE_ZLIB=ON > ft.log 2>&1
cmake --build ft-build -j8 >> ft.log 2>&1
cmake --install ft-build >> ft.log 2>&1
lipo -info "$OUT"/lib/libSDL2-2.0.0.dylib "$OUT"/lib/libfreetype.6.dylib
