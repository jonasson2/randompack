#!/bin/bash
#SBATCH --job-name=testu01-julia-pit
#SBATCH --partition=mimir,mimir-interactive,gpu-1xA100,gpu-2xA100
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=64

set -euo pipefail

ENGINE="julia"
MAXPAR=64

if [ $# -lt 3 ]
then
  echo "usage: sbatch run-testu01-julia-pit.slurm OUTDIR FIRSTSEED NSEEDS [TestU01PIT args...]" >&2
  exit 1
fi

OUTDIR="$HOME/$1"
FIRSTSEED="$2"
NSEEDS="$3"
shift 3
ARGS=( "$@" )

MODE=""
for arg in "${ARGS[@]}"
do
  case "$arg" in
    -S|-C|-B)
      MODE="$arg"
      ;;
    -g|-g*|-s|-s*|-i|-i*|-f|-f*|-e|-e*)
      echo "do not pass -g, -s, -i, -f or -e; this script uses Julia Float64 normals via stdin" >&2
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
EXE="$ROOT/release/examples"
LOGDIR="$OUTDIR/slurm"
JULIA_EXE="${JULIA:-julia}"
SCRIPT="$ROOT/sandbox/bigcrush/julia_normpipe.jl"

mkdir -p "$OUTDIR" "$LOGDIR"

run_one() {
  local seed="$1"
  local outfile="$OUTDIR/${ENGINE}_s${seed}.txt"
  local errfile="$LOGDIR/${ENGINE}_s${seed}.err"

  "$JULIA_EXE" "$SCRIPT" "$seed" | "$EXE/TestU01PIT" -i -n "${ARGS[@]}" >"$outfile" 2>"$errfile"

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
