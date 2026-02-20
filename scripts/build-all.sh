#!/bin/sh
# Build all language interfaces (per DEVELOPMENT.md)
set -eu

[ -f .randompack-root ] || {
  echo "build-all.sh: run this from the repository root (missing .randompack-root)" 1>&2
  exit 1
}

ROOT=$(cd "$(dirname "$0")/.." && pwd)

$ROOT/scripts/meson-setup.sh release
ninja -C $ROOT/release
ninja -C $ROOT/release install
$ROOT/scripts/syncpy.sh
$ROOT/scripts/syncR.sh
R CMD INSTALL $ROOT/r-package
