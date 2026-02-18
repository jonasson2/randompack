#!/bin/sh
set -eu

# Ensure we are at repo root
[ -f .randompack-root ] || {
  echo "meson-setup.sh: run this from the repository root (missing .randompack-root)" 1>&2
  exit 1
}

if [ $# -lt 1 ]; then
  echo "usage: scripts/meson-setup.sh <release|debug> [meson args...]" 1>&2
  exit 1
fi

buildtype="$1"
shift

ROOT=$(pwd)

case "$buildtype" in
  release|debug)
    ;;
  *)
    echo "usage: scripts/meson-setup.sh <release|debug> [meson args...]" 1>&2
    exit 1
    ;;
esac

meson setup release \
  --prefix "$ROOT/install" \
  -Dbuildtype="$buildtype" \
  "$@"
