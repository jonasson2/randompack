#!/bin/sh
# Run test variants in separate build dirs.
set -eu

[ -f .randompack-root ] || {
  echo "run-test-variants.sh: run this from the repository root (missing .randompack-root)" 1>&2
  exit 1
}

ROOT=$(cd "$(dirname "$0")/.." && pwd)

echo RUNNING BUILTIN BLAS VARIANT
meson setup "$ROOT/test-builtin-blas" -Dblas=builtin -Dbuildtype=release
meson test -C "$ROOT/test-builtin-blas"

echo RUNNING NO128 VARIANT
meson setup "$ROOT/test-no128" -Dforce_no128=true -Dbuildtype=release
meson test -C "$ROOT/test-no128"

echo RUNNING NOSIMD VARIANT
meson setup "$ROOT/test-nosimd" -Dforce_nosimd=true -Dbuildtype=release
meson test -C "$ROOT/test-nosimd"
