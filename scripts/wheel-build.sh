#!/bin/sh
echo Remember to git commit and push
gh workflow run linux.yml
#gh workflow run linux-arm.yml
gh workflow run mac-arm.yml
gh workflow run mac-x86.yml
gh workflow run windows.yml
gh workflow run docs.yml
