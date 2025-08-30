#!/usr/bin/env bash

SDLDEV_VERS=2.30.10
SDLTTF_VERS=2.0.15
SDLMIXER_VERS=2.0.4
SODIUM_VERS=1.0.20

# exit when any command fails
set -euo pipefail

# detect architecture from script name
if echo "$(basename $0)" | grep -q 64; then
    MINGW_ARCH=x86_64-w64-mingw32
    SODIUM_ARCH=win64
else
    MINGW_ARCH=i686-w64-mingw32
    SODIUM_ARCH=win32
fi

# set MINGW_PREFIX
MINGW_PREFIX=/usr/${MINGW_ARCH}
if [ ! -d "${MINGW_PREFIX}" ]; then
    echo "MinGW prefix not found (${MINGW_PREFIX})"
    exit 1
else
    echo "Installing to ${MINGW_PREFIX}"
fi

# only use sudo when necessary
if [ `id -u` -ne 0 ]; then
    SUDO=sudo
else
    SUDO=""
fi

rm -rf "tmp-mingw-${MINGW_ARCH}-prep"
mkdir -p "tmp-mingw-${MINGW_ARCH}-prep"
cd "tmp-mingw-${MINGW_ARCH}-prep"

wget -q https://www.libsdl.org/release/SDL2-devel-${SDLDEV_VERS}-mingw.tar.gz -OSDL2-devel-${SDLDEV_VERS}-mingw.tar.gz
tar -xzf SDL2-devel-${SDLDEV_VERS}-mingw.tar.gz
#wget -q https://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-devel-${SDLTTF_VERS}-mingw.tar.gz -OSDL2_ttf-devel-${SDLTTF_VERS}-mingw.tar.gz
#tar -xzf SDL2_ttf-devel-${SDLTTF_VERS}-mingw.tar.gz
#wget -q https://www.libsdl.org/projects/SDL_mixer/release/SDL2_mixer-devel-${SDLMIXER_VERS}-mingw.tar.gz -OSDL2_mixer-devel-${SDLMIXER_VERS}-mingw.tar.gz
#tar -xzf SDL2_mixer-devel-${SDLMIXER_VERS}-mingw.tar.gz
sed -i '/$(CROSS_PATH)\/cmake/ s/^/#/' SDL2*/Makefile
$SUDO make -C SDL2*/ cross CROSS_PATH=/usr ARCHITECTURES=${MINGW_ARCH}

wget -q https://github.com/jedisct1/libsodium/releases/download/${SODIUM_VERS}-RELEASE/libsodium-${SODIUM_VERS}-mingw.tar.gz -Olibsodium-${SODIUM_VERS}-mingw.tar.gz
tar -xzf libsodium-${SODIUM_VERS}-mingw.tar.gz --no-same-owner
$SUDO cp -r libsodium-${SODIUM_ARCH}/* ${MINGW_PREFIX}

# Fixup pkgconfig prefix:
find "${MINGW_PREFIX}/lib/pkgconfig/" -name '*.pc' -exec \
  $SUDO sed -i "s|^prefix=.*|prefix=${MINGW_PREFIX}|" '{}' \;

# Fixup CMake prefix:
find "${MINGW_PREFIX}" -name '*.cmake' -exec \
  $SUDO sed -i "s|/opt/local/${MINGW_ARCH}|${MINGW_PREFIX}|" '{}' \;
