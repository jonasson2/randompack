#!/bin/sh
# Delete unsuccessful GitHub Actions runs
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
REPO_ROOT=$(dirname "$SCRIPT_DIR")
cd "$REPO_ROOT"

echo "Repo root: $(pwd)"

# Get run ids that are NOT successful
ids=$(gh run list \
  --limit 200 \
  --json databaseId,conclusion \
  --jq '.[] | select(.conclusion != "success") | .databaseId')

if [ -z "$ids" ]; then
  echo "No unsuccessful runs found."
  exit 0
fi

echo "Runs to delete:"
echo "$ids"

for id in $ids; do
  echo "Deleting run $id"
  gh run delete "$id"
done

echo "Done."
