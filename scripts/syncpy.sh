#!/bin/sh
set -eu
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
REPO_ROOT=$(dirname "$SCRIPT_DIR")
cd "$REPO_ROOT"

echo "Syncing C sources to python/src..."

rsync -av --delete \
  --exclude='.DS_Store' \
  --exclude='meson.build' \
  --exclude='norm_polar.inc' \
  --exclude='printX.c' \
  src/ \
  python/src/

echo "Copying LICENSE..."
cp -f LICENSE python/LICENSE

echo "Sync complete."
