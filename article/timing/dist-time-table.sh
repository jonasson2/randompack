#!/bin/sh
set -eu

print_help() {
  cat <<EOF
Usage: article/timing/dist-time-table.sh

Read the timing files listed on the last
  %% .out files: ...
comment line in article/toms.tex and print only the LaTeX body rows for the
distribution table that follows it.

The output is intended to be pasted between that comment line and \\\\bottomrule.
Missing values are printed as \\NA. Empty separator columns are printed as &&.
EOF
}

if [ $# -gt 0 ]; then
  case "$1" in
    -h|--help)
      print_help
      exit 0
      ;;
    *)
      print_help 1>&2
      exit 1
      ;;
  esac
fi

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
toms="$script_dir/../toms.tex"
comment=$(
  awk '/^%% \.out files:/{line=$0} END{if (line) print line}' "$toms"
)

if [ -z "$comment" ]; then
  echo "dist-time-table.sh: no '%% .out files:' line found in article/toms.tex" >&2
  exit 1
fi

awk -v dir="$script_dir" -v comment="$comment" '
  function trim(s) {
    sub(/^[[:space:]]+/, "", s)
    sub(/[[:space:]]+$/, "", s)
    return s
  }
  function normalize_file(s) {
    s = trim(s)
    if (s == "") return ""
    if (s !~ /\.out$/) s = s ".out"
    return s
  }
  function parse_comment(line,    n, i, j, groups, toks, tok) {
    sub(/^.*:[[:space:]]*/, "", line)
    ngroups = split(line, groups, /[[:space:]]*&&[[:space:]]*/)
    nfiles = 0
    for (i = 1; i <= ngroups; i++) {
      group_start[i] = nfiles + 1
      group_len[i] = 0
      n = split(groups[i], toks, /[[:space:]]*&[[:space:]]*/)
      for (j = 1; j <= n; j++) {
        tok = normalize_file(toks[j])
        if (tok == "") continue
        files[++nfiles] = tok
        group_len[i]++
      }
    }
  }
  function canonical(name) {
    if (name == "u01" || name == "unif(0,1)") return "u01"
    if (name == "norm" || name == "std.normal") return "norm"
    if (name == "exp(1)" || name == "std.exp") return "exp(1)"
    return name
  }
  function display_name(key) {
    if (key == "u01") return "$U(0,1)$"
    if (key == "unif(2,5)") return "$U(2,5)$"
    if (key == "norm") return "Standard normal"
    if (key == "normal(2,3)") return "Normal$(2,3)$"
    if (key == "exp(1)") return "Exponential"
    if (key == "lognormal(0,1)") return "Log-normal"
    if (key == "skew-normal(0,1,5)") return "Skew-normal"
    if (key == "gumbel(0,1)") return "Gumbel"
    if (key == "pareto(1,2)") return "Pareto"
    if (key == "gamma(2,3)") return "Gamma$(2,3)$"
    if (key == "beta(2,5)") return "Beta$(2,5)$"
    if (key == "chi2(5)") return "Chi-square$(5)$"
    if (key == "t(10)") return "$t(10)$"
    if (key == "F(5,10)") return "$F(5,10)$"
    if (key == "weibull(2,3)") return "Weibull$(2,3)$"
    return key
  }
  function keep_key(key) {
    return key == "u01" ||
           key == "unif(2,5)" ||
           key == "norm" ||
           key == "normal(2,3)" ||
           key == "exp(1)" ||
           key == "lognormal(0,1)" ||
           key == "skew-normal(0,1,5)" ||
           key == "gumbel(0,1)" ||
           key == "pareto(1,2)" ||
           key == "gamma(2,3)" ||
           key == "beta(2,5)" ||
           key == "chi2(5)" ||
           key == "t(10)" ||
           key == "F(5,10)" ||
           key == "weibull(2,3)"
  }
  function add_key(key,    w) {
    if (!keep_key(key)) return
    if (!(key in key_seen)) {
      key_seen[key] = 1
      key_order[++nkeys] = key
      w = length(display_name(key))
      if (w > distw) distw = w
    }
  }
  function file_fmt(idx) {
    if (idx <= 4) return "%.2f"
    return "%.1f"
  }
  function cell_text(key, idx,    fmt) {
    if (val[key, idx] == "") return "\\NA"
    fmt = file_fmt(idx)
    return sprintf(fmt, val[key, idx])
  }
  function load_file(idx, path,    line, n, key) {
    path = dir "/" files[idx]
    while ((getline line < path) > 0) {
      n = split(line, fields, /[[:space:]]+/)
      if (n < 2) continue
      key = canonical(fields[1])
      if (!keep_key(key)) continue
      val[key, idx] = fields[2] + 0
      add_key(key)
    }
    close(path)
  }
  BEGIN {
    parse_comment(comment)
    if (ngroups != 4 || nfiles != 12) {
      print "dist-time-table.sh: expected 4 groups and 12 files in comment line" > "/dev/stderr"
      exit 1
    }

    distw = length("Distribution")
    add_key("u01")
    add_key("unif(2,5)")
    add_key("norm")
    add_key("normal(2,3)")
    add_key("exp(1)")
    add_key("lognormal(0,1)")
    add_key("skew-normal(0,1,5)")
    add_key("gumbel(0,1)")
    add_key("pareto(1,2)")
    add_key("gamma(2,3)")
    add_key("beta(2,5)")
    add_key("chi2(5)")
    add_key("t(10)")
    add_key("F(5,10)")
    add_key("weibull(2,3)")

    for (i = 1; i <= nfiles; i++) {
      colw[i] = 3
      load_file(i)
    }

    for (i = 1; i <= nkeys; i++) {
      key = key_order[i]
      for (j = 1; j <= nfiles; j++) {
        txt = cell_text(key, j)
        if (length(txt) > colw[j]) colw[j] = length(txt)
      }
    }

    for (i = 1; i <= nkeys; i++) {
      key = key_order[i]
      printf "%-*s", distw, display_name(key)
      for (g = 1; g <= ngroups; g++) {
        start = group_start[g]
        stop = start + group_len[g] - 1
        if (g == 1) {
          for (j = start; j <= stop; j++) {
            printf " & %*s", colw[j], cell_text(key, j)
          }
        }
        else {
          printf " && %*s", colw[start], cell_text(key, start)
          for (j = start + 1; j <= stop; j++) {
            printf " & %*s", colw[j], cell_text(key, j)
          }
        }
      }
      printf " \\\\\n"
    }
  }
' /dev/null
