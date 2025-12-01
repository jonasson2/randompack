// TestRandomMvn.c — unit tests for randompack_mvn and multivariate normal logic

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include "xCheck.h"

#include "randompack.h"
#include "VarmaUtilities.h"
#include "BlasGateway.h"
#include "ExtraUtil.h"    // for mean, var, relabsdiff, almostSame, almostEqual
#include "allocate.h"
#include "printX.h"

extern int TESTVERBOSITY;

static void msg(char *message) {
  if (TESTVERBOSITY >= 2) {
    printMsg("");
    printMsgUpper(message);
  }
}

static void test_range(int m, int n, double A[], int k, double Y[]) {
  // True iff every column of the k × m Y is in the range of the m × n A
  double *B, *X, *R;
  int info;
  print3I("m, n, k", m, n, k);
  printM("A", A, m, n);
  printM("Y", Y, k, m);
  allocate(B, n*n);
  allocate(X, n*k);
  allocate(R, m*k);
  syrk("Lower", "T", n, m, 1.0, A, m, 1.0, B, n);       // B := A'A
  gemm("T", "T", n, k, m, 1.0, A, m, Y, k, 0.0, X, n);  // X := A'Y'
  posv("Lower", n, k, B, n, X, n, &info);               // Solve BX = A'Y
  xCheck(info == 0);
  gemm("N", "N", m, k, n, 1.0, A, m, X, n, 0.0, R, m); // R := AX
  printM("R", R, m, k);
  printM("Y", Y, k, m);
  subtracttranspose(m, k, Y, k, R, m);
  xCheck(almostZero(R, m*k));
  freem(R);
  freem(X);
  freem(B);
}

static void stdevs(double Sig[], double stds[], int n, int N) {
  for (int k=0, kk=0; k<n; k++, kk+=(n+1)) {
    stds[k] = sqrt(Sig[kk]/N);
  }
}

static bool ok7sig(double x[], double sig[], int n) {
  // return true iff |x[k]| ≤ 7·sig for all k
  for (int k=0; k<n; k++) {
    if (fabs(x[k]) > 7*sig[k]) return false;
  }
  return true;
}

static void check_cov(char *transp, int N, double Sig[], double X[], randompack_rng *rng) {
  double C[16], Sms[4], Smsstd[4], Cmc[4], Cmcstd[4];
  int k, kk, ii, ij, jj;
  int ldX = (transp[0] == 'N') ? N : 4;
  randompack_mvn(transp, 0, Sig, 4, N, X, ldX, 0, rng);
  if (transp[0] == 'N') {
    cov("N", N, 4, X, C);
  }
  else {
    cov("T", 4, N, X, C);
  }
  for (k=0; k<4; k++) {
    kk = k*5;
    Sms[k] = fabs(C[kk] - Sig[kk]);
    Smsstd[k] = Sig[kk]*sqrt(2.0/(N-1));
  }
  xCheck(ok7sig(Sms, Smsstd, 4));
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
  printI("k", k);
  printV("Cmc", Cmc, 6);
  printV("Cmcstd", Cmcstd, 6);
  xCheck(ok7sig(Cmc, Cmcstd, 6));
}

static void test_randnm(double Sig[], int rank) {
  // Prepare:
  if (rank == 4) msg("Positive definite sigma");
  if (rank < 4)  msg("Positive semidefinite sigma");
  const int N = 10000, N1 = 10, N2 = 5;
  double mu[4] = {5, 10, 15, 20};
  double *X, *X1, *X2, *X3, LSig[16], S[16];
  double means[4], meanstd_N[4];
  allocate(X, N*4);
  allocate(X1, N1*4);
  allocate(X2, N1*4);
  allocate(X3, N1*4);
  printM("Sig", Sig, 4, 4);
  randompack_rng *rng;

  stdevs(Sig, meanstd_N, 4, N); // Stddev of means:

  msg("Check that singular Sig gives singular LSig");
  rng = randompack_create("Xorshift", 9);
  randompack_mvn("NoT", mu, Sig, 4, N1, X1, N1, LSig, rng);
  xCheck(rank == 4 ? LSig[15] > 0 : LSig[15] == 0);
  randompack_free(rng);

  msg("Check LSig·LSig' = Sig:");
  laset("Upper", 4, 4, 0.0, 0.0, S, 4);
  syrk("Lower", "Not", 4, 4, 1.0, LSig, 4, 0.0, S, 4.0);
  printM("S", S, 4, 4);
  xCheck(almostEqual(Sig, S, 16));
  msg("LSig 0:");

  msg("Reuse LSig (Sig=0):");
  rng = randompack_create("Xorshift", 9);
  randompack_mvn("NoT", mu, 0, 4, N1, X2, N1, LSig, rng);
  xCheck(almostEqual(X1, X2, N1*4));
  randompack_free(rng);

  msg("Check setting seed to same value");
  rng = randompack_create("Xorshift", 9);
  randompack_mvn("NoT", mu, Sig, 4, N1, X3, N1, 0, rng);
  xCheck(almostEqual(X2, X3, N1*4));
  randompack_free(rng);

  msg("-Check that X is in the range of LSig:");
  rng = randompack_create("Xorshift", 0);
  randompack_mvn("NoT", 0, Sig, 4, N2, X, N2, LSig, rng);
  test_range(4, rank, LSig, N2, X);
  randompack_free(rng);

  msg("Check correct means with specified mu:");
  rng = randompack_create("Xorshift", 9);
  randompack_mvn("NoT", mu, 0, 4, N, X, N, LSig, rng);
  meanmat("N", N, 4, X, N, means);
  printV("means", means, 4);
  axpy(4, -1.0, mu, 1, means, 1);
  printV("mu", mu, 4);
  printV("meanstd", meanstd_N, 4);
  xCheck(ok7sig(means, meanstd_N, 4));
  randompack_free(rng);

  rng = randompack_create("Xorshift", 9);
  randompack_mvn("T", mu, 0, 4, N, X, 4, LSig, rng); // also for X^T
  meanmat("T", 4, N, X, 4, means);
  printV("means", means, 4);
  axpy(4, -1.0, mu, 1, means, 1);
  xCheck(ok7sig(means, meanstd_N, 4));
  randompack_free(rng);

  msg("– and with mu=0:");
  rng = randompack_create("Xorshift", 0);
  randompack_mvn("NoT", 0, Sig, 4, N, X, N, LSig, rng);
  meanmat("N", N, 4, X, N, means);
  printV("means", means, 4);
  xCheck(ok7sig(means, meanstd_N, 4));

  msg("-Sample covariance");
  check_cov("NoT", N, Sig, X, rng);
  check_cov("T", N, Sig, X, rng);
  randompack_free(rng);
  freem(X);
  freem(X1);
  freem(X2);
  freem(X3);
}

static void test_multivariate_normal(void) {
  double Sig[16],
    Lnonsing[16] = {
      1, 2, 3, 2,
      0, 2, 1, 1,
      0, 0, 2, 0,
      0, 0, 0, 1},
    Lsing[16] = {
      1, 2, 3, 2,
      0, 2, 1, 1,
      0, 0, 0, 0,
      0, 0, 0, 0};
  xCheckAddMsg("Test positive definite Sigma");
  laset("Upper", 4, 4, 0.0, 0.0, Sig, 4);
  syrk("Lower", "NoT", 4, 4, 1.0, Lnonsing, 4, 0.0, Sig, 4);
  test_randnm(Sig, 4);

  xCheckAddMsg("Test positive semidefinite Sigma");
  laset("Upper", 4, 4, 0.0, 0.0, Sig, 4);
  syrk("Lower", "NoT", 4, 4, 1.0, Lsing, 4, 0.0, Sig, 4);
  test_randnm(Sig, 2);
}

// Check that ldX is honoured by randompack_mvn by drawing the same
// 3×2 matrix into two layouts with different leading dimensions.
static void test_mvn_ldx(void) {
  const int d = 3;
  const int n = 2;
  double Sig[9] = {
    1.0, 0.0, 0.0,
    0.0, 1.0, 0.0,
    0.0, 0.0, 1.0
  }; // 3×3 identity
  double X1[3*2];
  double Xbig[5*2];
  double X2[3*2];

  // Fill Xbig with sentinel values to ensure we only compare the filled block
  for (int i = 0; i < 10; i++) Xbig[i] = -999.0;

  randompack_rng *r1 = randompack_create("Xorshift", 123);
  randompack_rng *r2 = randompack_create("Xorshift", 123);

  bool ok1 = randompack_mvn("T", 0, Sig, d, n, X1, d, 0, r1); // 3×2, ldX = 3
  bool ok2 = randompack_mvn("T", 0, Sig, d, n, Xbig, 5, 0, r2); // 3×2 in 5×2, ldX = 5

  xCheck(ok1);
  xCheck(ok2);

  // Extract the leading 3×2 block from Xbig into X2 using contiguous layout
  for (int j = 0; j < n; j++) {
    for (int i = 0; i < d; i++) {
      X2[i + j*d] = Xbig[i + j*5];
    }
  }

  xCheck(almostEqual(X1, X2, d*n));

  randompack_free(r1);
  randompack_free(r2);
}

#define RUN_TEST(x)                             \
 do {                                           \
   xCheckAddMsg(#x);                            \
   test_##x();                                  \
 } while(0)

void TestRandomMvn(void) {
  RUN_TEST(multivariate_normal);
  RUN_TEST(mvn_ldx);
}
