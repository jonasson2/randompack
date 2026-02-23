#!/bin/sh
# Download successfully built wheels from github
set -eu
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
REPO_ROOT=$(dirname "$SCRIPT_DIR")
cd "$REPO_ROOT"/python
pwd
# Clean the wheelhouse and dist:
rm -rf wheelhouse/*
rm dist/*.whl

# Get run id-s of successful wheel-builds:
ids=$(gh run list --status success --json databaseId --jq '.[].databaseId')
echo $ids

# Download them to wheelhouse:
for id in $ids; do
  echo $id
  gh run download "$id" -D wheelhouse
done

# Copy to dist and check:
cp wheelhouse/*/*.whl dist
python -m twine check dist/*
