#!/bin/sh
# Create an archive for Julia and push it to randompack_src on github
set -eu
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
REPO_ROOT=$(dirname "$SCRIPT_DIR")
cd "$REPO_ROOT"

if [ -n "$(git status --porcelain)" ]; then
  echo "ERROR: git commit before creating tarball\n"
  echo "Uncommitted changes in:"
  git status --porcelain
  exit 1
fi

VER=$(<VERSION)
FILENAME=randompack-$VER.tar.gz
OUT=archives/$FILENAME
URL=https://raw.githubusercontent.com/jonasson2/randompack-src/main/$FILENAME
mkdir -p archives
git archive --format=tar.gz --prefix="randompack-$VER/" -o "$OUT" \
    HEAD meson.build meson_options.txt src LICENSE
SHA=$(shasum -a 256 "$OUT" | awk '{print $1}')
printf "%s\n" SHA:$SHA
cd archives
git commit -am "Update $FILENAME"
git push
echo "Created $OUT and pushed it to github."
