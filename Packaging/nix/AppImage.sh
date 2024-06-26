#!/usr/bin/env bash
set -euo pipefail
set -x

BUILD_DIR="${1-build}"
cmake --install "$BUILD_DIR" --prefix "${BUILD_DIR}/AppDir/usr"
mv "$BUILD_DIR"/AppDir/usr/share/diasurgical/devilutionx/devilx.mpq "$BUILD_DIR"/AppDir/usr/bin/devilx.mpq

APPIMAGE_BUILDER="${APPIMAGE_BUILDER:-linuxdeploy-x86_64.AppImage}"
if ! which "$APPIMAGE_BUILDER"; then
	if ! [[ -f linuxdeploy-x86_64.AppImage ]]; then
		wget -q https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage -N
		chmod +x linuxdeploy-x86_64.AppImage
	fi
	APPIMAGE_BUILDER=../linuxdeploy-x86_64.AppImage
fi

SRC_DIR="${PWD}"
cd "$BUILD_DIR"
LD_LIBRARY_PATH="${PWD}/AppDir/usr/lib" "$APPIMAGE_BUILDER" --appimage-extract-and-run \
	--appdir=AppDir \
	--custom-apprun="${SRC_DIR}/Packaging/nix/AppRun" \
	-d "devilutionx.desktop" \
	-o appimage
cd -

mv "${BUILD_DIR}/"DevilutionX*.AppImage devilutionx.appimage
