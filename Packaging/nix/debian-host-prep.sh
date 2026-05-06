#!/usr/bin/env bash
set -euo pipefail
set -x

PACKAGES=(
  rpm pkg-config cmake git tar
  libsdl2-dev libsodium-dev
  libspeechd-dev
)

if (( $# < 1 )) || [[ "$1" != --no-gcc ]]; then
  PACKAGES+=(g++)
fi

sudo apt-get update
sudo apt-get install -y "${PACKAGES[@]}"

