// Utilities for randompack tests (matrix helpers and norms)
#ifndef VARMAUTILITIES_H
#define VARMAUTILITIES_H

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>

#include "BlasGateway.h"
#include "randompack_config.h"

static inline void copytranspose(int m, int n, double A[], int ldA, double B[], int ldB) {
  // Set B to the transpose of the m×n matrix A. Matrices must not overlap.
  for (int j = 0; j < n; j++) {
    copy(m, A + j*ldA, 1, B + j, ldB);
  }
}

static inline double relabsdiff(double a[], double b[], int n) {
  // max(relative diff, absolute diff) between vectors a and b
  if (n == 0) return 0.0;
  double *c;
  ALLOC(c, n);
  copy(n, a, 1, c, 1);
  axpy(n, -1.0, b, 1, c, 1);
  int ia = iamax(n, a, 1);
  int ib = iamax(n, b, 1);
  int ic = iamax(n, c, 1);
  double rmx = fmax(fabs(a[ia]), fabs(b[ib]));
  rmx = fmax(1.0, rmx);
  double r = fabs(c[ic])/rmx;
  FREE(c);
  return r;
}

static inline void setzero(int n, double *x) { memset(x, 0, (size_t)n*sizeof(double)); }

static inline void subtracttranspose(int m, int n, double A[], int ldA, double B[], int ldB) {
  // B := B - A^T where A is n×m, B is m×n
  for (int i = 0; i < n; i++) {
    axpy(m, -1.0, A + i, ldA, B + ldB*i, 1);
  }
}

static inline void meanmat(char *transp, int m, int n, double A[], int ldA, double mu[]) {
  // Column (transp='N') or row (transp='T') means of matrix A
  switch (transp[0]) {
    case 'N': case 'n':
      setzero(n, mu);
      for (int i = 0; i < n; i++) {
        double *Ai = A + i*ldA;
        for (int j = 0; j < m; j++) mu[i] += Ai[j];
        mu[i] /= m;
      }
      break;
    case 'T': case 't':
      setzero(m, mu);
      for (int j = 0; j < m; j++) {
        for (int i = 0; i < n; i++) mu[j] += A[j + i*ldA];
        mu[j] /= n;
      }
      break;
    default:
      assert(0);
  }
}

static inline void copylowertoupper(int n, double A[], int ldA) {
  // Copy lower triangle to upper triangle of A
  for (int j = 0; j < n; j++) {
    for (int i = j + 1; i < n; i++) {
      A[j + i*ldA] = A[i + j*ldA];
    }
  }
}

#endif
