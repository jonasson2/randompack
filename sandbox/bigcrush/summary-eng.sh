#!/bin/bash

set -eu

if [ $# -ne 2 ] && [ $# -ne 3 ]
then
  echo "usage: $0 DIR ENGINE [EXCLUDE_FAMILY]" >&2
  exit 1
fi

dir=$1
engine=$2
exclude_family=${3-}
script_dir=$(cd "$(dirname "$0")" && pwd)

"$script_dir"/extract-dir.sh "$dir" | awk -vengine="$engine" '$2 == engine' \
  | python3 "$script_dir"/summary-eng.py "$dir" "$engine" "$exclude_family"
