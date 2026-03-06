// TestDpstrf.c — unit tests for rp_dpstrf
// Test(m, n):
// create an m by n B matrix with U(0,1) entries
// use gemm to compute m by m A = B*B'
// let pstrf compute L and piv so that A(piv, piv) = L*L'
// use the helper to get M1 = A(piv, piv) and gemm to get M2 = L*L'
// compare M1 to M2 (my guess is that 10*m*machine-epsilon is fine)
// Then call this test 5 times with each B-dimension:
// (2,1) (3,1) (3,2) (10,5) (10,9) (20,19) (40,39) (200,199) (300,100)
#include <float.h>
#include <stdbool.h>
#include <stdio.h>

#include "randompack.h"
#include "test_util.h"
#include "test_matrix_util.h"
#include "xCheck.h"

void rp_dpstrf(char *uplo, int n, double *a, int lda, int *piv, int *rank,
               double tol, double *work, int *info);

static void perm(double *B, double *A, int n, int *piv) {
  for (int j = 0; j < n; j++) {
    int pj = piv[j] - 1;
    for (int i = 0; i < n; i++) {
      int pi = piv[i] - 1;
      B[i + j*n] = A[pi + pj*n];
    }
  }
}

static void extract_lower(double *L, double *A, int n) {
  for (int i = 0; i < n*n; i++) L[i] = 0;
  for (int j = 0; j < n; j++) {
    for (int i = j; i < n; i++) {
      L[i + j*n] = A[i + j*n];
    }
  }
}

static void test_case(int m, int n, int seed) {
  size_t bn = (size_t)m*n;
  size_t an = (size_t)m*m;
  double *B;
  double *A;
  double *A0;
  double *L;
  double *M1;
  double *M2;
  double *work;
  int *piv;
  int rank = 0;
  int info = 0;
  char msg[80];
  randompack_rng *rng = create_seeded_rng("x256++simd", seed);
  TEST_ALLOC(B, bn);
  TEST_ALLOC(A, an);
  TEST_ALLOC(A0, an);
  TEST_ALLOC(L, an);
  TEST_ALLOC(M1, an);
  TEST_ALLOC(M2, an);
  TEST_ALLOC(work, 2*m);
  TEST_ALLOC(piv, m);
  bool ok = randompack_unif(B, bn, 0, 1, rng);
  check_success(ok, rng);
  gemm("N", "T", m, m, n, 1, B, m, B, m, 0, A0, m);
  copylowertoupper(m, A0, m);
  copy((int)an, A0, 1, A, 1);
  rp_dpstrf("L", m, A, m, piv, &rank, 0, work, &info);
  xCheckMsg(info >= 0, "rp_dpstrf info");
  int k = rank;
  if (k < 0) k = 0;
  if (k > m) k = m;
  extract_lower(L, A, m);
  gemm("N", "T", m, m, k, 1, L, m, L, m, 0, M2, m);
  if (k > 0) {
    perm(M1, A0, m, piv);
    double err = relabsdiff(M1, M2, (int)an);
    double tol = 10.0*m*DBL_EPSILON;
    snprintf(msg, sizeof(msg), "rp_dpstrf m=%d n=%d err=%.3e tol=%.3e", m, n, err, tol);
    xCheckMsg(err <= tol, msg);
  }
  FREE(piv);
  FREE(work);
  FREE(M2);
  FREE(M1);
  FREE(L);
  FREE(A0);
  FREE(A);
  FREE(B);
  randompack_free(rng);
}

void TestDpstrf(void) {
  int dims[][2] = {{2,1}, {3,1}, {3,2}, {10,5}, {10,9}, {20,19},
                   {40,39}, {200,199}, {300,100}};
  int ndims = LEN(dims);
  int seed = 123;
  for (int t = 0; t < 5; t++) {
    for (int k = 0; k < ndims; k++) {
      int m = dims[k][0];
      int n = dims[k][1];
      test_case(m, n, seed++);
    }
  }
}
