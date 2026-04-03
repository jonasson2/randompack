function reset_engine() {
  n = 0
  delete a
}
function flush_engine(    i, sp, v) {
  if (eng == "" || n == 0) return
  asort(a)
  sp = 1
  for (i = 1; i <= n; i++) {
    v = n*a[i]/i
    if (v < sp) sp = v
  }
  if (sp > 1) sp = 1
  printf "%-12s  %.3f\n", eng, sp
}
BEGIN {
  printf "%-12s  %s\n", "Engine", "Simes-p"
  eng = ""
  reset_engine()
}
$1 == "Engine" {
  next
}
NF >= 3 {
  if (eng != "" && $1 != eng) {
    flush_engine()
    reset_engine()
  }
  eng = $1
  a[++n] = $NF + 0
}
END {
  flush_engine()
}
