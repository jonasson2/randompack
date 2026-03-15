#!/bin/sh
set -eu
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
REPO_ROOT=$(dirname "$SCRIPT_DIR")
cd "$REPO_ROOT"

# MAKE THE R PACKAGE MATCH THE C PACKAGE

# SYNC SOURCE FILES
rsync -av --delete \
  --include='rp_dpstrf.c' \
  --exclude='*_R.c' \
  --exclude='init.c' \
  --exclude='Makevars' \
  --exclude='Makevars.in' \
  --exclude='.DS_Store' \
  --exclude='meson.build' \
  --exclude='blas.f' \
  --exclude='printX.c' \
  --exclude='*_float.inc' \
  --exclude='*_float.h' \
  src/ \
  r-package/src/

# COPY LICENSE FILE
cp -f LICENSE r-package/inst/THIRD-PARTY-NOTICES

# COMMENT OUT ALL FLOAT INCLUDES
# Find all source files and comment out any #include with *_float.*
find r-package/src -type f \( -name "*.c" -o -name "*.h" -o -name "*.inc" \) \
  -exec sed -i.bak 's|^\(#include.*_float\.\)|// \1|' {} \;

# Remove backup files
rm -f r-package/src/*.bak
