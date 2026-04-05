#!/bin/bash
#SBATCH --job-name=testu01-eng
#SBATCH --partition=mimir,mimir-interactive
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=64

set -eu

MAXPAR=64

if [ $# -lt 3 ]
then
  echo "usage: sbatch run-testu01-eng.slurm ENGINE FIRSTSEED NSEEDS [TestU01Driver args...]" >&2
  exit 1
fi

ENGINE="$1"
FIRSTSEED="$2"
NSEEDS="$3"
shift 3
OUTDIR="$HOME/test-$ENGINE"
ARGS=( "$@" )

MODE=""
for arg in "${ARGS[@]}"
do
  case "$arg" in
    -S|-C|-B)
      MODE="$arg"
      ;;
    -e|-e*)
      echo "do not pass -e; engine is selected by this script" >&2
      exit 1
      ;;
    -s|-s*)
      echo "do not pass -s; seeds are selected by this script" >&2
      exit 1
      ;;
  esac
done

if [ -z "$MODE" ]
then
  echo "one of -S, -C, -B must be supplied" >&2
  exit 1
fi

ROOT="$HOME/randompack"
EXE="$ROOT/release/examples/TestU01Driver"
LOGDIR="$OUTDIR/slurm"

mkdir -p "$OUTDIR" "$LOGDIR"

run_one() {
  local seed="$1"
  local outfile="$OUTDIR/${ENGINE}_s${seed}.txt"
  local errfile="$LOGDIR/${ENGINE}_s${seed}.err"

  "$EXE" "-e$ENGINE" "-s" "$seed" "${ARGS[@]}" >"$outfile" 2>"$errfile"

  elapsed_min=$(( ($(date +%s) - start)/60 ))
  printf "\ncompleted: %s, elapsed_min: %s\n" \
    "$(date '+%Y-%m-%d %H:%M:%S %Z')" "$elapsed_min" >>"$outfile"
}

running=0

for seed in $(seq "$FIRSTSEED" "$((FIRSTSEED+NSEEDS-1))")
do
  (
    start=$(date +%s)
    run_one "$seed"
  ) &

  running=$((running+1))
  if [ "$running" -ge "$MAXPAR" ]
  then
    wait -n
    running=$((running-1))
  fi
done

wait
