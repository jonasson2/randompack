// Utilities for VarmaLoglik

#ifndef VARMAUTILITIES_H
#define VARMAUTILITIES_H

#include <string.h>
#include <math.h>
#include <stdbool.h>
#include "BlasGateway.h"
#include "error.h"
#include "allocate.h"

static inline void copytranspose(int m, int n, double A[], int ldA, double B[], int ldB){
  // Set B to the transpose of the m×n matrix A. Leading dimensions are in ldA
  // and ldB. Matrices must not overlap.
  int j;
  double *A1j = A, *Bj1 = B;
  for (j=0; j<n; j++) {
    copy(m, A1j, 1, Bj1, ldB);
    A1j += ldA;
    Bj1++;
  }  
}

static inline double relabsdiff(double a[], double b[], int n) {
  // max(relative diff, absolute diff) where diff is difference between
  // vectors a and b
  int ia, ib, ic;
  double *c, rmx, r;
  if (n == 0) return 0.0;
  allocate(c, n);
  copy(n, a, 1, c, 1);
  axpy(n, -1.0, b, 1, c, 1);
  ia = iamax(n, a, 1);
  ib = iamax(n, b, 1);
  ic = iamax(n, c, 1);
  rmx = fmax(fabs(a[ia]), fabs(b[ib]));
  rmx = fmax(1.0, rmx);
  r = fabs(c[ic])/rmx;
  freem(c);
  return r;
}

static inline void setzero(int n, double *x) {   // set x[0]...x[n-1] to 0.0
  memset(x, 0, n*sizeof(double));
}

static inline void subtractmat(int m, int n, double A[], int ldA, double B[], int ldB) {
  // Subtract m×n matrix A from m×n matrix B
  int i;
  double *Ai, *Bi;
  for (i=0; i<n; i++) {
    Ai = A + ldA*i;
    Bi = B + ldB*i;    
    axpy(m, -1.0, Ai, 1, Bi, 1);
  }
}

static inline void subtracttranspose(int m, int n, double A[], int ldA, double B[], int ldB) {
  // Subtract transpose of n × m matrix A from m × n matrix B
  int i;
  for (i=0; i<n; i++) {
    axpy(m, -1.0, A + i, ldA, B + ldB*i, 1);
  }
}

static inline void addmat
(char *uplo, int m, int n, double A[], int ldA, double B[], int ldB){
  // With uplo beginning with "A", add m×n matrix A to m×n matrix B.
  // If uplo begins with "U" or "L" only the upper or lower triangles are added.
  int i;
  double *Ai, *Bi;
  switch (uplo[0]) {
    case('A'): case('a'):
      if (ldA == m && ldB == m) {
        axpy(m*n, 1.0, A, 1, B, 1);
      }
      else {
        for (i=0; i<n; i++) {
          Ai = A + ldA*i;
          Bi = B + ldB*i;
          axpy(m, 1.0, Ai, 1, Bi, 1);
        }
      }
      break;
    case('L'): case('l'):
      for (i=0; i<n; i++) {
        Ai = A + i + ldA*i;
        Bi = B + i + ldB*i;
        axpy(m-i, 1.0, Ai, 1, Bi, 1);
      }
      break;
    case('U'): case('u'):      
      for (i=0; i<n; i++) {
        Ai = A + ldA*i;
        Bi = B + ldB*i;
        axpy(i+1, 1.0, Ai, 1, Bi, 1);
      };
      break;
    default: xAssert(0);
  }
}

static inline void meanmat(char *transp, int m, int n, double A[], int ldA, double mu[]) {
  // Find means of columns (if *transp begins with 'N', mu is an n-vector) or
  // rows (if it begins with 'T', mu is an m-vector) of the m by n matrix A.
  double *Ai;
  int i, j;
  switch (transp[0]) {
    case('N'): case('n'):
      setzero(n, mu);
      for (i=0; i<n; i++) {
        Ai = A + i*ldA;
        for (j=0; j<m; j++) mu[i] += Ai[j];
        mu[i] /= m;
      }
      break;
    case('T'): case('t'):
      setzero(m, mu);
      for (i=0; i<n; i++) {
        Ai = A + i*ldA;
        for (j=0; j<m; j++) mu[j] += Ai[j];
      }
      scal(m, 1.0/n, mu, 1);
      break;
  }
}

static inline void aplusat(int n, double A[], int ldA) {
  // Add A' to A and return lower triangle of result in lower triangle of A
  int i;
  for (i=1; i<n; i++) {
    axpy(n-i, 1.0, A + (i-1) + i*ldA, ldA, A + i + (i-1)*ldA, 1);
  }
  scal(n, 2.0, A, ldA+1); // Scale diagonal
}

static inline void copylowertoupper(int n, double A[], int ldA) {
  // Set strictly upper triangle of n × n matrix A to strictly lower triangle of 
  // its transpose.
  int i;
  for (i=1; i<n; i++) {
    copy(n-i, A + i + (i-1)*ldA, 1, A + (i-1) + i*ldA, ldA);
  }
}

static inline void setEye(int n, double A[], int lda) {
  // Set matrix to identity
  laset("All", n, n, 0.0, 1.0, A, lda);
}

static inline void flipmat(double A[], double Aflp[], int r, int k) {
  // [A1 ... Ak] –> [Ak ... A1]
  for (int i = 0; i < k; i++) {
    lacpy("All", r, r, A + i*r*r, r, Aflp + (k - 1 - i)*r*r, r);
  }
}

static inline bool anynan(int n, double A[]) { // return true if A[i] != A[i] for any i
  int i;
  for (i=0; i<n; i++) if (A[i] != A[i]) return true;
  return false;
}

static inline bool islowermat(int n, double A[]) { // true iff A is lower triangular
  int i, j;
  for (i=0; i<n; i++)
    for (j=i+1; j<n; j++)
      if (A[i + j*n] != 0) return false;
  return true;
}

static inline int imin(int m, int n) {
  return m < n ? m : n;
}

static inline int imax(int m, int n) {
  return m > n ? m : n;
}

#endif
