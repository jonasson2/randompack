set -eu

# Ensure we are at repo root
[ -f .randompack-root ] || {
  echo "syncpy.sh: run this from the repository root (missing .randompack-root)" 1>&2
  exit 1
}

echo "Syncing C sources to python/src..."

rsync -av --delete \
  --exclude='.DS_Store' \
  --exclude='meson.build' \
  --exclude='printX.c' \
  src/ \
  python/src/

echo "Copying LICENSE..."
cp -f LICENSE python/LICENSE

echo "Sync complete."
