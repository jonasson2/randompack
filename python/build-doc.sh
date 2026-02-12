#!/bin/sh
set -eu
rm -rf docs/_build docs/reference/generated
sphinx-build -E -b html docs docs/_build
command -v open >/dev/null 2>&1 && open docs/_build/index.html || true
