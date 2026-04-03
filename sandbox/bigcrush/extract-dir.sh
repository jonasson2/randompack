#!/bin/bash

set -eu

dir=${1:-test1}
marker='========= Summary results of BigCrush'

if [ ! -d "$dir" ]
then
  echo "directory not found: $dir" >&2
  exit 1
fi

for file in "$dir"/*_s*.txt
do
  [ -e "$file" ] || continue
  ./extract.sh "$file"
done
