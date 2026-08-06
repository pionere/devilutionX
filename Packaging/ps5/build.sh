#!/usr/bin/env bash

set -e
SCRIPTDIR="${BASH_SOURCE[0]}"
SCRIPTDIR="$(dirname "${SCRIPTDIR}")"

declare -r CMAKE_ARGS="$@"
declare -r BUILD_DIR="build-ps5"

if [ -z "${PS5_PAYLOAD_SDK}" ]; then
    export PS5_PAYLOAD_SDK=/opt/ps5-payload-sdk
fi

source "${PS5_PAYLOAD_SDK}/toolchain/prospero.sh"

${CMAKE} -S "${SCRIPTDIR}/../../" -B"$BUILD_DIR" \
	 -DCMAKE_BUILD_TYPE=Release \
	 ${CMAKE_ARGS}
#${MAKE} -C "$BUILD_DIR" -j $(getconf _NPROCESSORS_ONLN)
${CMAKE} --build "$BUILD_DIR" -j $(getconf _NPROCESSORS_ONLN) --target package --config Release

#rm -rf "$BUILD_DIR"/DevilutionX
#mkdir "$BUILD_DIR"/DevilutionX

#cp -r "${SCRIPTDIR}/sce_sys" "$BUILD_DIR"/DevilutionX/
#cp "${SCRIPTDIR}/homebrew.js" "$BUILD_DIR"/DevilutionX/
#cp "${SCRIPTDIR}/README.txt" "$BUILD_DIR"/DevilutionX/
#cp "$BUILD_DIR"/devilx.mpq "$BUILD_DIR"/DevilutionX/
#cp "$BUILD_DIR"/../Packaging/resources/listfiles.txt "$BUILD_DIR"/DevilutionX/
#cp "$BUILD_DIR"/../Packaging/resources/mpqfiles.txt "$BUILD_DIR"/DevilutionX/
#cp "$BUILD_DIR"/devilutionx "$BUILD_DIR"/DevilutionX/devilutionx
#cp "$BUILD_DIR"/devil_patcher "$BUILD_DIR"/DevilutionX/devil_patcher | true

# Let github actions do this?
#cd "$BUILD_DIR"
#rm -f devilutionx.zip
#zip -r devilutionx.zip DevilutionX
