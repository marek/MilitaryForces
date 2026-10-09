#!/bin/bash
# build-deps.sh <x86_64|arm64>
# Builds SDL2, FreeType and OpenAL Soft from source as dynamic libraries for one macOS architecture, into deps/<arch>. The game
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
AL=1.24.3
# ONLY=openal builds just that one (the others are left as they are in deps/<arch>)
ONLY=${ONLY:-all}
COMMON="-DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=$ARCH -DCMAKE_OSX_DEPLOYMENT_TARGET=13.3 -DCMAKE_INSTALL_PREFIX=$OUT -DBUILD_SHARED_LIBS=ON"
cd "$WORK"
if [ "$ONLY" = all ]; then
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
fi
# OpenAL Soft: the game opens it by name at run time, so it is not found by following the executable's links; the
# game build copies it beside the executable itself. No utilities, examples or configuration files
curl -fsSL -o al.tar.gz https://github.com/kcat/openal-soft/archive/refs/tags/$AL.tar.gz
tar xzf al.tar.gz
cmake -S openal-soft-$AL -B al-build $COMMON -DLIBTYPE=SHARED -DALSOFT_UTILS=OFF -DALSOFT_EXAMPLES=OFF -DALSOFT_TESTS=OFF -DALSOFT_INSTALL_CONFIG=OFF -DALSOFT_INSTALL_HRTF_DATA=OFF -DALSOFT_INSTALL_AMBDEC_PRESETS=OFF -DALSOFT_INSTALL_EXAMPLES=OFF -DALSOFT_INSTALL_UTILS=OFF -DALSOFT_EMBED_HRTF_DATA=ON > al.log 2>&1
cmake --build al-build -j8 >> al.log 2>&1
cmake --install al-build >> al.log 2>&1
lipo -info "$OUT"/lib/libSDL2-2.0.0.dylib "$OUT"/lib/libfreetype.6.dylib "$OUT"/lib/libopenal.1.dylib
