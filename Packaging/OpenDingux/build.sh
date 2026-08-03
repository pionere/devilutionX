#!/usr/bin/env bash

set -euo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")/../.."

source Packaging/OpenDingux/targets.sh
source Packaging/OpenDingux/package-opk.sh

usage() {
  echo "Usage: build.sh [target] [cmakeargs]"
  usage_target
}

if ! check_target "$1"; then
  usage
  exit 64
fi

declare -r TARGET="$1"
declare -r CMAKE_ARGS="$2"
declare -r BUILD_DIR="build-${TARGET}"
declare -rA BUILDROOT_REPOS=(
	[lepus]=https://github.com/OpenDingux/buildroot.git
	[retrofw]=https://github.com/retrofw/buildroot.git
	[rg350]=https://github.com/OpenDingux/buildroot.git
	[gkd350h]=https://github.com/tonyjih/RG350_buildroot.git
)
declare -rA BUILDROOT_DEFCONFIGS=(
	[lepus]='od_lepus_defconfig BR2_EXTERNAL=board/opendingux'
	[retrofw]='RetroFW_defconfig BR2_EXTERNAL=retrofw'
	[rg350]='od_gcw0_defconfig BR2_EXTERNAL=board/opendingux'
	[gkd350h]='rg350_defconfig BR2_EXTERNAL=board/opendingux'
)

declare BUILDROOT_TARGET="$TARGET"

# If a TOOLCHAIN environment variable is set, just use that.
if [[ -z ${TOOLCHAIN:-} ]]; then
	BUILDROOT="${BUILDROOT:-$HOME/devilutionx-buildroots/$BUILDROOT_TARGET}"
	TOOLCHAIN="${BUILDROOT}/output/host"
fi

main() {
	>&2 echo "Building for target ${TARGET} in ${BUILD_DIR}"
	set -x
	if [[ -n ${BUILDROOT:-} ]]; then
		prepare_buildroot
		make_buildroot
	fi
	build
	package_opk
}

prepare_buildroot() {
	if [[ -d $BUILDROOT ]]; then
		return
	fi
	git clone --depth=1 "${BUILDROOT_REPOS[$BUILDROOT_TARGET]}" "$BUILDROOT"
	cd "$BUILDROOT"
	mkdir -p ../shared-dl
	ln -s ../shared-dl dl

	# Work around a BR2_EXTERNAL initialization bug in older buildroots.
	mkdir -p output
	touch output/.br-external.mk
	local -a config_args=(${BUILDROOT_DEFCONFIGS[$BUILDROOT_TARGET]})
	local -r config="${config_args[0]}"

	# If the buildroot uses per-package directories, disable them.
	# Otherwise, we'd have to buildroot the entire buildroot (up to `host-finalize`) to get
	# the merged host directory.
	if grep -q BR2_PER_PACKAGE_DIRECTORIES=y "configs/${config}"; then
		local -r new_config="${config%_defconfig}_no_ppd_defconfig"
		sed 's/BR2_PER_PACKAGE_DIRECTORIES=y/# BR2_PER_PACKAGE_DIRECTORIES is not selected/' \
		  "configs/${config}" > "configs/${new_config}"
		config_args[0]="$new_config"
	fi

	make "${config_args[@]}"
	cd -
}

make_buildroot() {
	cd "$BUILDROOT"
	local -a env_args=(
	  # Unset client variables that cause issues with buildroot
	  -u PERL_MM_OPT
	  -u CMAKE_GENERATOR -u CMAKE_GENERATOR_PLATFORM -u CMAKE_GENERATOR_TOOLSET -u CMAKE_GENERATOR_INSTANCE

	  # Enable parallelism
	  BR2_JLEVEL=0
	)
	env "${env_args[@]}" make toolchain sdl
	cd -
}

cmake_configure() {
	cmake -S. -B"$BUILD_DIR" \
		"-DTARGET_PLATFORM=$TARGET" \
		-DCMAKE_VERBOSE_MAKEFILE=ON \
		-DCMAKE_TOOLCHAIN_FILE="${TOOLCHAIN}/usr/share/buildroot/toolchainfile.cmake" \
		${CMAKE_ARGS} \
		"$@"
}

cmake_build() {
	cmake --build "$BUILD_DIR" -j "$(getconf _NPROCESSORS_ONLN)" --config Release
}

strip_bin() {
	"${TOOLCHAIN}/usr/bin/"*-linux-strip -s -R .comment -R .gnu.version "${BUILD_DIR}/devilutionx"
}

build_debug() {
	cmake_configure -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS_DEBUG="-g -fno-omit-frame-pointer"
	cmake_build
}

build_relwithdebinfo() {
	cmake_configure -DCMAKE_BUILD_TYPE=RelWithDebInfo
	cmake_build
}

build_release() {
	cmake_configure -DCMAKE_BUILD_TYPE=Release
	cmake_build
	strip_bin
}

build() {
	rm -f "${BUILD_DIR}/CMakeCache.txt"
	build_release
}

main
