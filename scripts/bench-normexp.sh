#!/bin/sh
set -e

root_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
release_dir="$root_dir/release"
modified_dir="$root_dir/modified"

if [ ! -f "$root_dir/src/norm_exp_mod.inc" ]; then
  cp "$root_dir/src/norm_exp.inc" "$root_dir/src/norm_exp_mod.inc"
fi

if [ -d "$release_dir" ]; then
  meson setup --reconfigure "$release_dir" -Dbuildtype=release
else
  meson setup "$release_dir" -Dbuildtype=release
fi

if [ -d "$modified_dir" ]; then
  meson setup --reconfigure "$modified_dir" -Dbuildtype=release -Dnorm_exp_mod=true
else
  meson setup "$modified_dir" -Dbuildtype=release -Dnorm_exp_mod=true
fi

ninja -C "$release_dir" examples/TimeNormExp
ninja -C "$modified_dir" examples/TimeNormExp

printf "=== release (norm_exp.inc) ===\n"
"$release_dir/examples/TimeNormExp" "$@"
printf "\n=== modified (norm_exp_mod.inc) ===\n"
"$modified_dir/examples/TimeNormExp" "$@"
