BEGINFILE {
  complete = 0
  fam = ""
  nout = 0
  delete out
  name = FILENAME
  sub(/^.*\//, "", name)
  cur_engine = name
  sub(/_s[0-9]+\.txt$/, "", cur_engine)
  cur_seed = name
  sub(/^.*_s/, "", cur_seed)
  sub(/\.txt$/, "", cur_seed)
}

/========= Summary results of BigCrush/ {
  complete = 1
  next
}

/^[^R].*test:$/ {
  fam = $0
  sub(/^s/, "", fam)
  sub(/ test:$/, "", fam)
  next
}

/p-value of test/ {
  if (fam == "") next
  split($0, a, ":")
  s = a[2]
  sub(/^[ 	]+/, "", s)
  if (s ~ /^1[ 	]*-/) {
    sub(/^1[ 	]*-[ 	]*/, "", s)
    split(s, b)
    pval = 1 - (b[1] + 0)
  }
  else {
    split(s, b)
    pval = b[1] + 0
  }
  qval = pval
  if (1 - pval < qval) qval = 1 - pval
  nout++
  out[nout] = cur_seed " " cur_engine " " fam " " qval
}

ENDFILE {
  if (!complete) exit
  for (i = 1; i <= nout; i++) print out[i]
}
