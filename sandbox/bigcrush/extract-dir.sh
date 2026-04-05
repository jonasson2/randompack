#!/bin/bash

set -eu

dir=${1:-test1}

if [ ! -d "$dir" ]
then
  echo "directory not found: $dir" >&2
  exit 1
fi

set -- "$dir"/*_s*.txt

if [ ! -e "$1" ]
then
  exit 0
fi

gawk -f extract.awk "$@"
