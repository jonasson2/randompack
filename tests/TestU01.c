// -*- C -*-
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

#include "allocate.h"
#include "ExtraUtil.h"
#include "printX.h"
#include "randompack.h"
#include "xCheck.h"

static void test_uniform_basic(void) {
  int N = 1e6;
  double meantol = 7*1/sqrt(12*N);
  double vartol = 7*1/sqrt(180*N);
  double exactmu = 0.5, exactvar = 1.0/12.0;
  double *x = 0;
  allocate(x, N);

  randompack_rng *rng = randompack_create("Xorshift", 0);
  randompack_u01(x, N, rng);

  double xmin = x[0], xmax = x[0];
  for (int i = 1; i < N; i++) {
    if (x[i] < xmin) xmin = x[i];
    if (x[i] > xmax) xmax = x[i];
  }
  xCheck(xmin >= 0.0 && xmax < 1.0);
  double mu = mean(x, N);
  double va = var(x, N, mu);
  xCheck(fabs(mu - exactmu) < meantol);
  xCheck(fabs(va - exactvar) < vartol);

  randompack_free(rng);
  freem(x);
}

void TestU01(void) {
  test_uniform_basic();
}
