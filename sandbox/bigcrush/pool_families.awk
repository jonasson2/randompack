/ test:$/ {
  fam = $0
  sub(/^s/, "", fam)
  sub(/ test:$/, "", fam)
  if (!(fam in seen)) {
    order[++nf] = fam
    seen[fam] = 1
    sum[fam] = 0
  }
  next
}
/p-value of test/ {
  split($0, a, ":")
  s = a[2]
  sub(/^[ \t]+/, "", s)
  if (s ~ /^1[ \t]*-/) {
    sub(/^1[ \t]*-[ \t]*/, "", s)
    split(s, b)
    pval = b[1] + 0
  }
  else {
    split(s, b)
    pval = b[1] + 0
    if (pval > 0.5) pval = 1-pval
    pval = 2*pval
  }
  p[fam] = p[fam] pval "\n"
}
END {
  for (i = 1; i <= nf; i++) {
    fam = order[i]
    print fam
    printf "%s", p[fam]
  }
}
