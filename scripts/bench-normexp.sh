#!/bin/sh
set -e

root_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
release_dir="$root_dir/release"
modified_dir="$root_dir/modified"

get_build_option()
{
  build_dir=$1
  option_name=$2
  meson introspect --buildoptions "$build_dir" | python3 -c '
import json
import sys

option_name = sys.argv[1]
for option in json.load(sys.stdin):
    if option["name"] == option_name:
        value = option["value"]
        if isinstance(value, bool):
            print("true" if value else "false")
        else:
            print(value)
        raise SystemExit(0)
raise SystemExit(1)
' "$option_name"
}

ensure_build_dir()
{
  build_dir=$1

  if [ ! -d "$build_dir" ]; then
    meson setup "$build_dir" -Dbuildtype=release
    return
  fi

  buildtype=$(get_build_option "$build_dir" buildtype)
  if [ "$buildtype" != "release" ]; then
    meson setup --reconfigure "$build_dir" -Dbuildtype=release
  fi
}

ensure_build_dir "$release_dir"
ensure_build_dir "$modified_dir"

ninja -C "$release_dir" benchmark/TimeNormExp
ninja -C "$modified_dir" benchmark/TimeNormExp

printf "\n=== modified ===\n"
modified_out=$("$modified_dir/benchmark/TimeNormExp" "$@")
printf "%s\n" "$modified_out"

printf "\n=== release ===\n"
release_out=$("$release_dir/benchmark/TimeNormExp" "$@")
printf "%s\n" "$release_out"

modified_norm=$(printf "%s\n" "$modified_out" | awk '/^norm / {print $2}')
modified_exp=$(printf "%s\n" "$modified_out" | awk '/^exp  / {print $2}')
release_norm=$(printf "%s\n" "$release_out" | awk '/^norm / {print $2}')
release_exp=$(printf "%s\n" "$release_out" | awk '/^exp  / {print $2}')

printf "\n"
python3 - "$release_norm" "$modified_norm" "$release_exp" "$modified_exp" <<'PY'
import sys


def describe(label, release, modified):
    pct = 100*(modified - release)/modified
    relation = "faster" if pct >= 0 else "slower"
    print(f"{label:<4} {abs(pct):5.2f}% {relation}")


describe("norm", float(sys.argv[1]), float(sys.argv[2]))
describe("exp", float(sys.argv[3]), float(sys.argv[4]))
PY
