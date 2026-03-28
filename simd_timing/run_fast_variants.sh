#!/bin/sh
set -e
# All variants are NEON only except fast22 which is the production one.

variants="SoA_2x1 SoA_1x2 SoA_2x2 SoA_4x1 SoA_1x4 "
variants+="AoS_2x1 AoS_1x2 AoS_2x2 AoS_4x1 AoS_1x4 "
variants+="AoS_1x1"

for v in $variants; do
  cp "src/${v}.inc" src/fast.inc
  ninja -C release >/dev/null
  printf "%s " "$v"
  release/benchmark/TimeFast
done
