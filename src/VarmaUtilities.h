// Utilities for randompack tests (matrix helpers and norms)
#ifndef VARMAUTILITIES_H
#define VARMAUTILITIES_H

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>

#include "BlasGateway.h"
#include "allocate.h"

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
  allocate(c, n);
  copy(n, a, 1, c, 1);
  axpy(n, -1.0, b, 1, c, 1);
  int ia = iamax(n, a, 1);
  int ib = iamax(n, b, 1);
  int ic = iamax(n, c, 1);
  double rmx = fmax(fabs(a[ia]), fabs(b[ib]));
  rmx = fmax(1.0, rmx);
  double r = fabs(c[ic])/rmx;
  freem(c);
  return r;
}

static inline void setzero(int n, double *x) { memset(x, 0, (size_t)n*sizeof(double)); }

static inline void subtractmat(int m, int n, double A[], int ldA, double B[], int ldB) {
  // B := B - A for m×n matrices
  for (int i = 0; i < n; i++) {
    axpy(m, -1.0, A + ldA*i, 1, B + ldB*i, 1);
  }
}

static inline void subtracttranspose(int m, int n, double A[], int ldA, double B[], int ldB) {
  // B := B - A^T where A is n×m, B is m×n
  for (int i = 0; i < n; i++) {
    axpy(m, -1.0, A + i, ldA, B + ldB*i, 1);
  }
}

static inline void addmat(char *uplo, int m, int n, double A[], int ldA, double B[], int ldB) {
  // B := B + A with optional triangular restriction
  switch (uplo[0]) {
    case 'A': case 'a':
      if (ldA == m && ldB == m) axpy(m*n, 1.0, A, 1, B, 1);
      else {
        for (int i = 0; i < n; i++) axpy(m, 1.0, A + ldA*i, 1, B + ldB*i, 1);
      }
      break;
    case 'L': case 'l':
      for (int i = 0; i < n; i++) axpy(m - i, 1.0, A + i + ldA*i, 1, B + i + ldB*i, 1);
      break;
    case 'U': case 'u':
      for (int i = 0; i < n; i++) axpy(i + 1, 1.0, A + ldA*i, 1, B + ldB*i, 1);
      break;
    default:
      assert(0);
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
      for (int i = 0; i < n; i++) {
        double *Ai = A + i;
        for (int j = 0; j < m; j++) mu[j] += Ai[j*ldA];
      }
      for (int j = 0; j < m; j++) mu[j] /= n;
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

static inline void flipmat(double A[], double Aflp[], int r, int k) {
  // [A1 ... Ak] –> [Ak ... A1]
  for (int i = 0; i < k; i++) {
    lacpy("All", r, r, A + i*r*r, r, Aflp + (k - 1 - i)*r*r, r);
  }
}

static inline bool anynan(int n, double A[]) {
  for (int i = 0; i < n; i++) if (A[i] != A[i]) return true;
  return false;
}

static inline bool islowermat(int n, double A[]) {
  for (int i = 0; i < n; i++)
    for (int j = i + 1; j < n; j++)
      if (A[i + j*n] != 0) return false;
  return true;
}

static inline int imin(int m, int n) { return m < n ? m : n; }
static inline int imax(int m, int n) { return m > n ? m : n; }

#endif
