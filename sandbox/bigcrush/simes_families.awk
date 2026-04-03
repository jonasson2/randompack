function reset_family() {
  n = 0
  delete a
}
function flush_family(    i, sp, v) {
  if (fam == "" || n == 0) return
  asort(a)
  sp = 1
  for (i = 1; i <= n; i++) {
    v = n*a[i]/i
    if (v < sp) sp = v
  }
  if (sp > 1) sp = 1
  printf "%-12s  %-30s  %.6g\n", engine, fam, sp
}
BEGIN {
  if (engine == "") {
    print "need -v engine=NAME" > "/dev/stderr"
    exit 1
  }
  printf "%-12s  %-30s  %s\n", "Engine", "Test-family", "Simes-p"
  fam = ""
  reset_family()
}
NF == 1 && $1 !~ /^[0-9.]/ {
  flush_family()
  fam = $0
  reset_family()
  next
}
NF == 1 {
  a[++n] = $1 + 0
}
END {
  flush_family()
}
