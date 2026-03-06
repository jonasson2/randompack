#!/bin/sh
# Create an archive for Julia
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

VER=$(cat VERSION)
FILENAME=randompack-$VER.tar.gz
echo $FILENAME
OUT=archives/$FILENAME
URL=https://raw.githubusercontent.com/jonasson2/randompack-src/main/$FILENAME
mkdir -p archives
git archive --format=tar.gz --prefix="randompack-$VER/" -o "$OUT" \
    HEAD meson.build meson_options.txt src LICENSE
printf "%s\n" "Created $OUT"
SHA=$(shasum -a 256 "$OUT" | awk '{print $1}')
printf "%s\n%s\n" "SHA (update in packaging/build_tarballs.jl):" "$SHA"
cd archives
# git commit -am "Update $FILENAME"
# git push
# echo "Created $OUT and pushed it to github."
