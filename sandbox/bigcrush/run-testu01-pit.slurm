#!/bin/bash
#SBATCH --job-name=testu01-pit
#SBATCH --partition=mimir,mimir-interactive,gpu-1xA100,gpu-2xA100
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=64

set -eu

MAXPAR=64

if [ $# -lt 3 ]; then
  echo "usage: sbatch run-testu01-pit.slurm OUTDIR FIRSTSEED NSEEDS [TestU01PIT args...]" >&2
  exit 1
fi

OUTDIR="$HOME/$1"
FIRSTSEED="$2"
NSEEDS="$3"
shift 3
ARGS=( "$@" )

case "$NSEEDS" in
  ''|*[!0-9]*)
    echo "NSEEDS must be a positive integer" >&2
    exit 1
    ;;
esac

if [ "$NSEEDS" -le 0 ]; then
  echo "NSEEDS must be a positive integer" >&2
  exit 1
fi

MODE=""
ENGINE="x256++simd"
i=0
while [ "$i" -lt "${#ARGS[@]}" ]; do
  arg="${ARGS[$i]}"
  case "$arg" in
    -S|-C|-B)
      MODE="$arg"
      ;;
    -s|-s*)
      echo "do not pass -s; seeds are selected by this script" >&2
      exit 1
      ;;
    -g)
      i=$((i+1))
      if [ "$i" -ge "${#ARGS[@]}" ]; then
        echo "missing engine after -g" >&2
        exit 1
      fi
      ENGINE="${ARGS[$i]}"
      ;;
    -g*)
      ENGINE="${arg#-g}"
      ;;
  esac
  i=$((i+1))
done

if [ -z "$MODE" ]; then
  echo "one of -S, -C, -B must be supplied" >&2
  exit 1
fi

ROOT="$HOME/randompack"
EXE="$ROOT/release/examples/TestU01PIT"
LOGDIR="$OUTDIR/slurm"

mkdir -p "$OUTDIR" "$LOGDIR"

run_one() {
  local seed="$1"
  local outfile="$OUTDIR/${engine}_s${seed}.txt"
  local errfile="$LOGDIR/${engine}_s${seed}.err"

  "$EXE" "${ARGS[@]}" "-s" "$seed" >"$outfile" 2>"$errfile"

  elapsed_min=$(( ($(date +%s) - start)/60 ))
  printf "\ncompleted: %s, elapsed_min: %s\n" \
    "$(date '+%Y-%m-%d %H:%M:%S %Z')" "$elapsed_min" >>"$outfile"
}

running=0

for ((offset = 0; offset < NSEEDS; offset++)); do
  seed=$((FIRSTSEED + offset))
  (
    engine="$ENGINE"
    start=$(date +%s)
    run_one "$seed"
  ) &

  running=$((running+1))
  if [ "$running" -ge "$MAXPAR" ]; then
    wait -n
    running=$((running-1))
  fi
done

wait
