// -*- C -*-
// Tests for continuous distributions via randompack_* and *_f.

#include <math.h>
#include <stdbool.h>
#include <string.h>

#include "test_util.h"
#include "test_cdfs.h"
#include "randompack.h"
#include "randompack_config.h"
#include "printX.h"
#include "xCheck.h"

extern int TESTVERBOSITY;
extern int TESTFLOAT;

typedef double (*cdf1_fun)(double x, double p0);
typedef double (*cdf2_fun)(double x, double p0, double p1);
typedef double (*cdf3_fun)(double x, double p0, double p1, double p2);
typedef double (*cdf0_fun)(double x);
typedef bool (*d0_fun)(double x[], size_t n, randompack_rng *rng);
typedef bool (*d0f_fun)(float x[], size_t n, randompack_rng *rng);
typedef bool (*d1_fun)(double x[], size_t n, double p0, randompack_rng *rng);
typedef bool (*d2_fun)(double x[], size_t n, double p0, double p1,
  randompack_rng *rng);
typedef bool (*d3_fun)(double x[], size_t n, double p0, double p1, double p2,
  randompack_rng *rng);
typedef bool (*d1f_fun)(float x[], size_t n, float p0, randompack_rng *rng);
typedef bool (*d2f_fun)(float x[], size_t n, float p0, float p1,
  randompack_rng *rng);
typedef bool (*d3f_fun)(float x[], size_t n, float p0, float p1, float p2,
  randompack_rng *rng);

typedef enum {
  SUPPORT_NONE,
  SUPPORT_NONNEG,
  SUPPORT_UNIT,
  SUPPORT_PARETO,
} support_type;

typedef struct {
  const char *name;
  int arity;
  int ncases;
  double p0[4];
  double p1[4];
  double p2[4];
  support_type support;
  union {
    d0_fun d0;
    d1_fun d1;
    d2_fun d2;
    d3_fun d3;
  } draw;
  union {
    d0f_fun d0;
    d1f_fun d1;
    d2f_fun d2;
    d3f_fun d3;
  } drawf;
  union {
    cdf0_fun c0;
    cdf1_fun c1;
    cdf2_fun c2;
    cdf3_fun c3;
  } cdf;
} dist_spec;

typedef struct {
  const char *name;
  int arity;
  double p0;
  double p1;
  double p2;
} illegal_case;

static const dist_spec dist_specs[] = {
  { "norm", 0, 1, { 0 }, { 0 }, { 0 }, SUPPORT_NONE,
    { .d0 = randompack_norm }, { .d0 = randompack_normf },
    { .c0 = test_cdf_norm } },
  { "normal", 2, 3, { 0, 1, -3 }, { 1, 2, 0.5 }, { 0 }, SUPPORT_NONE,
    { .d2 = randompack_normal }, { .d2 = randompack_normalf },
    { .c2 = test_cdf_normal } },
  { "lognormal", 2, 3, { 0, -0.7, 1.2 }, { 1, 0.4, 1.3 }, { 0 },
    SUPPORT_NONNEG,
    { .d2 = randompack_lognormal }, { .d2 = randompack_lognormalf },
    { .c2 = test_cdf_lognormal } },
  { "gamma", 2, 3, { 0.7, 2, 0.3 }, { 1, 1, 3 }, { 0 }, SUPPORT_NONNEG,
    { .d2 = randompack_gamma }, { .d2 = randompack_gammaf },
    { .c2 = gamma_cdf } },
  { "beta", 2, 3, { 0.5, 2, 0.7 }, { 0.5, 5, 3 }, { 0 }, SUPPORT_UNIT,
    { .d2 = randompack_beta }, { .d2 = randompack_betaf },
    { .c2 = test_cdf_beta } },
  { "chi2", 1, 2, { 1, 10 }, { 0 }, { 0 }, SUPPORT_NONNEG,
    { .d1 = randompack_chi2 }, { .d1 = randompack_chi2f },
    { .c1 = test_cdf_chi2 } },
  { "t", 1, 2, { 1, 30 }, { 0 }, { 0 }, SUPPORT_NONE,
    { .d1 = randompack_t }, { .d1 = randompack_tf },
    { .c1 = test_cdf_t } },
  { "gumbel", 2, 2, { 0, 2 }, { 1, 0.5 }, { 0 }, SUPPORT_NONE,
    { .d2 = randompack_gumbel }, { .d2 = randompack_gumbelf },
    { .c2 = test_cdf_gumbel } },
  { "pareto", 2, 2, { 1, 2 }, { 1.5, 0.7 }, { 0 }, SUPPORT_PARETO,
    { .d2 = randompack_pareto }, { .d2 = randompack_paretof },
    { .c2 = test_cdf_pareto } },
  { "weibull", 2, 3, { 1, 0.7, 3 }, { 1, 2, 0.5 }, { 0 }, SUPPORT_NONNEG,
    { .d2 = randompack_weibull }, { .d2 = randompack_weibullf },
    { .c2 = test_cdf_weibull } },
  { "f", 2, 2, { 1, 30 }, { 1, 40 }, { 0 }, SUPPORT_NONNEG,
    { .d2 = randompack_f }, { .d2 = randompack_ff },
    { .c2 = test_cdf_f } },
  { "skew-normal", 3, 1, { 0 }, { 1 }, { 0 }, SUPPORT_NONE,
    { .d3 = randompack_skew_normal }, { .d3 = randompack_skew_normalf },
    { .c3 = test_cdf_skew_normal } },
};

static const illegal_case illegal_cases[] = {
  { "normal", 2, 0, 0, 0 },
  { "normal", 2, 0, -1, 0 },
  { "lognormal", 2, 0, 0, 0 },
  { "lognormal", 2, 0, -1, 0 },
  { "gamma", 2, 0, 1, 0 },
  { "gamma", 2, -1, 1, 0 },
  { "gamma", 2, 0.7, 0, 0 },
  { "gamma", 2, 0.7, -1, 0 },
  { "beta", 2, 0, 1, 0 },
  { "beta", 2, -1, 1, 0 },
  { "beta", 2, 1, 0, 0 },
  { "beta", 2, 1, -1, 0 },
  { "chi2", 1, 0, 0, 0 },
  { "chi2", 1, -1, 0, 0 },
  { "t", 1, 0, 0, 0 },
  { "t", 1, -1, 0, 0 },
  { "gumbel", 2, 0, 0, 0 },
  { "gumbel", 2, 0, -1, 0 },
  { "pareto", 2, 0, 2, 0 },
  { "pareto", 2, -1, 2, 0 },
  { "pareto", 2, 1, 0, 0 },
  { "pareto", 2, 1, -1, 0 },
  { "weibull", 2, 0, 1, 0 },
  { "weibull", 2, -1, 1, 0 },
  { "weibull", 2, 1, 0, 0 },
  { "weibull", 2, 1, -1, 0 },
  { "f", 2, 0, 1, 0 },
  { "f", 2, -1, 1, 0 },
  { "f", 2, 1, 0, 0 },
  { "f", 2, 1, -1, 0 },
  { "skew-normal", 3, 0, 0, 0 },
  { "skew-normal", 3, 0, -1, 0 },
};

static const dist_spec *find_spec(const char *distname, int arity) {
  for (size_t i = 0; i < LEN(dist_specs); i++) {
    if (dist_specs[i].arity == arity && strcmp(dist_specs[i].name, distname) == 0)
      return &dist_specs[i];
  }
  return 0;
}

static void test_continuous_edge_cases(const char *engine) {
  if (TESTVERBOSITY >= 2) {
    fprintf(stderr, "TestContinuous edge cases\n");
    fflush(stderr);
  }
  double buf[4] = { 1, 2, 3, 4 };
  double orig[4] = { 1, 2, 3, 4 };
  float buf_f[4] = { 1, 2, 3, 4 };
  float orig_f[4] = { 1, 2, 3, 4 };
  bool ok;
  randompack_rng *rng = create_seeded_rng(engine, 123);
  for (size_t i = 0; i < LEN(dist_specs); i++) {
    const dist_spec *spec = &dist_specs[i];
    double p0 = spec->p0[0];
    double p1 = spec->p1[0];
    double p2 = spec->p2[0];
    float p0f = (float)p0;
    float p1f = (float)p1;
    float p2f = (float)p2;
    memcpy(buf, orig, sizeof(buf));
    memcpy(buf_f, orig_f, sizeof(buf_f));
    if (spec->arity == 0) {
      ok = spec->draw.d0(buf, 0, rng); check_success(ok, rng);
      CHECK_EQUALV_MSG(buf, orig, LEN(buf), spec->name);
      ok = spec->draw.d0(0, LEN(buf), rng); check_failure(ok, rng);
      ok = spec->draw.d0(buf, LEN(buf), 0); xCheck(!ok);
      if (TESTFLOAT) {
        ok = spec->drawf.d0(buf_f, 0, rng); check_success(ok, rng);
        CHECK_EQUALV_MSG(buf_f, orig_f, LEN(buf_f), spec->name);
        ok = spec->drawf.d0(0, LEN(buf_f), rng); check_failure(ok, rng);
        ok = spec->drawf.d0(buf_f, LEN(buf_f), 0); xCheck(!ok);
      }
    }
    else if (spec->arity == 1) {
      ok = spec->draw.d1(buf, 0, p0, rng); check_success(ok, rng);
      CHECK_EQUALV_MSG(buf, orig, LEN(buf), spec->name);
      ok = spec->draw.d1(0, LEN(buf), p0, rng); check_failure(ok, rng);
      ok = spec->draw.d1(buf, LEN(buf), p0, 0); xCheck(!ok);
      if (TESTFLOAT) {
        ok = spec->drawf.d1(buf_f, 0, p0f, rng); check_success(ok, rng);
        CHECK_EQUALV_MSG(buf_f, orig_f, LEN(buf_f), spec->name);
        ok = spec->drawf.d1(0, LEN(buf_f), p0f, rng); check_failure(ok, rng);
        ok = spec->drawf.d1(buf_f, LEN(buf_f), p0f, 0); xCheck(!ok);
      }
    }
    else if (spec->arity == 2) {
      ok = spec->draw.d2(buf, 0, p0, p1, rng); check_success(ok, rng);
      CHECK_EQUALV_MSG(buf, orig, LEN(buf), spec->name);
      ok = spec->draw.d2(0, LEN(buf), p0, p1, rng); check_failure(ok, rng);
      ok = spec->draw.d2(buf, LEN(buf), p0, p1, 0); xCheck(!ok);
      if (TESTFLOAT) {
        ok = spec->drawf.d2(buf_f, 0, p0f, p1f, rng); check_success(ok, rng);
        CHECK_EQUALV_MSG(buf_f, orig_f, LEN(buf_f), spec->name);
        ok = spec->drawf.d2(0, LEN(buf_f), p0f, p1f, rng);
        check_failure(ok, rng);
        ok = spec->drawf.d2(buf_f, LEN(buf_f), p0f, p1f, 0); xCheck(!ok);
      }
    }
    else if (spec->arity == 3) {
      ok = spec->draw.d3(buf, 0, p0, p1, p2, rng); check_success(ok, rng);
      CHECK_EQUALV_MSG(buf, orig, LEN(buf), spec->name);
      ok = spec->draw.d3(0, LEN(buf), p0, p1, p2, rng); check_failure(ok, rng);
      ok = spec->draw.d3(buf, LEN(buf), p0, p1, p2, 0); xCheck(!ok);
      if (TESTFLOAT) {
        ok = spec->drawf.d3(buf_f, 0, p0f, p1f, p2f, rng);
        check_success(ok, rng);
        CHECK_EQUALV_MSG(buf_f, orig_f, LEN(buf_f), spec->name);
        ok = spec->drawf.d3(0, LEN(buf_f), p0f, p1f, p2f, rng);
        check_failure(ok, rng);
        ok = spec->drawf.d3(buf_f, LEN(buf_f), p0f, p1f, p2f, 0); xCheck(!ok);
      }
    }
    else {
      xCheck(false);
    }
  }
  for (size_t i = 0; i < LEN(illegal_cases); i++) {
    const illegal_case *c = &illegal_cases[i];
    const dist_spec *spec = find_spec(c->name, c->arity);
    xCheck(spec != 0);
    if (c->arity == 1) {
      ok = spec->draw.d1(buf, LEN(buf), c->p0, rng);
      check_failure(ok, rng);
      if (TESTFLOAT) {
        ok = spec->drawf.d1(buf_f, LEN(buf_f), (float)c->p0, rng);
        check_failure(ok, rng);
      }
    }
    else if (c->arity == 2) {
      ok = spec->draw.d2(buf, LEN(buf), c->p0, c->p1, rng);
      check_failure(ok, rng);
      if (TESTFLOAT) {
        ok = spec->drawf.d2(buf_f, LEN(buf_f), (float)c->p0, (float)c->p1, rng);
        check_failure(ok, rng);
      }
    }
    else if (c->arity == 3) {
      ok = spec->draw.d3(buf, LEN(buf), c->p0, c->p1, c->p2, rng);
      check_failure(ok, rng);
      if (TESTFLOAT) {
        ok = spec->drawf.d3(buf_f, LEN(buf_f), (float)c->p0, (float)c->p1,
          (float)c->p2, rng);
        check_failure(ok, rng);
      }
    }
    else {
      xCheck(false);
    }
  }
  randompack_free(rng);
}

static void pit_one_case(const char *engine, const dist_spec *spec, double p0,
  double p1, double p2) {
  if (TESTVERBOSITY >= 2) {
    fprintf(stderr, "PIT %s\n", spec->name);
    fflush(stderr);
  }
  // Run PIT test for a specified engine / distribution / parameter-tuple
  int N = N_STAT_SLOW;
  double *x, *u;
  float *y = 0;
  float *v = 0;
  TEST_ALLOC(x, N);
  TEST_ALLOC(u, N);
  if (TESTFLOAT) {
    TEST_ALLOC(y, N);
    TEST_ALLOC(v, N);
  }
  if (spec->arity == 1) {
    float p0f = (float)p0;
    DRAW(engine, 42, spec->draw.d1(x, N, p0, rng));
    for (int i = 0; i < N; i++) u[i] = spec->cdf.c1(x[i], p0);
    if (TESTFLOAT) {
      DRAW(engine, 42, spec->drawf.d1(y, N, p0f, rng));
      for (int i = 0; i < N; i++) v[i] = (float)spec->cdf.c1((double)y[i], p0);
    }
  }
  else if (spec->arity == 2) {
    float p0f = (float)p0;
    float p1f = (float)p1;
    DRAW(engine, 42, spec->draw.d2(x, N, p0, p1, rng));
    for (int i = 0; i < N; i++) u[i] = spec->cdf.c2(x[i], p0, p1);
    if (TESTFLOAT) {
      DRAW(engine, 42, spec->drawf.d2(y, N, p0f, p1f, rng));
      for (int i = 0; i < N; i++)
        v[i] = (float)spec->cdf.c2((double)y[i], p0, p1);
    }
  }
  else if (spec->arity == 3) {
    float p0f = (float)p0;
    float p1f = (float)p1;
    float p2f = (float)p2;
    DRAW(engine, 42, spec->draw.d3(x, N, p0, p1, p2, rng));
    for (int i = 0; i < N; i++) u[i] = spec->cdf.c3(x[i], p0, p1, p2);
    if (TESTFLOAT) {
      DRAW(engine, 42, spec->drawf.d3(y, N, p0f, p1f, p2f, rng));
      for (int i = 0; i < N; i++)
        v[i] = (float)spec->cdf.c3((double)y[i], p0, p1, p2);
    }
  }
  else if (spec->arity == 0) {
    DRAW(engine, 42, spec->draw.d0(x, N, rng));
    for (int i = 0; i < N; i++) u[i] = spec->cdf.c0(x[i]);
    if (TESTFLOAT) {
      DRAW(engine, 42, spec->drawf.d0(y, N, rng));
      for (int i = 0; i < N; i++) v[i] = (float)spec->cdf.c0((double)y[i]);
    }
  }
  else {
    xCheck(false);
  }
  if (spec->support != SUPPORT_NONE) {
    double smin = 0;
    double smax = INFINITY;
    if (spec->support == SUPPORT_UNIT) smax = 1;
    else if (spec->support == SUPPORT_PARETO) smin = p0;
    TEST_SUPPORT(double, x, N, smin, smax);
    if (TESTFLOAT) {
      TEST_SUPPORT(float, y, N, (float)smin, (float)smax);
    }
  }
  check_u01_distribution(u, N, (char *)spec->name, (char *)engine);
  if (TESTFLOAT) {
    check_u01_distributionf(v, N, (char *)spec->name, (char *)engine);
    FREE(v);
    FREE(y);
  }
  FREE(u);
  FREE(x);
}

static void test_determinism_and_PIT(const char *engine) {
  if (TESTVERBOSITY >= 2) {
    fprintf(stderr, "TestContinuous determinism and PIT\n");
    fflush(stderr);
  }
  for (int i = 0; i < LEN(dist_specs); i++) {
    const dist_spec *spec = &dist_specs[i];
    if (TESTVERBOSITY >= 2) {
      fprintf(stderr, "dist %s\n", spec->name);
      fflush(stderr);
    }
    double xd[3], yd[3], zd[3];
    float xf[3], yf[3], zf[3];
    int len = LEN(xd);
    // Determinism: same seed => same draws, different seed => different draws.
    // Determinism uses the first parameter set.
    if (spec->arity == 0) {
      DRAW(engine, 42, spec->draw.d0(xd, len, rng));
      DRAW(engine, 42, spec->draw.d0(yd, len, rng));
      DRAW(engine, 43, spec->draw.d0(zd, len, rng));
      CHECK_EQUALV_MSG(xd, yd, len, spec->name);
      CHECK_DIFFV_MSG(xd, zd, len, spec->name);
      if (TESTFLOAT) {
        DRAW(engine, 42, spec->drawf.d0(xf, len, rng));
        DRAW(engine, 42, spec->drawf.d0(yf, len, rng));
        DRAW(engine, 43, spec->drawf.d0(zf, len, rng));
        CHECK_EQUALV_MSG(xf, yf, len, spec->name);
        CHECK_DIFFV_MSG(xf, zf, len, spec->name);
      }
    }
    else if (spec->arity == 1) {
      double p0 = spec->p0[0];
      float p0f = (float)p0;
      DRAW(engine, 42, spec->draw.d1(xd, len, p0, rng));
      DRAW(engine, 42, spec->draw.d1(yd, len, p0, rng));
      DRAW(engine, 43, spec->draw.d1(zd, len, p0, rng));
      CHECK_EQUALV_MSG(xd, yd, len, spec->name);
      CHECK_DIFFV_MSG(xd, zd, len, spec->name);
      if (TESTFLOAT) {
        DRAW(engine, 42, spec->drawf.d1(xf, len, p0f, rng));
        DRAW(engine, 42, spec->drawf.d1(yf, len, p0f, rng));
        DRAW(engine, 43, spec->drawf.d1(zf, len, p0f, rng));
        CHECK_EQUALV_MSG(xf, yf, len, spec->name);
        CHECK_DIFFV_MSG(xf, zf, len, spec->name);
      }
    }
    else if (spec->arity == 2) {
      double p0 = spec->p0[0];
      double p1 = spec->p1[0];
      float p0f = (float)p0;
      float p1f = (float)p1;
      DRAW(engine, 42, spec->draw.d2(xd, len, p0, p1, rng));
      DRAW(engine, 42, spec->draw.d2(yd, len, p0, p1, rng));
      DRAW(engine, 43, spec->draw.d2(zd, len, p0, p1, rng));
      CHECK_EQUALV_MSG(xd, yd, len, spec->name);
      CHECK_DIFFV_MSG(xd, zd, len, spec->name);
      if (TESTFLOAT) {
        DRAW(engine, 42, spec->drawf.d2(xf, len, p0f, p1f, rng));
        DRAW(engine, 42, spec->drawf.d2(yf, len, p0f, p1f, rng));
        DRAW(engine, 43, spec->drawf.d2(zf, len, p0f, p1f, rng));
        CHECK_EQUALV_MSG(xf, yf, len, spec->name);
        CHECK_DIFFV_MSG(xf, zf, len, spec->name);
      }
    }
    else if (spec->arity == 3) {
      double p0 = spec->p0[0];
      double p1 = spec->p1[0];
      double p2 = spec->p2[0];
      float p0f = (float)p0;
      float p1f = (float)p1;
      float p2f = (float)p2;
      DRAW(engine, 42, spec->draw.d3(xd, len, p0, p1, p2, rng));
      DRAW(engine, 42, spec->draw.d3(yd, len, p0, p1, p2, rng));
      DRAW(engine, 43, spec->draw.d3(zd, len, p0, p1, p2, rng));
      CHECK_EQUALV_MSG(xd, yd, len, spec->name);
      CHECK_DIFFV_MSG(xd, zd, len, spec->name);
      if (TESTFLOAT) {
        DRAW(engine, 42, spec->drawf.d3(xf, len, p0f, p1f, p2f, rng));
        DRAW(engine, 42, spec->drawf.d3(yf, len, p0f, p1f, p2f, rng));
        DRAW(engine, 43, spec->drawf.d3(zf, len, p0f, p1f, p2f, rng));
        CHECK_EQUALV_MSG(xf, yf, len, spec->name);
        CHECK_DIFFV_MSG(xf, zf, len, spec->name);
      }
    }
    else {
      xCheck(false);
    }
    // PIT: map draws through CDF and check for U(0,1).
    // Each spec may define multiple parameter cases.
    for (int k = 0; k < spec->ncases; k++)
      pit_one_case(engine, spec, spec->p0[k], spec->p1[k], spec->p2[k]);
  }
}

void TestContinuous(void) {
  int n = 0;
  char **engines = get_engines(&n);
  for (int i = 0; i < n; i++) {
    const char *engine = engines[i];
    test_continuous_edge_cases(engine);
    test_determinism_and_PIT(engine);
  }
  free_engines(engines, n);
}

void TestContinuousx(char *engine) {
  test_continuous_edge_cases(engine);
  test_determinism_and_PIT(engine);
}
