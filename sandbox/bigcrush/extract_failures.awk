#!/usr/bin/env -S gawk -f
# Usage:
#   gawk -f extract_failures.awk test1/*.txt

function trim(s) {
  sub(/^[[:space:]]+/, "", s)
  sub(/[[:space:]]+$/, "", s)
  return s
}

function fmt_exp(s) {
  sub(/e\+0*/, "e+", s)
  sub(/e-0*/, "e-", s)
  return s
}

function fmt_p(s,    x) {
  s = trim(s)

  if (s ~ /^1[[:space:]]*-[[:space:]]*/) {
    sub(/^1[[:space:]]*-[[:space:]]*/, "", s)
    x = s + 0
    return fmt_exp(sprintf("%.1e", x))
  }

  x = s + 0
  if (x > 0.5) x = 1.0 - x
  return fmt_exp(sprintf("%.1e", x))
}

FNR == 1 {
  inblock = 0
  engine = ""
  seed = ""

  file = FILENAME
  sub(/^.*\//, "", file)
  sub(/\.txt$/, "", file)

  if (file ~ /_s[0-9]+$/) {
    engine = file
    sub(/_s[0-9]+$/, "", engine)
    seed = file
    sub(/^.*_s/, "", seed)
  }
}

/^ The following tests gave p-values outside / {
  inblock = 1
  next
}

inblock && /^ All other tests were passed/ { inblock = 0; next }
inblock && /^ ----------------------------------------------/ { next }
inblock && /^\(eps/ { next }
inblock && /^ *Test +p-value/ { next }

inblock && /^[[:space:]]*[0-9]+[[:space:]]/ {
  line = $0
  line = trim(line)

  # Remove leading test number.
  if (!match(line, /^[0-9]+[[:space:]]+/)) next
  testno = substr(line, 1, RLENGTH)
  sub(/[[:space:]]+$/, "", testno)
  line = substr(line, RLENGTH + 1)

  # Extract trailing p-value.
  if (!match(line, /(1[[:space:]]*-[[:space:]]*)?[0-9.]+([eE][-+]?[0-9]+)?[[:space:]]*$/))
    next

  p = substr(line, RSTART, RLENGTH)
  testname = substr(line, 1, RSTART - 1)

  p = trim(p)
  testname = trim(testname)

  printf "%-11s %1s %3s  %-30s %7s\n",
    engine, seed, testno, testname, fmt_p(p)
}
