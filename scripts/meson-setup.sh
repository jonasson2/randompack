#!/bin/sh
set -eu
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
REPO_ROOT=$(dirname "$SCRIPT_DIR")
cd "$REPO_ROOT"

if [ $# -lt 1 ]; then
  echo "usage: scripts/meson-setup.sh <builddir> [--buildtype=release|debug] [meson args...]" 1>&2
  exit 1
fi

builddir="$1"
shift

if [ -e "$builddir" ]; then
  rm -rf "$builddir"
fi

buildtype="release"
while [ $# -gt 0 ]; do
  case "$1" in
    --buildtype=*)
      buildtype="${1#--buildtype=}"
      shift
      ;;
    --buildtype)
      shift
      if [ $# -lt 1 ]; then
        echo "usage: scripts/meson-setup.sh <builddir> [--buildtype=release|debug] [meson args...]" 1>&2
        exit 1
      fi
      buildtype="$1"
      shift
      ;;
    *)
      break
      ;;
  esac
done

case "$buildtype" in
  release|debug)
    ;;
  *)
    echo "usage: scripts/meson-setup.sh <builddir> [--buildtype=release|debug] [meson args...]" 1>&2
    exit 1
    ;;
esac

meson setup "$builddir" \
  --prefix "$PWD/install" \
  -Dlibdir=lib \
  -Dbuildtype="$buildtype" \
  "$@"
