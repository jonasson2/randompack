#!/bin/sh
set -eu
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
REPO_ROOT=$(dirname "$SCRIPT_DIR")
cd "$REPO_ROOT"

if [ -z "$1" ]; then
  echo "Usage: $0 NEW_VERSION"
  exit 1
fi

VER="$1"

# Update VERSION file
echo "$VER" > VERSION

# Update C header
sed -i.bak -E \
  "s/^#define[[:space:]]+RANDOMPACK_VERSION[[:space:]]+\".*\"/#define RANDOMPACK_VERSION \"$VER\"/" \
  src/randompack.h
rm -f src/randompack.h.bak

# Update Python pyproject.toml
sed -i.bak -E \
  "s/^version[[:space:]]*=[[:space:]]*\".*\"/version = \"$VER\"/" \
  python/pyproject.toml
rm -f python/pyproject.toml.bak

# Update R DESCRIPTION
sed -i.bak -E \
  "s/^Version:[[:space:]]*.*/Version: $VER/" \
  r-package/DESCRIPTION
rm -f r-package/DESCRIPTION.bak

# Update Julia Project.toml
sed -i.bak -E \
  "s/^version[[:space:]]*=[[:space:]]*\".*\"/version = \"$VER\"/" \
  Randompack.jl/Project.toml
rm -f Randompack.jl/Project.toml.bak

# Update Julia/Yggdrasil recipe
sed -i.bak -E \
  "s/^version[[:space:]]*=[[:space:]]*v\".*\"/version = v\"$VER\"/" \
  packaging/build_tarballs.jl
rm -f packaging/build_tarballs.jl.bak

# Update meson.build (version: 'x.y.z')
sed -i.bak -E \
  "s/^  version[[:space:]]*:[[:space:]]*'.*'/  version: '$VER'/" \
  meson.build
rm -f meson.build.bak

# Update python/meson.build (version: 'x.y.z')
sed -i.bak -E \
  "s/^  version[[:space:]]*:[[:space:]]*'.*'/  version: '$VER'/" \
  python/meson.build
rm -f python/meson.build.bak

echo "Version set to $VER"
