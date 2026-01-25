#!/bin/sh
set -e
# All variants are NEON only except fast22 which is the production one.

variants="fast11 fast21 fast22 fast41 fast81"

for v in $variants; do
  cp "src/${v}.inc" src/fast.inc
  ninja -C release >/dev/null
  printf "%s " "$v"
  release/examples/TimeFast
done
