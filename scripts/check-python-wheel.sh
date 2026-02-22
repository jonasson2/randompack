#!/bin/sh
set -eu
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
REPO_ROOT=$(dirname "$SCRIPT_DIR")
cd "$REPO_ROOT"

# check-python-wheel.sh
# Build and test a clean Python wheel and sdist (local system only).
#
# Build a clean Python wheel and sdist for the randompack package.
#
# Assumptions:
#   - The Python package lives in ./python/
#   - meson-python is the build backend.
#
# What this script does:
#   1. Creates a temporary staging directory.
#   2. Rsyncs ./python into that directory (excluding build junk).
#   3. Initializes a temporary Git repo there (required for sdist).
#   4. Builds wheel + sdist using `python -m build`.
#   5. Installs the wheel into a fresh venv and runs tests.
#   6. Copies final artifacts into ./release-python/.
#
# The main repository is not modified.
tmp="${TMPDIR:-/tmp}/randompack-python-release"
release_dir="$PWD/release-python"

echo "Staging in $tmp"
rm -rf "$tmp"
mkdir -p "$tmp"

echo "Copying python package..."
rsync -av \
  --exclude '__pycache__' \
  --exclude '.mesonpy-*' \
  --exclude 'dist' \
  --exclude 'build' \
  --exclude '.pytest_cache' \
  --exclude 'docs/_build' \
  python/ "$tmp"/

cd "$tmp"

echo "Initializing temporary Git repo (required for sdist)..."
git init -q
git add -A
git commit -qm "release"

echo "Building wheel and sdist..."
python -m build

echo
echo "Running clean install and tests..."
python -m venv .venv
. .venv/bin/activate
python -m pip install -U pip >/dev/null
python -m pip install dist/*.whl >/dev/null
python -m pip install -U pytest >/dev/null
pytest -q
deactivate

mkdir -p "$release_dir"
cp dist/* "$release_dir/"

cd "$REPO_ROOT"
echo "Release complete."
echo "Artifacts now in: $release_dir"
ls -1 "$release_dir"
