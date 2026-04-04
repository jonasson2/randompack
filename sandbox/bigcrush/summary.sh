#!/bin/bash

set -eu

overdisp=
nlowest=
pos=
while [ $# -gt 0 ]
do
  case "$1" in
    -o)
      shift
      if [ $# -lt 1 ]
      then
        echo "usage: $0 [-o w.xxx,x.xxx,y.yyy,z.zzz] DIR [NFAM] [EXCLUDE_FAMILY]" >&2
        exit 1
      fi
      overdisp=$1
      ;;
    -o*)
      overdisp=${1#-o}
      ;;
    -l)
      shift
      if [ $# -lt 1 ]
      then
        echo "usage: $0 [-o w.xxx,x.xxx,y.yyy,z.zzz] [-l N] DIR [NFAM] [EXCLUDE_FAMILY]" >&2
        exit 1
      fi
      nlowest=$1
      ;;
    -l*)
      nlowest=${1#-l}
      ;;
    -*)
      echo "usage: $0 [-o w.xxx,x.xxx,y.yyy,z.zzz] [-l N] DIR [NFAM] [EXCLUDE_FAMILY]" >&2
      exit 1
      ;;
    *)
      pos="${pos}${pos:+
}$1"
      ;;
  esac
  shift
done

set -- $(printf '%s\n' "$pos")

if [ $# -lt 1 ] || [ $# -gt 3 ]
then
  echo "usage: $0 [-o w.xxx,x.xxx,y.yyy,z.zzz] [-l N] DIR [NFAM] [EXCLUDE_FAMILY]" >&2
  exit 1
fi

dir=$1
nfam=${2:-0}
exclude_family=${3-}

set -- python3 summary.py
if [ -n "$overdisp" ]
then
  set -- "$@" -o "$overdisp"
fi
if [ -n "$nlowest" ]
then
  set -- "$@" -l "$nlowest"
fi
set -- "$@" "$dir" "$nfam" "$exclude_family"
./extract-dir.sh "$dir" | "$@"
