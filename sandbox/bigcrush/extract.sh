#!/bin/bash

set -eu

if [ $# -ne 1 ]
then
  echo "usage: $0 FILE" >&2
  exit 1
fi

file=$1

if [ ! -f "$file" ]
then
  echo "file not found: $file" >&2
  exit 1
fi

name=${file##*/}
engine=${name%_s*}
seed=${name##*_s}
seed=${seed%.txt}

gawk -vseed="$seed" -vengine="$engine" -f extract.awk "$file"
