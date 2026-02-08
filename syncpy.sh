#!/bin/sh
# MAKE THE PYTHON PACKAGE MATCH THE C PACKAGE
set -eu
ROOT=$(cd "$(dirname "$0")" && pwd)

# SYNC SOURCE FILES
rsync -av --delete \
  --exclude='.DS_Store' \
  --exclude='meson.build' \
  --exclude='printX.c' \
  $ROOT/src/ \
  $ROOT/python/src/

# COPY LICENSE FILE
cp -f $ROOT/LICENSE $ROOT/python/LICENSE

# COPY VERSION NUMBER FROM MESON.BUILD TO SETUP.PY
ver=$(sed -n "s/.*version[[:space:]]*:[[:space:]]*'\([^']*\)'.*/\1/p" \
  $ROOT/meson.build | head -n 1)
[ -n "$ver" ] || { echo "syncpy.sh: version not found" 1>&2; exit 1; }
tmp=$ROOT/python/setup.py.tmp
sed "s/^\(\s*version\s*=\s*\).*/\1\"$ver\",/" $ROOT/python/setup.py > $tmp
mv $tmp $ROOT/python/setup.py
