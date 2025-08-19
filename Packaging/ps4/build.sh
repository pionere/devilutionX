#!/usr/bin/env bash

set -e

cd "$(dirname "${BASH_SOURCE[0]}")/../.."

declare -r CMAKE_ARGS="$1"
declare -r BUILD_DIR="build-ps4"

cmake -S. -B"$BUILD_DIR" \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_VERBOSE_MAKEFILE=ON \
      -DCMAKE_TOOLCHAIN_FILE="/opt/pacbrew/ps4/openorbis/cmake/ps4.cmake" \
      ${CMAKE_ARGS} \
      "$@"

cmake --build "$BUILD_DIR" -j $(getconf _NPROCESSORS_ONLN) --config Release
mv "$BUILD_DIR"/IV0001-DVLX00001_00-*.pkg "$BUILD_DIR"/devilutionx-ps4.pkg
