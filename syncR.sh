#!/bin/sh
# MAKE THE R PACKAGE MATCH THE C PACKAGE
set -eu
ROOT=$(cd "$(dirname "$0")" && pwd)

# SYNC SOURCE FILES
rsync -av --delete \
  --exclude='*_R.c' \
  --exclude='init.c' \
  --exclude='Makevars' \
  --exclude='.DS_Store' \
  --exclude='meson.build' \
  --exclude='printX.c' \
  --exclude='printX.h' \
  --exclude='*_float.inc' \
  --exclude='*_float.h' \
  $ROOT/src/ \
  $ROOT/r-package/src/

# COPY LICENSE FILE
cp -f $ROOT/LICENSE $ROOT/r-package/

# COMMENT OUT ALL FLOAT INCLUDES
# Find all source files and comment out any #include with *_float.*
find $ROOT/r-package/src -type f \( -name "*.c" -o -name "*.h" -o -name "*.inc" \) \
  -exec sed -i.bak 's|^\(#include.*_float\.\)|// \1|' {} \;

# Remove backup files
rm -f $ROOT/r-package/src/*.bak

# COPY VERSION NUMBER FROM MESON.BUILD TO DESCRIPTION
ver=$(sed -n "s/.*version[[:space:]]*:[[:space:]]*'\([^']*\)'.*/\1/p" \
  $ROOT/meson.build | head -n 1)
[ -n "$ver" ] || { echo "syncR.sh: version not found" 1>&2; exit 1; }
tmp=$ROOT/r-package/DESCRIPTION.tmp
sed "s/^Version:.*/Version: $ver/" $ROOT/r-package/DESCRIPTION > $tmp
mv $tmp $ROOT/r-package/DESCRIPTION
