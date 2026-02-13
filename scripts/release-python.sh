#!/bin/sh
#
# release-python.sh
#
# Build a clean Python wheel and sdist for the randompack package.
#
# Assumptions:
#   - Script is run from the repository root.
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
#

set -e

[ -f .randompack-root ] || {
  echo "release-python.sh: run this from the repository root (missing .randompack-root)" 1>&2
  exit 1
}

repo_root="$(pwd)"
tmp="${TMPDIR:-/tmp}/randompack-python-release"
release_dir="$repo_root/release-python"

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

cd "$repo_root"
echo "Release complete."
echo "Artifacts now in: $release_dir"
ls -1 "$release_dir"
