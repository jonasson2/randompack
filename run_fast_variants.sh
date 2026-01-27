#!/bin/sh
set -e
# All variants are NEON only except fast22 which is the production one.

variants="fast11 fast21 fast21soa fast41 fast41soa fast12 fast12soa fast14 fast14soa fast22 fast22soa"

for v in $variants; do
  cp "src/${v}.inc" src/fast.inc
  ninja -C release >/dev/null
  printf "%s " "$v"
  release/examples/TimeFast
done
