set -eu

# Ensure we are at repo root
[ -f .randompack-root ] || {
  echo "syncpy.sh: run this from the repository root (missing .randompack-root)" 1>&2
  exit 1
}

ROOT=$(cd "$(dirname "$0")/.." && pwd)

echo "Syncing C sources to python/src..."

rsync -av --delete \
  --exclude='.DS_Store' \
  --exclude='meson.build' \
  --exclude='printX.c' \
  $ROOT/src/ \
  $ROOT/python/src/

echo "Copying LICENSE..."
cp -f $ROOT/LICENSE $ROOT/python/LICENSE

echo "Sync complete."
