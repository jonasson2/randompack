#!/bin/bash

set -eu

allow_partial=0
if [ "${1-}" = "-p" ]
then
  allow_partial=1
  shift
fi

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

if [ "$allow_partial" -eq 1 ]
then
  gawk -vallow_partial=1 -f extract.awk "$@"
else
  gawk -f extract.awk "$@"
fi
