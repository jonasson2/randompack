#!/bin/sh
# Make sure we've committed and pushed
if [ -n "$(git status --porcelain)" ] || [ -n "$(git log origin/main..HEAD)" ]; then
  echo "ERROR: commit and push before building wheels"
  exit 1
fi

# Delete all previous successful runs
gh run list --status success --json databaseId --jq '.[].databaseId' | \
  xargs -I{} gh run delete {}

echo Remember to git commit and push
gh workflow run linux.yml
gh workflow run linux-arm.yml
gh workflow run mac-arm.yml
gh workflow run mac-x86.yml
gh workflow run windows.yml
gh workflow run docs.yml
