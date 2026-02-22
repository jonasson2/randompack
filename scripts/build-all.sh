#!/bin/sh
set -eu
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
REPO_ROOT=$(dirname "$SCRIPT_DIR")
cd "$REPO_ROOT"

# Build all language interfaces (per DEVELOPMENT.md)
scripts/meson-setup.sh release
ninja -C release
ninja -C release install
scripts/syncpy.sh
scripts/syncR.sh
R CMD INSTALL r-package
