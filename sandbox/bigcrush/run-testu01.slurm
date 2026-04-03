#!/bin/bash
#SBATCH --job-name=testu01
#SBATCH --partition=mimir,mimir-interactive,gpu-1xA100,gpu-2xA100
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=60

set -eu

ENGINES=(
  "x256++"
  "x256**"
  "x128+"
  "xoro++"
  "pcg64"
  "squares"
  "philox"
  "sfc64"
  "cwg128"
  "ranlux++"
  "chacha20"
)

MAXPAR=60

if [ $# -lt 2 ]; then
  echo "usage: sbatch run_testu01_all.slurm OUTDIR FIRSTSEED [TestU01Driver args...]" >&2
  exit 1
fi

OUTDIR="$HOME/$1"
FIRSTSEED="$2"
shift 2
ARGS=( "$@" )

SEEDS=( "$FIRSTSEED" "$((FIRSTSEED+1))" "$((FIRSTSEED+2))" "$((FIRSTSEED+3))" "$((FIRSTSEED+4))" )

MODE=""
for arg in "${ARGS[@]}"; do
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

if [ -z "$MODE" ]; then
  echo "one of -S, -C, -B must be supplied" >&2
  exit 1
fi

ROOT="$HOME/randompack"
EXE="$ROOT/release/examples/TestU01Driver"
LOGDIR="$OUTDIR/slurm"
RUNSFILE="$OUTDIR/runs.tsv"

mkdir -p "$OUTDIR" "$LOGDIR"

printf "engine\tseed\tmode\toutfile\terrfile\tjobid\tnode\n" >"$RUNSFILE"

run_one() {
  local engine="$1"
  local seed="$2"
  local outfile="$OUTDIR/${engine}_s${seed}.txt"
  local errfile="$LOGDIR/${engine}_s${seed}.err"

  "$EXE" "-e$engine" "-s" "$seed" "${ARGS[@]}" >"$outfile" 2>"$errfile"

  elapsed_min=$(( ($(date +%s) - start)/60 ))
  printf "\ncompleted: %s, elapsed_min: %s\n" \
    "$(date '+%Y-%m-%d %H:%M:%S %Z')" "$elapsed_min" >>"$outfile"
}

running=0

for engine in "${ENGINES[@]}"; do
  for seed in "${SEEDS[@]}"; do
    printf "%s\t%s\t%s\t%s\t%s\t%s\t%s\n" \
      "$engine" \
      "$seed" \
      "$MODE" \
      "${engine}_s${seed}.txt" \
      "slurm/${engine}_s${seed}.err" \
      "${SLURM_JOB_ID:-}" \
      "${SLURMD_NODENAME:-}" >>"$RUNSFILE"

    (
      start=$(date +%s)
      run_one "$engine" "$seed"
    ) &

    running=$((running+1))
    if [ "$running" -ge "$MAXPAR" ]; then
      wait -n
      running=$((running-1))
    fi
  done
done

wait
