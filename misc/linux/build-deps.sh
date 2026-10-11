#!/bin/bash
# build-deps.sh <x86_64|arm64>
# Builds SDL2, FreeType and OpenAL Soft from source as shared libraries into deps/<arch>. The game build copies them
# next to its executable, so the game needs nothing but the C library and the system's OpenGL. SDL2 and OpenAL Soft
# open their video and audio backends (X11, Wayland, PulseAudio, PipeWire, ALSA) at run time, so the headers for
# those are needed here but the libraries are not needed by the players. Sources are downloaded to a temporary
# folder and removed afterwards.
set -e
ARCH=${1:?usage: build-deps.sh <x86_64|arm64>}
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
OUT=$ROOT/deps/$ARCH
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT
SDL=2.32.8
FT=2.13.3
AL=1.24.3
JOBS=$(nproc)
# the C++ runtime goes into OpenAL Soft itself, so it does not ask for a newer libstdc++ than the player has
COMMON=(-DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$OUT" -DCMAKE_INSTALL_LIBDIR=lib -DBUILD_SHARED_LIBS=ON
	"-DCMAKE_SHARED_LINKER_FLAGS=-static-libstdc++ -static-libgcc")
cd "$WORK"
curl -fsSL -o sdl.tar.gz https://github.com/libsdl-org/SDL/releases/download/release-$SDL/SDL2-$SDL.tar.gz
curl -fsSL -o ft.tar.gz https://download.savannah.gnu.org/releases/freetype/freetype-$FT.tar.gz
curl -fsSL -o al.tar.gz https://github.com/kcat/openal-soft/archive/refs/tags/$AL.tar.gz
tar xzf sdl.tar.gz; tar xzf ft.tar.gz; tar xzf al.tar.gz

cmake -S SDL2-$SDL -B sdl-build "${COMMON[@]}" -DSDL_SHARED=ON -DSDL_STATIC=OFF -DSDL_TEST=OFF > sdl.log 2>&1 || { tail -30 sdl.log; exit 1; }
cmake --build sdl-build -j$JOBS >> sdl.log 2>&1 || { tail -30 sdl.log; exit 1; }
cmake --install sdl-build >> sdl.log 2>&1

# FreeType on its own: no PNG, bzip2, HarfBuzz or Brotli, and its own copy of zlib
cmake -S freetype-$FT -B ft-build "${COMMON[@]}" -DFT_DISABLE_PNG=ON -DFT_DISABLE_BZIP2=ON -DFT_DISABLE_HARFBUZZ=ON \
	-DFT_DISABLE_BROTLI=ON -DFT_DISABLE_ZLIB=ON > ft.log 2>&1 || { tail -30 ft.log; exit 1; }
cmake --build ft-build -j$JOBS >> ft.log 2>&1 || { tail -30 ft.log; exit 1; }
cmake --install ft-build >> ft.log 2>&1

# OpenAL Soft: no utilities, examples or configuration files; HRTF data built in. Its other backends are opened at
# run time, but sndio (a BSD sound system) would be linked in, so it is left out
cmake -S openal-soft-$AL -B al-build "${COMMON[@]}" -DLIBTYPE=SHARED -DALSOFT_UTILS=OFF -DALSOFT_EXAMPLES=OFF -DALSOFT_TESTS=OFF \
	-DALSOFT_BACKEND_SNDIO=OFF \
	-DALSOFT_INSTALL_CONFIG=OFF -DALSOFT_INSTALL_HRTF_DATA=OFF -DALSOFT_INSTALL_AMBDEC_PRESETS=OFF \
	-DALSOFT_INSTALL_EXAMPLES=OFF -DALSOFT_INSTALL_UTILS=OFF -DALSOFT_EMBED_HRTF_DATA=ON > al.log 2>&1 || { tail -30 al.log; exit 1; }
cmake --build al-build -j$JOBS >> al.log 2>&1 || { tail -30 al.log; exit 1; }
cmake --install al-build >> al.log 2>&1

for LIB in libSDL2-2.0.so.0 libfreetype.so.6 libopenal.so.1; do
	echo "$LIB:"; readelf -d "$OUT/lib/$LIB" | grep NEEDED
done
