#!/bin/bash

set -eu

overdisp=
pos=
while [ $# -gt 0 ]
do
  case "$1" in
    -o)
      shift
      if [ $# -lt 1 ]
      then
        echo "usage: $0 [-o x.xxx,y.yyy,z.zzz] DIR [NFAM] [EXCLUDE_FAMILY]" >&2
        exit 1
      fi
      overdisp=$1
      ;;
    -o*)
      overdisp=${1#-o}
      ;;
    -*)
      echo "usage: $0 [-o x.xxx,y.yyy,z.zzz] DIR [NFAM] [EXCLUDE_FAMILY]" >&2
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
  echo "usage: $0 [-o x.xxx,y.yyy,z.zzz] DIR [NFAM] [EXCLUDE_FAMILY]" >&2
  exit 1
fi

dir=$1
nfam=${2:-0}
exclude_family=${3-}

if [ -n "$overdisp" ]
then
  ./extract-dir.sh "$dir" | python3 summary.py -o "$overdisp" "$dir" "$nfam" "$exclude_family"
else
  ./extract-dir.sh "$dir" | python3 summary.py "$dir" "$nfam" "$exclude_family"
fi
