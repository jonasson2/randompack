#!/bin/sh
# Run all normal tests (per DEVELOPMENT.md)
set -eu

[ -f .randompack-root ] || {
  echo "test-all.sh: run this from the repository root (missing .randompack-root)" 1>&2
  exit 1
}

ROOT=$(cd "$(dirname "$0")/.." && pwd)

echo RUNNING C AND FORTRAN TESTS
meson test -C $ROOT/release

echo RUNNING PYTHON TESTS
cd $ROOT/python
pytest -q

echo RUNNING R TESTS
cd $ROOT
build_out=$(R CMD build r-package 2>&1)
tarball=$(printf '%s\n' "$build_out" | rg -o "randompack_[0-9][^ ]*\.tar\.gz" | tail -n 1)
[ -n "$tarball" ] || {
  echo "test-all.sh: could not find tarball name from R CMD build output" 1>&2
  exit 1
}
R CMD check $tarball

echo RUNNING JULIA TESTS
cd $ROOT/Randompack.jl
export JULIA_PROJECT=.
julia test/runtests.jl
