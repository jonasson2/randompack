// -*- C -*-
// TestMvn.c — unit tests for randompack_mvn and multivariate normal logic

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#include "xCheck.h"
#include "randompack.h"
#include "randompack_config.h"
#include "MatrixTestUtil.h"
#include "BlasGateway.h"
#include "TestUtil.h"    // create_seeded_rng, check_rng_clean, check_success, check_failure,
                         // mean/var helpers, almostEqual/almostZero, etc.
#include "printX.h"

extern int TESTVERBOSITY;

static void msg(char *message) {
  if (TESTVERBOSITY < 2) {
    (void)message;
    return;
  }
  printMsg("");
  printMsgUpper(message);
}

static void test_range(int m, int n, double A[], int k, double Y[]) {
  // True iff every column of the k × m Y is in the range of the m × n A
  double *B, *X, *R;
  int info;

  if (TESTVERBOSITY >= 4) {
    print3I("m, n, k", m, n, k);
    printM("A", A, m, n);
    printM("Y", Y, k, m);
  }

  TEST_ALLOC(B, n*n);
  TEST_ALLOC(X, n*k);
  TEST_ALLOC(R, m*k);

  syrk("Lower", "T", n, m, 1.0, A, m, 1.0, B, n);        // B := A'A
  gemm("T", "T", n, k, m, 1.0, A, m, Y, k, 0.0, X, n);   // X := A'Y'
  posv("Lower", n, k, B, n, X, n, &info);                // Solve (A'A)X = A'Y
  xCheck(info == 0);
  gemm("N", "N", m, k, n, 1.0, A, m, X, n, 0.0, R, m);   // R := AX

  if (TESTVERBOSITY >= 4) {
    printM("R", R, m, k);
    printM("Y", Y, k, m);
  }

  subtracttranspose(m, k, Y, k, R, m);
  xCheck(almostZero(R, m*k));

  FREE(R);
  FREE(X);
  FREE(B);
}

static void stdevs(double Sig[], double stds[], int n, int N) {
  for (int k=0, kk=0; k<n; k++, kk+=(n+1)) {
    stds[k] = sqrt(Sig[kk]/N);
  }
}

static bool ok8sig(double x[], double sig[], int n) {
  // return true iff |x[k]| ≤ 8·sig for all k
  for (int k=0; k<n; k++) {
    if (fabs(x[k]) > 8*sig[k]) return false;
  }
  return true;
}

static void check_cov(char *transp, int N, double Sig[], double X[],
                      randompack_rng *rng) {
  double C[16], Sms[4], Smsstd[4], Cmc[6], Cmcstd[6];
  int k, kk, ii, ij, jj;
  int ldX = (transp[0] == 'N') ? N : 4;

  bool ok = randompack_mvn(transp, 0, Sig, 4, N, X, ldX, 0, rng);
  check_success(ok, rng);

  if (transp[0] == 'N')
    cov("N", N, 4, X, C);
  else
    cov("T", 4, N, X, C);

  for (k=0; k<4; k++) {
    kk = k*5;
    Sms[k] = fabs(C[kk] - Sig[kk]);
    Smsstd[k] = Sig[kk]*sqrt(2.0/(N-1));
  }
  xCheck(ok8sig(Sms, Smsstd, 4));

  k = 0;
  for (int i=0; i<4; i++) {
    for (int j=0; j<i; j++) {
      ii = i + 4*i;
      ij = i + 4*j;
      jj = j + 4*j;
      Cmc[k] = fabs(C[ij] - Sig[ij]);
      Cmcstd[k] = sqrt((Sig[ii]*Sig[jj] + Sig[ij]*Sig[ij])/(N-1));
      k++;
    }
  }
  if (TESTVERBOSITY >= 3) {
    printV("Cmc", Cmc, 6);
    printV("Cmcstd", Cmcstd, 6);
  }
  xCheck(ok8sig(Cmc, Cmcstd, 6));
}

static void test_randnm(double Sig[], int rank) {
  if (rank == 4) msg("Positive definite sigma");
  if (rank < 4)  msg("Positive semidefinite sigma");

  int N = 10000, N1 = 10, N2 = 5;
  double mu[4] = {5, 10, 15, 20};
  double *X, *X1, *X2, *X3, LSig[16], S[16];
  double means[4], meanstd_N[4];
  bool ok;

  TEST_ALLOC(X,  N*4);
  TEST_ALLOC(X1, N1*4);
  TEST_ALLOC(X2, N1*4);
  TEST_ALLOC(X3, N1*4);

  if (TESTVERBOSITY >= 3) printM("Sig", Sig, 4, 4);

  stdevs(Sig, meanstd_N, 4, N); // Stddev of means

  msg("Check that singular Sig gives singular LSig");
  randompack_rng *rng = create_seeded_rng("x256++", 9);
  check_rng_clean(rng);
  ok = randompack_mvn("N", mu, Sig, 4, N1, X1, N1, LSig, rng);
  check_success(ok, rng);
  if (rank == 4) xCheck(LSig[15] > 0.0);
  else          xCheck(fabs(LSig[15]) < 1e-12);
  randompack_free(rng);

  msg("Check LSig·LSig' = Sig");
  laset("Upper", 4, 4, 0.0, 0.0, S, 4);
  syrk("Lower", "N", 4, 4, 1.0, LSig, 4, 0.0, S, 4);   // S := LSig*LSig'
  if (TESTVERBOSITY >= 3) printM("S", S, 4, 4);
  xCheck(almostEqual(Sig, S, 16));

  msg("Reuse LSig (Sig=0)");
  rng = create_seeded_rng("x256++", 9);
  check_rng_clean(rng);
  ok = randompack_mvn("N", mu, 0, 4, N1, X2, N1, LSig, rng);
  check_success(ok, rng);
  xCheck(almostEqual(X1, X2, N1*4));
  randompack_free(rng);

  msg("Check setting seed to same value");
  rng = create_seeded_rng("x256++", 9);
  check_rng_clean(rng);
  ok = randompack_mvn("N", mu, Sig, 4, N1, X3, N1, 0, rng);
  check_success(ok, rng);
  xCheck(almostEqual(X2, X3, N1*4));
  randompack_free(rng);

  msg("Check that X is in the range of LSig");
  rng = create_seeded_rng("x256++", 0);
  check_rng_clean(rng);
  ok = randompack_mvn("N", 0, Sig, 4, N2, X, N2, LSig, rng);
  check_success(ok, rng);
  test_range(4, rank, LSig, N2, X);
  randompack_free(rng);

  msg("Check correct means with specified mu");
  rng = create_seeded_rng("x256++", 9);
  check_rng_clean(rng);
  ok = randompack_mvn("N", mu, 0, 4, N, X, N, LSig, rng);
  check_success(ok, rng);
  meanmat("N", N, 4, X, N, means);
  axpy(4, -1.0, mu, 1, means, 1);
  xCheck(ok8sig(means, meanstd_N, 4));
  randompack_free(rng);

  msg("Check correct means with specified mu (transp='T')");
  rng = create_seeded_rng("x256++", 9);
  check_rng_clean(rng);
  ok = randompack_mvn("T", mu, 0, 4, N, X, 4, LSig, rng);
  check_success(ok, rng);
  meanmat("T", 4, N, X, 4, means);
  axpy(4, -1.0, mu, 1, means, 1);
  xCheck(ok8sig(means, meanstd_N, 4));
  randompack_free(rng);

  msg("Check correct means with mu=0");
  rng = create_seeded_rng("x256++", 0);
  check_rng_clean(rng);
  ok = randompack_mvn("N", 0, Sig, 4, N, X, N, LSig, rng);
  check_success(ok, rng);
  meanmat("N", N, 4, X, N, means);
  xCheck(ok8sig(means, meanstd_N, 4));

  msg("Sample covariance");
  check_cov("N", N, Sig, X, rng);
  check_cov("T", N, Sig, X, rng);
  randompack_free(rng);

  FREE(X);
  FREE(X1);
  FREE(X2);
  FREE(X3);
}

static void test_multivariate_normal(void) {
  double Sig[16];
  double Lnonsing[16] = {
    1, 2, 3, 2,
    0, 2, 1, 1,
    0, 0, 2, 0,
    0, 0, 0, 1
  };
  double Lsing[16] = {
    1, 2, 3, 2,
    0, 2, 1, 1,
    0, 0, 0, 0,
    0, 0, 0, 0
  };

  laset("Upper", 4, 4, 0.0, 0.0, Sig, 4);
  syrk("Lower", "N", 4, 4, 1.0, Lnonsing, 4, 0.0, Sig, 4);
  test_randnm(Sig, 4);

  laset("Upper", 4, 4, 0.0, 0.0, Sig, 4);
  syrk("Lower", "N", 4, 4, 1.0, Lsing, 4, 0.0, Sig, 4);
  test_randnm(Sig, 2);
}

// Check that ldX is honoured by randompack_mvn for both transp='N' and 'T'
// by drawing the same d×n matrix into two layouts with different leading dimensions.
static void test_mvn_ldx(void) {
  int d = 3, n = 2;
  double Sig[9] = {1,0,0, 0,1,0, 0,0,1};
  double X1[6];
  double Xbig[10];
  double X2[6];

  for (int t=0; t<2; t++) {
    char transp[2] = { t ? 'T' : 'N', 0 };
    int row = t ? d : n;
    int col = t ? n : d;
    int ld_big = row + 2; // padded ldX

    for (int i=0; i<10; i++) Xbig[i] = -999.0;

    randompack_rng *r1 = create_seeded_rng("x256++", 123);
    randompack_rng *r2 = create_seeded_rng("x256++", 123);
    check_rng_clean(r1);
    check_rng_clean(r2);

    bool ok = randompack_mvn(transp, 0, Sig, d, n, X1, row, 0, r1);
    check_success(ok, r1);
    ok = randompack_mvn(transp, 0, Sig, d, n, Xbig, ld_big, 0, r2);
    check_success(ok, r2);

    // Extract leading block from Xbig into contiguous X2 (ld = row)
    for (int j=0; j<col; j++)
      for (int i=0; i<row; i++)
        X2[i + j*row] = Xbig[i + j*ld_big];

    xCheck(almostEqual(X1, X2, row*col));

    randompack_free(r1);
    randompack_free(r2);
  }
}

static void test_mvn_bad_args(void) {
  randompack_rng *rng = create_seeded_rng("x256++", 1);
  check_rng_clean(rng);

  int d = 3, n = 2;
  double Sig[9] = {1,0,0, 0,1,0, 0,0,1};
  double mu[3] = {1, 2, 3};
  double X[6];
  double L[9];

  bool ok;

  // Neither Sig nor L provided
  ok = randompack_mvn("N", mu, 0, d, n, X, d, 0, rng);
  check_failure(ok, rng);

  // transp not starting with 'N' or 'T'
  ok = randompack_mvn("Q", mu, Sig, d, n, X, d, 0, rng);
  check_failure(ok, rng);

  // transp is an empty string
  ok = randompack_mvn("", mu, Sig, d, n, X, d, 0, rng);
  check_failure(ok, rng);

  // transp is a null pointer
  ok = randompack_mvn(0, mu, Sig, d, n, X, d, 0, rng);
  check_failure(ok, rng);

  // Non-positive dimension d
  ok = randompack_mvn("N", mu, Sig, 0, n, X, d, 0, rng);
  check_failure(ok, rng);

  // Negative sample count n
  ok = randompack_mvn("N", mu, Sig, d, -1, X, d, 0, rng);
  check_failure(ok, rng);

  // X is null but n > 0
  ok = randompack_mvn("N", mu, Sig, d, n, 0, d, 0, rng);
  check_failure(ok, rng);

  // ldX <= 0 when X is non-null
  ok = randompack_mvn("N", mu, Sig, d, n, X, 0, 0, rng);
  check_failure(ok, rng);

  // ldX too small: must be rejected (prevents out-of-bounds writes)
  ok = randompack_mvn("N", mu, Sig, d, n, X, n - 1, 0, rng);
  check_failure(ok, rng);
  ok = randompack_mvn("T", mu, Sig, d, n, X, d - 1, 0, rng);
  check_failure(ok, rng);

  // Sanity check: valid calls must still succeed after failures
  ok = randompack_mvn("N", mu, Sig, d, n, X, d, L, rng);
  check_success(ok, rng);
  ok = randompack_mvn("T", mu, Sig, d, n, X, d, 0, rng);
  check_success(ok, rng);

  randompack_free(rng);
}

void TestMvn(void) {
  test_mvn_bad_args();
  test_multivariate_normal();
  test_mvn_ldx();
}
