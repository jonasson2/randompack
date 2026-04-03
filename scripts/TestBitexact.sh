#!/bin/sh
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
REPO_ROOT=$(dirname "$SCRIPT_DIR")
cd "$REPO_ROOT"

remote_host=
remote_folder=randompack
builddir=release
ndraws=1e9
dist='N(0,1)'
params=
have_params=false
engine='x256++simd'
precision=double

quote_sh() {
  printf "'%s'" "$(printf "%s" "$1" | sed "s/'/'\\\\''/g")"
}

usage() {
  echo "usage: scripts/TestBitexact.sh [-h remote-host] [-f remote-folder] [-b remote-build] [-n number-of-draws] [-d distribution] [-p parameters] [-e engine] [-P precision]" 1>&2
}

while getopts "h:f:b:n:d:p:e:P:" opt
do
  case "$opt" in
    h) remote_host=$OPTARG ;;
    f) remote_folder=$OPTARG ;;
    b) builddir=$OPTARG ;;
    n) ndraws=$OPTARG ;;
    d) dist=$OPTARG ;;
    p) params=$OPTARG; have_params=true ;;
    e) engine=$OPTARG ;;
    P) precision=$OPTARG ;;
    *) usage; exit 1 ;;
  esac
done

ninja -C "$builddir" examples/TestBitexact >/dev/null
set -- "$builddir/examples/TestBitexact" -n "$ndraws" -d "$dist" -e "$engine" -P "$precision"
if [ "$have_params" = true ]; then
  set -- "$@" -p "$params"
fi
"$@"

if [ -n "$remote_host" ]; then
  remote_cmd="cd $(quote_sh "$remote_folder") && scripts/TestBitexact.sh -f $(quote_sh "$remote_folder") -b $(quote_sh "$builddir") -n $(quote_sh "$ndraws") -d $(quote_sh "$dist") -e $(quote_sh "$engine") -P $(quote_sh "$precision")"
  if [ "$have_params" = true ]; then
    remote_cmd="$remote_cmd -p $(quote_sh "$params")"
  fi
  ssh "$remote_host" "$remote_cmd"
fi
