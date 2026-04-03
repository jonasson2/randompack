#!/bin/bash

set -eu

if [ $# -ne 2 ]
then
  echo "usage: $0 DIR FAMILY" >&2
  exit 1
fi

dir=$1
family=$2
script_dir=$(cd "$(dirname "$0")" && pwd)

"$script_dir"/extract-dir.sh "$dir" | python3 "$script_dir"/summary-fam.py "$dir" "$family"
