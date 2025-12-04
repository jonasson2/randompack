// -*- C -*-

#ifndef TEST_RANDOM_HELPERS_H
#define TEST_RANDOM_HELPERS_H

#include <stdbool.h>

static inline int min_intv(const int *x, int n) {
  int m = x[0];
  for (int i = 1; i < n; i++) {
    if (x[i] < m) m = x[i];
  }
  return m;
}

static inline int max_intv(const int *x, int n) {
  int m = x[0];
  for (int i = 1; i < n; i++) {
    if (x[i] > m) m = x[i];
  }
  return m;
}

static inline bool equal_intv_offset(const int *a, const int *b, int n,
                                     int offset) {
  for (int i = 0; i < n; i++) {
    if (b[i] != a[i] + offset) return false;
  }
  return true;
}

static inline bool is_perm_0_to_n_minus1(const int *x, int n) {
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
