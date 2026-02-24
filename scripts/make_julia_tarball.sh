#!/bin/sh
set -eu
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
REPO_ROOT=$(dirname "$SCRIPT_DIR")
cd "$REPO_ROOT"
VER=${1:-0.1.1}
OUT=${2:-"$REPO_ROOT/build/archives/randompack-$VER.tar.gz"}
mkdir -p "$(dirname "$OUT")"
git archive --format=tar.gz --prefix=randompack-$VER/ -o "$OUT" \
  HEAD \
  meson.build meson_options.txt src LICENSE
echo "$OUT"
