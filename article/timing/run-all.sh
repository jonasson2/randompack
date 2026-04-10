#!/bin/sh
set -eu

print_help() {
  cat <<EOF
Usage: article/timing/run-all.sh

Run the standard timing matrix and write the standard output files.

Outputs:
  mac.out
  spark.out
  xeon.out
  i5.out
  rp-py-mac.out
  rp-r-mac.out
  rp-python.out
  rp-r.out
  rp-julia.out
  cpp.out
  numpy.out
  r.out
  julia.out
EOF
}

run_case() {
  host="$1"
  column="$2"
  language="$3"
  outfile="$4"
  printf "\n==> %s %s %s -> %s\n" "$host" "$column" "$language" "$outfile"
  "$mean_time" -H "$host" "$column" "$language" -r 10 -o "$outdir/$outfile"
}

case "${1:-}" in
  -h|--help)
    print_help
    exit 0
    ;;
esac

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
mean_time="$script_dir/mean-time.sh"
outdir="$script_dir"

if [ ! -x "$mean_time" ]; then
  echo "run-all.sh: missing executable $mean_time" 1>&2
  exit 1
fi

run_case pluto -2 -c cpp.out
run_case pluto -2 -p numpy.out
run_case pluto -2 -R r.out
run_case pluto -2 -j julia.out
run_case mac -3 -p rp-py-mac.out
run_case mac -3 -R rp-r-mac.out
run_case pluto -3 -p rp-python.out
run_case pluto -3 -R rp-r.out
run_case pluto -3 -j rp-julia.out
run_case mac -2 -C mac.out
run_case spark1 -2 -C spark.out
run_case pluto -2 -C i5.out
run_case elja -2 -C xeon.out
