#!/usr/bin/env bash
set -euo pipefail
set -x

FLAVOR="$(lsb_release -sc)"

if dpkg-vendor --derives-from Ubuntu; then
	sudo tee /etc/apt/sources.list.d/arm64.list <<LIST
deb [arch=arm64] http://ports.ubuntu.com/ ${FLAVOR} main restricted
deb [arch=arm64] http://ports.ubuntu.com/ ${FLAVOR}-updates main restricted
deb [arch=arm64] http://ports.ubuntu.com/ ${FLAVOR} universe
deb [arch=arm64] http://ports.ubuntu.com/ ${FLAVOR}-updates universe
deb [arch=arm64] http://ports.ubuntu.com/ ${FLAVOR} multiverse
deb [arch=arm64] http://ports.ubuntu.com/ ${FLAVOR}-updates multiverse
deb [arch=arm64] http://ports.ubuntu.com/ ${FLAVOR}-backports main restricted universe multiverse
LIST
	sudo sed -i 's/deb http/deb [arch=amd64,i386] http/' /etc/apt/sources.list
fi

sudo dpkg --add-architecture arm64
sudo apt-get update
sudo apt-get install -y cmake git smpq gettext crossbuild-essential-arm64 \
  libsdl2-dev:arm64 libsdl2-image-dev:arm64 libsodium-dev:arm64 \
  libsimpleini-dev:arm64 libpng-dev:arm64 libbz2-dev:arm64 libfmt-dev:arm64
