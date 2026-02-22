#!/bin/sh
set -eu
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
REPO_ROOT=$(dirname "$SCRIPT_DIR")
cd "$REPO_ROOT"

# Run test variants in separate build dirs.

echo RUNNING BUILTIN BLAS VARIANT
meson setup test-builtin-blas -Dblas=builtin -Dbuildtype=release
meson test -C test-builtin-blas

echo RUNNING NO128 VARIANT
meson setup test-no128 -Dforce_no128=true -Dbuildtype=release
meson test -C test-no128

echo RUNNING NOSIMD VARIANT
meson setup test-nosimd -Dforce_nosimd=true -Dbuildtype=release
meson test -C test-nosimd
