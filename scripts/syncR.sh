#!/bin/sh
set -eu
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
REPO_ROOT=$(dirname "$SCRIPT_DIR")
cd "$REPO_ROOT"

# MAKE THE R PACKAGE MATCH THE C PACKAGE

# Remove artifacts that are intentionally excluded from the rsync.
rm -f \
  r-package/src/randompack_float.c \
  r-package/src/*_float.inc \
  r-package/src/*_float.h

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
  --exclude='norm_exp_mod.inc' \
  --exclude='norm_polar.inc' \
  --exclude='printX.c' \
  --exclude='randompack_float.c' \
  --exclude='*_float.inc' \
  --exclude='*_float.h' \
  src/ \
  r-package/src/

# COPY LICENSE FILE
cp -f LICENSE r-package/inst/THIRD-PARTY-NOTICES
