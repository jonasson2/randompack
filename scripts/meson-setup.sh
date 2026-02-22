#!/bin/sh
set -eu
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
REPO_ROOT=$(dirname "$SCRIPT_DIR")
cd "$REPO_ROOT"

if [ $# -lt 1 ]; then
  echo "usage: scripts/meson-setup.sh <release|debug> [meson args...]" 1>&2
  exit 1
fi

buildtype="$1"
shift

case "$buildtype" in
  release|debug)
    ;;
  *)
    echo "usage: scripts/meson-setup.sh <release|debug> [meson args...]" 1>&2
    exit 1
    ;;
esac

meson setup release \
  --prefix "$PWD/install" \
  -Dbuildtype="$buildtype" \
  "$@"
