#!/usr/bin/env bash

declare -r CMAKE_ARGS="$1"

set -e
SCRIPTDIR="${BASH_SOURCE[0]}"
SCRIPTDIR="$(dirname "${SCRIPTDIR}")"

echo "Scriptdir: ${SCRIPTDIR}, Args: ${CMAKE_ARGS}"

cmake -S "${SCRIPTDIR}/../../" \
      -B build-ps4 \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_VERBOSE_MAKEFILE=ON \
      -DCMAKE_TOOLCHAIN_FILE="/opt/pacbrew/ps4/openorbis/cmake/ps4.cmake" \
      "${CMAKE_ARGS}"

cmake --build build-ps4 -j $(getconf _NPROCESSORS_ONLN)
mv build-ps4/IV0001-DVLX00001_00-*.pkg build-ps4/devilutionx.pkg
