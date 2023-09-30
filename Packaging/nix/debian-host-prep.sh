#!/usr/bin/env bash
set -euo pipefail
set -x

sudo apt-get update
sudo apt-get install -y \
  rpm pkg-config cmake g++ git smpq gettext libsdl2-dev libsodium-dev \
  libpng-dev libbz2-dev libspeechd-dev
