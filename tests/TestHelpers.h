// -*- C -*-

#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include <stdbool.h>

int iminv(int *x, int n);
int imaxv(int *x, int n);

static inline bool equal_intv_offset(int *a, int *b, int n,
                                     int offset) {
  for (int i = 0; i < n; i++) {
    if (b[i] != a[i] + offset) return false;
  }
  return true;
}

static inline bool is_perm_0_to_n_minus1(int *x, int n) {
  int seen[n];
  for (int i = 0; i < n; i++) seen[i] = 0;
  for (int i = 0; i < n; i++) {
    int v = x[i];
    if (v < 0 || v >= n) return false;
    if (seen[v]) return false;
    seen[v] = 1;
  }
  return true;
}

#endif
