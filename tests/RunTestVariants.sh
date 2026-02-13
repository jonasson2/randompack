#!/bin/sh
set -e

build_root="${1:-.}"
"${build_root}/RunTestNo128" -v
"${build_root}/RunTestNoSimd" -v
