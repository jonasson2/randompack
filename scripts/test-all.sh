#!/bin/sh
set -eu
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
REPO_ROOT=$(dirname "$SCRIPT_DIR")
cd "$REPO_ROOT"

# Run all normal tests (per DEVELOPMENT.md)

echo RUNNING C AND FORTRAN TESTS
meson test -C release
#scripts/run-test-variants.sh

echo RUNNING PYTHON TESTS
cd python
pytest -q
cd ..

echo RUNNING R TESTS
if ! build_out=$(R CMD build r-package 2>&1); then
  echo "$build_out" 1>&2
  exit 1
fi
tarball=$(printf '%s\n' "$build_out" | rg -o "randompack_[0-9][^ ]*\.tar\.gz" | tail -n 1)
[ -n "$tarball" ] || {
  echo "test-all.sh: could not find tarball name from R CMD build output" 1>&2
  exit 1
}
R CMD check $tarball

echo RUNNING JULIA TESTS
cd Randompack.jl
export JULIA_PROJECT=.
julia test/runtests.jl
