#!/bin/sh
set -eu
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
REPO_ROOT=$(dirname "$SCRIPT_DIR")
cd "$REPO_ROOT"

print_help() {
  cat 1>&2 <<EOF
Usage: scripts/meson-setup.sh <builddir> [meson args...]
       CC=<C-compiler> scripts/meson-setup.sh <builddir> [meson args...]
       cc=<C-compiler> scripts/meson-setup.sh <builddir> [meson args...]

Set up a Meson build directory for randompack.

Notes:
  buildtype=release is the default.
  The install prefix is set to install/.
  The library directory is install/lib/.
  If <builddir> already exists, it is removed first.
  Additional Meson arguments are passed through unchanged.
  If CC and cc are unset, clang is preferred when available, otherwise cc is
  used. If both are set, CC takes precedence.
EOF
}

if [ $# -lt 1 ]; then
  print_help
  exit 1
fi

if [ "$1" = "-h" ] || [ "$1" = "--help" ]; then
  print_help
  exit 0
fi

builddir="$1"
shift

if [ -e "$builddir" ]; then
  rm -rf "$builddir"
fi

buildtype="release"
while [ $# -gt 0 ]; do
  case "$1" in
    --buildtype=*)
      buildtype="${1#--buildtype=}"
      shift
      ;;
    -Dbuildtype=*)
      buildtype="${1#-Dbuildtype=}"
      shift
      ;;
    --buildtype)
      shift
      if [ $# -lt 1 ]; then
        print_help
        exit 1
      fi
      buildtype="$1"
      shift
      ;;
    *)
      break
      ;;
  esac
done

case "$buildtype" in
  release|debug)
    ;;
  *)
    print_help
    exit 1
    ;;
esac

if [ -n "${cc:-}" ] && [ -z "${CC:-}" ]; then
  CC="$cc"
  export CC
fi

if [ -z "${CC:-}" ]; then
  if command -v clang >/dev/null 2>&1; then
    CC=clang
  else
    CC=cc
  fi
  export CC
fi

meson setup "$builddir" \
  --prefix "$PWD/install" \
  -Dlibdir=lib \
  -Dbuildtype="$buildtype" \
  "$@"
