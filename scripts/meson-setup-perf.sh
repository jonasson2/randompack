set -eu
[ -f .randompack-root ] || {
  echo "meson-setup-perf.sh: run this from the repository root (missing .randompack-root)" 1>&2
  exit 1
}
rm -rf perf
meson setup perf -Dbuildtype=release -Dperf=true
ninja -C perf
