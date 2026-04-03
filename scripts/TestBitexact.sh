#!/bin/sh
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
REPO_ROOT=$(dirname "$SCRIPT_DIR")
cd "$REPO_ROOT"

remote_host=
remote_folder=randompack
builddir=release
seed=123
ndraws=1e8
dist='N(0,1)'
params=
have_params=false
engine='x256++simd'
precision=double
set_bitexact=true
print_host=${TESTBITEXACT_PRINT_HOST-1}

quote_sh() {
  printf "'%s'" "$(printf "%s" "$1" | sed "s/'/'\\\\''/g")"
}

usage() {
  cat 1>&2 <<EOF
TestBitexact.sh - compare deterministic draw summaries locally and remotely

Usage:
  scripts/TestBitexact.sh [options]

Options:
  -h                Show this help message
  -r REMOTE_HOST    Remote host to run the same test on
                    Default: none
  -f REMOTE_FOLDER  Remote repo folder
                    Default: randompack
  -b BUILD          Build folder
                    Default: release
  -s SEED           Seed
                    Default: 123
  -n NDRAWS         Number of draws
                    Default: 1e8
  -d DIST           Distribution
                    Default: N(0,1)
  -p PARAMS         Distribution parameters, comma-separated
                    Default: distribution-dependent
  -e ENGINE         RNG engine
                    Default: x256++simd
  -P PRECISION      Precision: double or float
                    Default: double
  -x                Do not set bitexact mode
                    Default: off

Distributions:
  u01, unif, norm, normal, exp, lognormal, gamma, beta, chi2, t, f,
  gumbel, pareto, weibull, skew_normal
EOF
}

while getopts "hr:f:b:s:n:d:p:e:P:x" opt
do
  case "$opt" in
    h) usage; exit 0 ;;
    r) remote_host=$OPTARG ;;
    f) remote_folder=$OPTARG ;;
    b) builddir=$OPTARG ;;
    s) seed=$OPTARG ;;
    n) ndraws=$OPTARG ;;
    d) dist=$OPTARG ;;
    p) params=$OPTARG; have_params=true ;;
    e) engine=$OPTARG ;;
    P) precision=$OPTARG ;;
    x) set_bitexact=false ;;
    *) usage; exit 1 ;;
  esac
done

ninja -C "$builddir" examples/TestBitexact >/dev/null
set -- "$builddir/examples/TestBitexact" -s "$seed" -n "$ndraws" -d "$dist" -e "$engine" -P "$precision"
if [ "$have_params" = true ]; then
  set -- "$@" -p "$params"
fi
if [ "$set_bitexact" = false ]; then
  set -- "$@" -x
fi
if [ "$print_host" = 1 ]; then
  printf "%-10s %s\n" "host:" "local"
fi
"$@"

if [ -n "$remote_host" ]; then
  remote_cmd="cd $(quote_sh "$remote_folder") && TESTBITEXACT_PRINT_HOST=0 scripts/TestBitexact.sh -f $(quote_sh "$remote_folder") -b $(quote_sh "$builddir") -s $(quote_sh "$seed") -n $(quote_sh "$ndraws") -d $(quote_sh "$dist") -e $(quote_sh "$engine") -P $(quote_sh "$precision")"
  if [ "$have_params" = true ]; then
    remote_cmd="$remote_cmd -p $(quote_sh "$params")"
  fi
  if [ "$set_bitexact" = false ]; then
    remote_cmd="$remote_cmd -x"
  fi
  if [ "$print_host" = 1 ]; then
    printf "\n"
    printf "%-10s %s\n" "host:" "$remote_host"
  fi
  ssh "$remote_host" "$remote_cmd"
fi
