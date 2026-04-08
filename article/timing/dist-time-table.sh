#!/bin/sh
set -eu

print_help() {
  cat <<EOF
Usage: article/timing/dist-time-table.sh

Read timing summaries from standard output files in article/timing/ and print a
combined article table.

Expected files:
  mac.out        spark.out     xeon.out      i5.out
  rp-python.out  rp-r.out      rp-julia.out
  cpp.out        numpy.out     r.out         julia.out

Each file should contain rows in the format:
  distribution mean

Missing files are allowed and are shown as '-'.
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
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT HUP INT TERM

for f in \
  mac.out spark.out xeon.out i5.out \
  rp-python.out rp-r.out rp-julia.out \
  cpp.out numpy.out r.out julia.out
do
  if [ -f "$script_dir/$f" ]; then
    cp "$script_dir/$f" "$tmpdir/$f"
  else
    : > "$tmpdir/$f"
  fi
done

awk '
  function canonical(name) {
    if (name == "u01" || name == "unif(0,1)") return "u01"
    if (name == "norm" || name == "std.normal") return "norm"
    if (name == "exp(1)" || name == "std.exp") return "exp(1)"
    return name
  }
  function display_name(key) {
    if (key == "u01") return "U(0,1)"
    if (key == "unif(2,5)") return "U(2,5)"
    if (key == "norm") return "Standard normal"
    if (key == "normal(2,3)") return "Normal(2,3)"
    if (key == "exp(1)") return "Exp(1)"
    if (key == "exp(2)") return "Exp(2)"
    if (key == "lognormal(0,1)") return "Log-normal"
    if (key == "gumbel(0,1)") return "Gumbel"
    if (key == "pareto(1,2)") return "Pareto"
    if (key == "gamma(2,3)") return "Gamma(2,3)"
    if (key == "gamma(0.5,2)") return "Gamma(0.5,2)"
    if (key == "beta(2,5)") return "Beta(2,5)"
    if (key == "chi2(5)") return "Chi-square(5)"
    if (key == "t(10)") return "t(10)"
    if (key == "F(5,10)") return "F(5,10)"
    if (key == "weibull(2,3)") return "Weibull(2,3)"
    return key
  }
  function add_key(key) {
    if (!(key in key_seen)) {
      key_seen[key] = 1
      key_order[++nkeys] = key
      w = length(display_name(key))
      if (w > distw) distw = w
    }
  }
  function fmt(v) {
    if (v == "") return "-"
    return sprintf("%.2f", v)
  }
  BEGIN {
    distw = length("Distribution")
    file_order[1] = "mac.out"
    file_order[2] = "spark.out"
    file_order[3] = "xeon.out"
    file_order[4] = "i5.out"
    file_order[5] = "rp-python.out"
    file_order[6] = "rp-r.out"
    file_order[7] = "rp-julia.out"
    file_order[8] = "cpp.out"
    file_order[9] = "numpy.out"
    file_order[10] = "r.out"
    file_order[11] = "julia.out"
    col_head[1] = "Mac-M4"
    col_head[2] = "Spark"
    col_head[3] = "Xeon"
    col_head[4] = "Core-i5"
    col_head[5] = "Python"
    col_head[6] = "R"
    col_head[7] = "Julia"
    col_head[8] = "C++"
    col_head[9] = "NumPy"
    col_head[10] = "Base-R"
    col_head[11] = "Julia"

    add_key("u01")
    add_key("unif(2,5)")
    add_key("norm")
    add_key("normal(2,3)")
    add_key("exp(1)")
    add_key("exp(2)")
    add_key("lognormal(0,1)")
    add_key("gumbel(0,1)")
    add_key("pareto(1,2)")
    add_key("gamma(2,3)")
    add_key("gamma(0.5,2)")
    add_key("beta(2,5)")
    add_key("chi2(5)")
    add_key("t(10)")
    add_key("F(5,10)")
    add_key("weibull(2,3)")
  }
  FNR == 1 {
    file = FILENAME
    sub(/^.*\//, "", file)
    file_idx = 0
    for (i = 1; i <= 11; i++)
      if (file_order[i] == file) file_idx = i
    if ($1 == "Distribution")
      next
  }
  file_idx > 0 && NF >= 2 && $1 != "Distribution" {
    key = canonical($1)
    val[key, file_idx] = $2 + 0
    add_key(key)
  }
  END {
    printf "%-*s  %63s  %36s\n", distw, "",
      "----------------------- Randompack -----------------------",
      "-------- Other libraries --------"
    printf "%-*s  %36s  %27s  %36s\n", distw, "",
      "---------- C version --------",
      "---- Interfaces (on Core-i5) ----",
      "------ (on Core-i5) ------"
    printf "%-*s", distw, "Distribution"
    for (i = 1; i <= 11; i++)
      printf " %8s", col_head[i]
    printf "\n"
    for (i = 1; i <= nkeys; i++) {
      key = key_order[i]
      printf "%-*s", distw, display_name(key)
      for (j = 1; j <= 11; j++)
        printf " %8s", fmt(val[key, j])
      printf "\n"
    }
  }
' \
"$tmpdir/mac.out" \
"$tmpdir/spark.out" \
"$tmpdir/xeon.out" \
"$tmpdir/i5.out" \
"$tmpdir/rp-python.out" \
"$tmpdir/rp-r.out" \
"$tmpdir/rp-julia.out" \
"$tmpdir/cpp.out" \
"$tmpdir/numpy.out" \
"$tmpdir/r.out" \
"$tmpdir/julia.out"
