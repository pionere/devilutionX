#!/usr/bin/env bash
set -euo pipefail
set -x

PACKAGES=(
  cmake git smpq gettext
  libsdl2-dev:arm64 libsodium-dev:arm64
  libpng-dev:arm64 libbz2-dev:arm64 libspeechd-dev:arm64
)

if (( $# < 1 )) || [[ "$1" != --no-gcc ]]; then
  PACKAGES+=(g++-multilib)
fi

sudo dpkg --add-architecture i386
sudo apt-get update
sudo apt-get install --ignore-hold -y "${PACKAGES[@]}"

