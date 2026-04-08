#!/bin/sh
set -eu

print_help() {
  cat <<EOF
Usage: article/timing/mean-time.sh [options] [benchmark args...]

Run a benchmark repeatedly and tabulate mean and standard deviation.

Options:
  -r n   Number of repetitions (default 5)
  -n n   Same as -r
  -o f   Write distribution means to file f
  -H h   Run the benchmark remotely on host h over ssh
  -D d   Remote repo root (default ~/randompack)
  -C     Run release/benchmark/TimeDistributions
  -p     Run python/examples/TimeDist.py
  -j     Run Randompack.jl/examples/TimeDist.jl
  -R     Run r-package/inst/examples/TimeDist.R
  -2     Use the second column
  -f     Use the third column instead of the default second column
  -3     Same as -f
  -4     Use the fourth column
  -k n   Use column n
  -h     Show this help

Notes:
  A sleep of 0.5 s is inserted between runs.
  The default benchmark is release/benchmark/TimeDistributions with -d 3.
  All remaining arguments are passed to the selected benchmark unchanged.
  In remote mode, only the timing program runs remotely.
  Use -H mac for the local machine.
EOF
}

quote_sh() {
  printf "'%s'" "$(printf "%s" "$1" | sed "s/'/'\\\\''/g")"
}

make_remote_cmd() {
  cmd=
  case "$mode" in
    c)
      cmd="release/benchmark/TimeDistributions -d 3"
      ;;
    python)
      cmd="python python/examples/TimeDist.py"
      ;;
    julia)
      cmd="JULIA_PROJECT=Randompack.jl julia Randompack.jl/examples/TimeDist.jl"
      ;;
    r)
      cmd="Rscript r-package/inst/examples/TimeDist.R"
      ;;
  esac
  for arg in "$@"; do
    cmd="$cmd $(quote_sh "$arg")"
  done
  if [ "$mode" = c ]; then
    fallback="release/benchmark/TimeDistributions"
    for arg in "$@"; do
      fallback="$fallback $(quote_sh "$arg")"
    done
    printf "cd %s && (%s || %s)" "$remote_dir" "$cmd" "$fallback"
  else
    printf "cd %s && %s" "$remote_dir" "$cmd"
  fi
}

run_remote_cmd() {
  cmd="$1"
  case "$remote_host" in
    elja)
      ssh "$remote_host" "bash -lc $(quote_sh "source ~/.bashrc; $cmd")"
      ;;
    *)
      ssh "$remote_host" "zsh -ic $(quote_sh "$cmd")"
      ;;
  esac
}

repeats=5
field=2
mode=c
outfile=
remote_host=
remote_dir='~/randompack'
pass_args=$(mktemp)
trap 'rm -f "$pass_args"' EXIT HUP INT TERM

while [ $# -gt 0 ]; do
  case "$1" in
    -r)
      shift
      if [ $# -lt 1 ]; then
        print_help 1>&2
        exit 1
      fi
      repeats="$1"
      ;;
    -n)
      shift
      if [ $# -lt 1 ]; then
        print_help 1>&2
        exit 1
      fi
      repeats="$1"
      ;;
    -o)
      shift
      if [ $# -lt 1 ]; then
        print_help 1>&2
        exit 1
      fi
      outfile="$1"
      ;;
    -H)
      shift
      if [ $# -lt 1 ]; then
        print_help 1>&2
        exit 1
      fi
      remote_host="$1"
      ;;
    -D)
      shift
      if [ $# -lt 1 ]; then
        print_help 1>&2
        exit 1
      fi
      remote_dir="$1"
      ;;
    -C)
      mode=c
      ;;
    -p)
      mode=python
      ;;
    -j)
      mode=julia
      ;;
    -R)
      mode=r
      ;;
    -f)
      field=3
      ;;
    -2)
      field=2
      ;;
    -3)
      field=3
      ;;
    -4)
      field=4
      ;;
    -k)
      shift
      if [ $# -lt 1 ]; then
        print_help 1>&2
        exit 1
      fi
      field="$1"
      ;;
    -h|--help)
      print_help
      exit 0
      ;;
    *)
      printf "%s\n" "$1" >> "$pass_args"
      ;;
  esac
  shift
done

if [ "$remote_host" = mac ]; then
  remote_host=
fi

case "$repeats" in
  ''|*[!0-9]*)
    print_help 1>&2
    exit 1
    ;;
esac

if [ "$repeats" -le 0 ]; then
  print_help 1>&2
  exit 1
fi

case "$field" in
  ''|*[!0-9]*)
    print_help 1>&2
    exit 1
    ;;
esac

if [ "$field" -le 1 ]; then
  print_help 1>&2
  exit 1
fi

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repo_root=$(dirname "$(dirname "$script_dir")")
time_dist="$repo_root/release/benchmark/TimeDistributions"
python_dist="$repo_root/python/examples/TimeDist.py"
julia_dist="$repo_root/Randompack.jl/examples/TimeDist.jl"
r_dist="$repo_root/r-package/inst/examples/TimeDist.R"

case "$mode" in
  c)
    if [ ! -x "$time_dist" ]; then
      echo "mean-time.sh: missing executable $time_dist" 1>&2
      exit 1
    fi
    ;;
  python)
    if [ ! -f "$python_dist" ]; then
      echo "mean-time.sh: missing script $python_dist" 1>&2
      exit 1
    fi
    ;;
  julia)
    if [ ! -f "$julia_dist" ]; then
      echo "mean-time.sh: missing script $julia_dist" 1>&2
      exit 1
    fi
    ;;
  r)
    if [ ! -f "$r_dist" ]; then
      echo "mean-time.sh: missing script $r_dist" 1>&2
      exit 1
    fi
    ;;
esac

tmp=$(mktemp)
trap 'rm -f "$tmp" "$pass_args"' EXIT HUP INT TERM

set --
while IFS= read -r arg; do
  set -- "$@" "$arg"
done < "$pass_args"

i=1
while [ "$i" -le "$repeats" ]; do
  printf "%d/%d\n" "$i" "$repeats" 1>&2
  if [ -n "$remote_host" ]; then
    remote_cmd=$(make_remote_cmd "$@")
    run_remote_cmd "$remote_cmd" \
      | awk -v mode="$mode" -v field="$field" '
          function trim(s) {
            sub(/^[[:space:]]+/, "", s)
            sub(/[[:space:]]+$/, "", s)
            return s
          }
          mode == "c" && /^Distribution[[:space:]]+/ { in_table = 1; next }
          mode == "julia" && /^Distribution[[:space:]]+/ { in_table = 1; next }
          mode == "python" && /^DISTRIBUTION[[:space:]]+/ { in_table = 1; next }
          mode != "r" && in_table && NF >= field && $(field) ~ /^[0-9.]+$/ {
            print $1 "\t" $(field)
          }
          mode == "r" && /^DISTRIBUTION[[:space:]]+/ { in_table = 1; next }
          mode == "r" && in_table {
            name = trim(substr($0, 1, 14))
            col2 = trim(substr($0, 16, 10))
            col3 = trim(substr($0, 27, 11))
            col4 = trim(substr($0, 39, 10))
            if (field == 2 && col2 ~ /^[0-9.]+$/) print name "\t" col2
            if (field == 3 && col3 ~ /^[0-9.]+$/) print name "\t" col3
            if (field == 4 && col4 ~ /^[0-9.]+$/) print name "\t" col4
          }
        ' >> "$tmp"
  else
    case "$mode" in
      c)
        "$time_dist" -d 3 "$@" \
          | awk -v field="$field" '
              /^Distribution[[:space:]]+/ { in_table = 1; next }
              in_table && NF >= field && $(field) ~ /^[0-9.]+$/ {
                print $1 "\t" $(field)
              }
            ' >> "$tmp"
        ;;
      python)
        python "$python_dist" "$@" \
          | awk -v field="$field" '
              /^DISTRIBUTION[[:space:]]+/ { in_table = 1; next }
              in_table && NF >= field && $(field) ~ /^[0-9.]+$/ {
                print $1 "\t" $(field)
              }
            ' >> "$tmp"
        ;;
      julia)
        JULIA_PROJECT="$repo_root/Randompack.jl" julia "$julia_dist" "$@" \
          | awk -v field="$field" '
              /^Distribution[[:space:]]+/ { in_table = 1; next }
              in_table && NF >= field && $(field) ~ /^[0-9.]+$/ {
                print $1 "\t" $(field)
              }
            ' >> "$tmp"
        ;;
      r)
        Rscript "$r_dist" "$@" \
          | awk -v field="$field" '
              function trim(s) {
                sub(/^[[:space:]]+/, "", s)
                sub(/[[:space:]]+$/, "", s)
                return s
              }
              /^DISTRIBUTION[[:space:]]+/ { in_table = 1; next }
              in_table {
                name = trim(substr($0, 1, 14))
                col2 = trim(substr($0, 16, 10))
                col3 = trim(substr($0, 27, 11))
                col4 = trim(substr($0, 39, 10))
                if (field == 2 && col2 ~ /^[0-9.]+$/) print name "\t" col2
                if (field == 3 && col3 ~ /^[0-9.]+$/) print name "\t" col3
                if (field == 4 && col4 ~ /^[0-9.]+$/) print name "\t" col4
              }
            ' >> "$tmp"
        ;;
    esac
  fi
  if [ "$i" -lt "$repeats" ]; then
    sleep 0.5
  fi
  i=$((i + 1))
done

awk '
  BEGIN {
    print "Distribution mean sd"
  }
  {
    name = $1
    x = $2 + 0
    if (!(name in seen)) {
      seen[name] = 1
      order[++norder] = name
      w = length(name)
      if (w > maxw) maxw = w
    }
    n[name]++
    sum[name] += x
    sum2[name] += x*x
  }
  END {
    if (maxw < 12) maxw = 12
    for (i = 1; i <= norder; i++) {
      name = order[i]
      mean = sum[name]/n[name]
      if (n[name] > 1) {
        var = (sum2[name] - sum[name]*sum[name]/n[name])/(n[name] - 1)
        if (var < 0) var = 0
        sd = sqrt(var)
      }
      else {
        sd = 0
      }
      printf "%-*s %7.3f %7.3f\n", maxw, name, mean, sd
      if (outfile != "") {
        if (i == 1)
          printf "%s %.3f\n", name, mean > outfile
        else
          printf "%s %.3f\n", name, mean >> outfile
      }
    }
  }
' outfile="$outfile" "$tmp"
