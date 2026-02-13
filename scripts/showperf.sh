set -eu
[ -f .randompack-root ] || {
  echo "showperf.sh: run this from the repository root (missing .randompack-root)" 1>&2
  exit 1
}
cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
