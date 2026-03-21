// -*- C -*-
// TimeLogExp.c: time exp/log implementations (ns/value).

#if defined(BUILD_AVX2)
#include <immintrin.h>
#endif
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "getopt.h"

#include "Util.h"
#include "randompack.h"
#include "randompack_config.h"
#include "openlibm.inc"

#if defined(USE_ACCEL_VV)
void vvexp(double *y, const double *x, const int *n);
void vvlog(double *y, const double *x, const int *n);
void vvexpf(float *y, const float *x, const int *n);
void vvlogf(float *y, const float *x, const int *n);
#endif

#if defined(BUILD_AVX2)
__m256d Sleef_expd4_u10avx2(__m256d d);
__m256d Sleef_logd4_u10avx2(__m256d d);
__m256d Sleef_logd4_u35avx2(__m256d d);
__m256 Sleef_expf8_u10avx2(__m256 d);
__m256 Sleef_logf8_u10avx2(__m256 d);
#endif

static void print_engines(void) {
  int n = 0;
  int elen = 0;
  int dlen = 0;
  if (!randompack_engines(0, 0, &n, &elen, &dlen) || n <= 0 || elen <= 0 ||
      dlen <= 0) {
    printf("  (no engines available)\n");
    return;
  }
  char *names = malloc((size_t)n*(size_t)elen);
  char *descs = malloc((size_t)n*(size_t)dlen);
  if (!names || !descs) {
    free(names);
    free(descs);
    printf("  (allocation failed)\n");
    return;
  }
  if (!randompack_engines(names, descs, &n, &elen, &dlen)) {
    free(names);
    free(descs);
    printf("  (engine listing unavailable)\n");
    return;
  }
  int width = elen - 1;
  for (int i = 0; i < n; i++) {
    printf("  %-*s  %s\n", width, names + i*elen, descs + i*dlen);
  }
  free(names);
  free(descs);
}

static void print_help(void) {
  printf("TimeLogExp - time exp/log implementations (ns/value)\n");
  printf("Usage: TimeLogExp [options]\n\n");
  printf("Options:\n");
  printf("  -h            Show this help message\n");
  printf("  -e engine     RNG engine (default x256++simd)\n");
  printf("  -t seconds    Benchmark time per function (default 0.1)\n");
  printf("  -c chunk      Chunk size (values per call, default 4096)\n");
  printf("  -s seed       RNG seed (default 7)\n\n");
  printf("Engines:\n");
  print_engines();
}

static bool get_options(int argc, char **argv, char **engine, double *bench_time,
                        int *chunk, int *seed, bool *help) {
  opterr = 0;
  optind = 1;
  int opt;
  *engine = "x256++simd";
  *bench_time = 0.1;
  *chunk = 4096;
  *seed = 7;
  *help = false;
  while ((opt = getopt(argc, argv, "he:t:c:s:")) != -1) {
    switch (opt) {
      case 'h':
        *help = true;
        return true;
      case 'e':
        *engine = optarg;
        break;
      case 't':
        *bench_time = atof(optarg);
        if (*bench_time <= 0)
          return false;
        break;
      case 'c':
        *chunk = atoi(optarg);
        if (*chunk <= 0)
          return false;
        break;
      case 's':
        *seed = atoi(optarg);
        break;
      default:
        return false;
    }
  }
  if (optind < argc)
    return false;
  return true;
}

static inline void consume5(const double *buf, int chunk) {
  static volatile uint64_t sink;
  uint64_t u;
  memcpy(&u, &buf[0], sizeof(u)); sink ^= u;
  memcpy(&u, &buf[chunk/4], sizeof(u)); sink ^= u;
  memcpy(&u, &buf[chunk/2], sizeof(u)); sink ^= u;
  memcpy(&u, &buf[3*chunk/4], sizeof(u)); sink ^= u;
  memcpy(&u, &buf[chunk-1], sizeof(u)); sink ^= u;
}

static inline void consume32(const void *p) {
  static volatile uint32_t sink;
  uint32_t u;
  memcpy(&u, p, sizeof(u));
  sink ^= u;
}

typedef double (*exp_fn)(double x);
typedef float (*expf_fn)(float x);
#if defined(USE_ACCEL_VV)
typedef void (*vvexp_fn)(double *y, const double *x, const int *n);
typedef void (*vvexpf_fn)(float *y, const float *x, const int *n);
#endif
#if defined(BUILD_AVX2)
typedef __m256d (*exp_vec_d_fn)(__m256d x);
typedef __m256 (*exp_vec_f_fn)(__m256 x);
#endif

typedef struct {
  char *name;
  exp_fn fn;
} exp_spec;

typedef struct {
  char *name;
  expf_fn fn;
} expf_spec;

#if defined(USE_ACCEL_VV)
typedef struct {
  char *name;
  vvexp_fn fn;
} vvexp_spec;

typedef struct {
  char *name;
  vvexpf_fn fn;
} vvexpf_spec;
#endif

static void apply_fn_scalar(double out[], const double in[], int n, exp_fn fn) {
  for (int i = 0; i < n; i++)
    out[i] = fn(in[i]);
}

static void apply_fn_scalar_f(float out[], const float in[], int n, expf_fn fn) {
  for (int i = 0; i < n; i++)
    out[i] = fn(in[i]);
}

#if defined(USE_ACCEL_VV)
static void apply_fn_vv_d(double out[], const double in[], int n, vvexp_fn fn) {
  fn(out, in, &n);
}

static void apply_fn_vv_f(float out[], const float in[], int n, vvexpf_fn fn) {
  fn(out, in, &n);
}
#endif

#if defined(BUILD_AVX2)
static void apply_fn_vec_d(double out[], const double in[], int n, exp_vec_d_fn fn) {
  for (int i = 0; i < n; i += 4) {
    __m256d x = _mm256_loadu_pd(in + i);
    __m256d y = fn(x);
    _mm256_storeu_pd(out + i, y);
  }
}

static void apply_fn_vec_f(float out[], const float in[], int n, exp_vec_f_fn fn) {
  for (int i = 0; i < n; i += 8) {
    __m256 x = _mm256_loadu_ps(in + i);
    __m256 y = fn(x);
    _mm256_storeu_ps(out + i, y);
  }
}
#endif

static double time_fn_scalar(int chunk, double bench_time, exp_fn fn, double in[],
                             double out[], randompack_rng *rng) {
  int reps = max(1, 1000000/chunk);
  int64_t calls = 0;
  uint64_t total = 0;
  size_t len = (size_t)chunk;
  uint64_t t0 = clock_nsec();
  uint64_t deadline = t0 + (uint64_t)(bench_time*1e9);
  uint64_t t = t0;
  while (t < deadline) {
    ASSERT(randompack_u01(in, len, rng));
    uint64_t t1 = clock_nsec();
    for (int i = 0; i < reps; i++) {
      apply_fn_scalar(out, in, chunk, fn);
      consume5(out, chunk);
    }
    uint64_t t2 = clock_nsec();
    total += t2 - t1;
    calls += reps;
    t = clock_nsec();
  }
  return (calls > 0) ? total/((double)calls*chunk) : 0;
}

static double time_fn_scalar_f(int chunk, double bench_time, expf_fn fn, float in[],
                               float out[], randompack_rng *rng) {
  int reps = max(1, 1000000/chunk);
  int64_t calls = 0;
  uint64_t total = 0;
  size_t len = (size_t)chunk;
  uint64_t t0 = clock_nsec();
  uint64_t deadline = t0 + (uint64_t)(bench_time*1e9);
  uint64_t t = t0;
  while (t < deadline) {
    ASSERT(randompack_u01f(in, len, rng));
    uint64_t t1 = clock_nsec();
    for (int i = 0; i < reps; i++) {
      apply_fn_scalar_f(out, in, chunk, fn);
      consume32(&out[chunk - 1]);
    }
    uint64_t t2 = clock_nsec();
    total += t2 - t1;
    calls += reps;
    t = clock_nsec();
  }
  return (calls > 0) ? total/((double)calls*chunk) : 0;
}

#if defined(USE_ACCEL_VV)
static double time_fn_vv_d(int chunk, double bench_time, vvexp_fn fn, double in[],
                           double out[], randompack_rng *rng) {
  int reps = max(1, 1000000/chunk);
  int64_t calls = 0;
  uint64_t total = 0;
  size_t len = (size_t)chunk;
  uint64_t t0 = clock_nsec();
  uint64_t deadline = t0 + (uint64_t)(bench_time*1e9);
  uint64_t t = t0;
  while (t < deadline) {
    ASSERT(randompack_u01(in, len, rng));
    uint64_t t1 = clock_nsec();
    for (int i = 0; i < reps; i++) {
      apply_fn_vv_d(out, in, chunk, fn);
      consume5(out, chunk);
    }
    uint64_t t2 = clock_nsec();
    total += t2 - t1;
    calls += reps;
    t = clock_nsec();
  }
  return (calls > 0) ? total/((double)calls*chunk) : 0;
}

static double time_fn_vv_f(int chunk, double bench_time, vvexpf_fn fn, float in[],
                           float out[], randompack_rng *rng) {
  int reps = max(1, 1000000/chunk);
  int64_t calls = 0;
  uint64_t total = 0;
  size_t len = (size_t)chunk;
  uint64_t t0 = clock_nsec();
  uint64_t deadline = t0 + (uint64_t)(bench_time*1e9);
  uint64_t t = t0;
  while (t < deadline) {
    ASSERT(randompack_u01f(in, len, rng));
    uint64_t t1 = clock_nsec();
    for (int i = 0; i < reps; i++) {
      apply_fn_vv_f(out, in, chunk, fn);
      consume32(&out[chunk - 1]);
    }
    uint64_t t2 = clock_nsec();
    total += t2 - t1;
    calls += reps;
    t = clock_nsec();
  }
  return (calls > 0) ? total/((double)calls*chunk) : 0;
}
#endif

#if defined(BUILD_AVX2)
static double time_fn_vec_d(int chunk, double bench_time, exp_vec_d_fn fn,
                            double in[], double out[], randompack_rng *rng) {
  int reps = max(1, 1000000/chunk);
  int64_t calls = 0;
  uint64_t total = 0;
  size_t len = (size_t)chunk;
  uint64_t t0 = clock_nsec();
  uint64_t deadline = t0 + (uint64_t)(bench_time*1e9);
  uint64_t t = t0;
  while (t < deadline) {
    ASSERT(randompack_u01(in, len, rng));
    uint64_t t1 = clock_nsec();
    for (int i = 0; i < reps; i++) {
      apply_fn_vec_d(out, in, chunk, fn);
      consume5(out, chunk);
    }
    uint64_t t2 = clock_nsec();
    total += t2 - t1;
    calls += reps;
    t = clock_nsec();
  }
  return (calls > 0) ? total/((double)calls*chunk) : 0;
}

static double time_fn_vec_f(int chunk, double bench_time, exp_vec_f_fn fn,
                            float in[], float out[], randompack_rng *rng) {
  int reps = max(1, 1000000/chunk);
  int64_t calls = 0;
  uint64_t total = 0;
  size_t len = (size_t)chunk;
  uint64_t t0 = clock_nsec();
  uint64_t deadline = t0 + (uint64_t)(bench_time*1e9);
  uint64_t t = t0;
  while (t < deadline) {
    ASSERT(randompack_u01f(in, len, rng));
    uint64_t t1 = clock_nsec();
    for (int i = 0; i < reps; i++) {
      apply_fn_vec_f(out, in, chunk, fn);
      consume32(&out[chunk - 1]);
    }
    uint64_t t2 = clock_nsec();
    total += t2 - t1;
    calls += reps;
    t = clock_nsec();
  }
  return (calls > 0) ? total/((double)calls*chunk) : 0;
}
#endif

int main(int argc, char **argv) {
  char *engine;
  double bench_time;
  int chunk, seed;
  bool help;
  if (!get_options(argc, argv, &engine, &bench_time, &chunk, &seed, &help) ||
      help) {
    print_help();
    return help ? 0 : 1;
  }
#if defined(__linux__)
  pin_to_cpu0();
#endif
  randompack_rng *rng = randompack_create(engine);
  if (!rng) {
    fprintf(stderr, "randompack_create failed: %s\n", engine);
    return 1;
  }
  if (!randompack_seed(seed, 0, 0, rng)) {
    fprintf(stderr, "randompack_seed failed\n");
    randompack_free(rng);
    return 1;
  }
  const double warmup_time = 0.1;
  uint64_t t0 = clock_nsec();
  uint64_t deadline = t0 + (uint64_t)(warmup_time*1e9);
  uint64_t t = t0;
  while (t < deadline) {
    warmup_cpu(10);
    t = clock_nsec();
  }
  if (chunk % 4 != 0 || chunk % 8 != 0) {
    fprintf(stderr, "chunk must be a multiple of 4 and 8\n");
    randompack_free(rng);
    return 1;
  }
#if defined(BUILD_AVX2)
  double *test_in = 0;
  double *test_out = 0;
  float *test_in_f = 0;
  float *test_out_f = 0;
  double tests[2] = { 0.2, 3.0 };
  float testsf[2] = { 0.2f, 3.0f };
  if (!ALLOC(test_in, 4) || !ALLOC(test_out, 4) ||
      !ALLOC(test_in_f, 8) || !ALLOC(test_out_f, 8)) {
    fprintf(stderr, "allocation failed\n");
    FREE(test_in);
    FREE(test_out);
    FREE(test_in_f);
    FREE(test_out_f);
    randompack_free(rng);
    return 1;
  }
  for (int i = 0; i < 2; i++) {
    double x = tests[i];
    double ref = exp(x);
    double ol = openlibm_exp(x);
    for (int j = 0; j < 4; j++)
      test_in[j] = x;
    apply_fn_vec_d(test_out, test_in, 4, Sleef_expd4_u10avx2);
    double sv = test_out[0];
    if (fabs(ref - ol) > 1e-15 || fabs(ref - sv) > 1e-15) {
      fprintf(stderr, "smoke test failed for x=%.17g\n", x);
      FREE(test_in);
      FREE(test_out);
      FREE(test_in_f);
      FREE(test_out_f);
      randompack_free(rng);
      return 1;
    }
  }
  for (int i = 0; i < 2; i++) {
    float x = testsf[i];
    float ref = expf(x);
    float ol = openlibm_expf(x);
    for (int j = 0; j < 8; j++)
      test_in_f[j] = x;
    apply_fn_vec_f(test_out_f, test_in_f, 8, Sleef_expf8_u10avx2);
    float sv = test_out_f[0];
    if (fabsf(ref - ol) > 1e-6f || fabsf(ref - sv) > 1e-6f) {
      fprintf(stderr, "smoke test failed for x=%.9g\n", x);
      FREE(test_in);
      FREE(test_out);
      FREE(test_in_f);
      FREE(test_out_f);
      randompack_free(rng);
      return 1;
    }
  }
  for (int i = 0; i < 2; i++) {
    double x = tests[i];
    double ref = log(x);
    double ol = openlibm_log(x);
    for (int j = 0; j < 4; j++)
      test_in[j] = x;
    apply_fn_vec_d(test_out, test_in, 4, Sleef_logd4_u10avx2);
    double sv = test_out[0];
    if (fabs(ref - ol) > 1e-15 || fabs(ref - sv) > 1e-15) {
      fprintf(stderr, "smoke test failed for x=%.17g\n", x);
      FREE(test_in);
      FREE(test_out);
      FREE(test_in_f);
      FREE(test_out_f);
      randompack_free(rng);
      return 1;
    }
  }
  for (int i = 0; i < 2; i++) {
    float x = testsf[i];
    float ref = logf(x);
    float ol = openlibm_logf(x);
    for (int j = 0; j < 8; j++)
      test_in_f[j] = x;
    apply_fn_vec_f(test_out_f, test_in_f, 8, Sleef_logf8_u10avx2);
    float sv = test_out_f[0];
    if (fabsf(ref - ol) > 1e-6f || fabsf(ref - sv) > 1e-6f) {
      fprintf(stderr, "smoke test failed for x=%.9g\n", x);
      FREE(test_in);
      FREE(test_out);
      FREE(test_in_f);
      FREE(test_out_f);
      randompack_free(rng);
      return 1;
    }
  }
  FREE(test_in);
  FREE(test_out);
  FREE(test_in_f);
  FREE(test_out_f);
#endif
  double *in = 0;
  double *out = 0;
  float *inf = 0;
  float *outf = 0;
  if (!ALLOC(in, chunk) || !ALLOC(out, chunk) ||
      !ALLOC(inf, chunk) || !ALLOC(outf, chunk)) {
    fprintf(stderr, "allocation failed\n");
    FREE(in);
    FREE(out);
    FREE(inf);
    FREE(outf);
    randompack_free(rng);
    return 1;
  }
  exp_spec exps[] = {
    { "openlibm_exp", openlibm_exp },
    { "exp", exp },
  };
  expf_spec expfs[] = {
    { "openlibm_expf", openlibm_expf },
    { "expf", expf },
  };
#if defined(USE_ACCEL_VV)
  vvexp_spec vvexps[] = {
    { "vvexp", vvexp },
    { "vvlog", vvlog },
  };
  vvexpf_spec vvexpfs[] = {
    { "vvexpf", vvexpf },
    { "vvlogf", vvlogf },
  };
#endif
  printf("time per value:   ns/value\n");
  printf("warmup_time:      %.3f s\n", warmup_time);
  printf("bench_time:       %.3f s per func\n", bench_time);
  printf("chunk:            %d\n\n", chunk);
  printf("%-22s %10s\n", "Function", "double");
  for (int i = 0; i < LEN(exps); i++) {
    double ns = time_fn_scalar(chunk, bench_time, exps[i].fn, in, out, rng);
    printf("%-22s %10.2f\n", exps[i].name, ns);
  }
#if defined(USE_ACCEL_VV)
  for (int i = 0; i < LEN(vvexps); i++) {
    double ns = time_fn_vv_d(chunk, bench_time, vvexps[i].fn, in, out, rng);
    printf("%-22s %10.2f\n", vvexps[i].name, ns);
  }
#endif
#if defined(BUILD_AVX2)
  {
    double ns = time_fn_vec_d(chunk, bench_time, Sleef_expd4_u10avx2, in, out, rng);
    printf("%-22s %10.2f\n", "sleef_exp", ns);
  }
#endif
  double ns = time_fn_scalar(chunk, bench_time, openlibm_log, in, out, rng);
  printf("%-22s %10.2f\n", "openlibm_log", ns);
  ns = time_fn_scalar(chunk, bench_time, log, in, out, rng);
  printf("%-22s %10.2f\n", "log", ns);
#if defined(BUILD_AVX2)
  ns = time_fn_vec_d(chunk, bench_time, Sleef_logd4_u10avx2, in, out, rng);
  printf("%-22s %10.2f\n", "sleef_u10log", ns);
  ns = time_fn_vec_d(chunk, bench_time, Sleef_logd4_u35avx2, in, out, rng);
  printf("%-22s %10.2f\n", "sleef_u35log", ns);
#endif
  printf("\n%-22s %10s\n", "Function", "float");
  for (int i = 0; i < LEN(expfs); i++) {
    double nsf = time_fn_scalar_f(chunk, bench_time, expfs[i].fn, inf, outf, rng);
    printf("%-22s %10.2f\n", expfs[i].name, nsf);
  }
#if defined(USE_ACCEL_VV)
  for (int i = 0; i < LEN(vvexpfs); i++) {
    double nsf = time_fn_vv_f(chunk, bench_time, vvexpfs[i].fn, inf, outf, rng);
    printf("%-22s %10.2f\n", vvexpfs[i].name, nsf);
  }
#endif
#if defined(BUILD_AVX2)
  {
    double nsf = time_fn_vec_f(chunk, bench_time, Sleef_expf8_u10avx2, inf, outf, rng);
    printf("%-22s %10.2f\n", "sleef_expf", nsf);
  }
#endif
  double nsf = time_fn_scalar_f(chunk, bench_time, openlibm_logf, inf, outf, rng);
  printf("%-22s %10.2f\n", "openlibm_logf", nsf);
  nsf = time_fn_scalar_f(chunk, bench_time, logf, inf, outf, rng);
  printf("%-22s %10.2f\n", "logf", nsf);
#if defined(BUILD_AVX2)
  nsf = time_fn_vec_f(chunk, bench_time, Sleef_logf8_u10avx2, inf, outf, rng);
  printf("%-22s %10.2f\n", "sleef_u10logf", nsf);
#endif
  FREE(in);
  FREE(out);
  FREE(inf);
  FREE(outf);
  randompack_free(rng);
  return 0;
}
