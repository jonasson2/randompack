// -*- C -*-
// TimeLogExp.c: time exp/log implementations (ns/value).

#if defined(BUILD_AVX2)
#include <immintrin.h>
#elif defined(__aarch64__) || defined(_M_ARM64)
#include <arm_neon.h>
#endif
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "getopt.h"

#include "TimeUtil.h"
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
#if defined(BUILD_AVX512)
void sleef_avx512_exp(double out[], const double in[], int n);
void sleef_avx512_log(double out[], const double in[], int n);
#endif

#if defined(BUILD_AVX2)
__m256d Sleef_expd4_u10avx2(__m256d d);
__m256d Sleef_logd4_u10avx2(__m256d d);
__m256d Sleef_logd4_u35avx2(__m256d d);
__m256 Sleef_expf8_u10avx2(__m256 d);
__m256 Sleef_logf8_u10avx2(__m256 d);
#elif defined(__aarch64__) || defined(_M_ARM64)
float64x2_t Sleef_expd2_u10advsimd(float64x2_t d);
float64x2_t Sleef_logd2_u35advsimd(float64x2_t d);
float32x4_t Sleef_expf4_u10advsimd(float32x4_t d);
float32x4_t Sleef_logf4_u10advsimd(float32x4_t d);
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
typedef void (*arr_d_fn)(double out[], const double in[], int n);
#if defined(USE_ACCEL_VV)
typedef void (*vvexp_fn)(double *y, const double *x, const int *n);
typedef void (*vvexpf_fn)(float *y, const float *x, const int *n);
#endif
#if defined(BUILD_AVX2)
typedef __m256d (*exp_vec_d_fn)(__m256d x);
typedef __m256 (*exp_vec_f_fn)(__m256 x);
#elif defined(__aarch64__) || defined(_M_ARM64)
typedef float64x2_t (*exp_vec_d_fn)(float64x2_t x);
typedef float32x4_t (*exp_vec_f_fn)(float32x4_t x);
#endif

typedef struct {
  char *name;
  exp_fn fn;
} exp_spec;

typedef struct {
  char *name;
  expf_fn fn;
} expf_spec;

typedef struct {
  char *name;
  arr_d_fn fn;
} arr_d_spec;

#if defined(BUILD_AVX2) || defined(__aarch64__) || defined(_M_ARM64)
typedef struct {
  char *name;
  exp_vec_d_fn fn;
} exp_vec_d_spec;

typedef struct {
  char *name;
  exp_vec_f_fn fn;
} exp_vec_f_spec;

#if defined(BUILD_AVX2)
enum { vec_d_lanes = 4, vec_f_lanes = 8 };
static exp_vec_d_spec vec_exps[] = {
  { "sleef_exp", Sleef_expd4_u10avx2 },
};
static exp_vec_d_spec vec_logs[] = {
  { "sleef_u10log", Sleef_logd4_u10avx2 },
  { "sleef_u35log", Sleef_logd4_u35avx2 },
};
static exp_vec_f_spec vec_expfs[] = {
  { "sleef_expf", Sleef_expf8_u10avx2 },
};
static exp_vec_f_spec vec_logfs[] = {
  { "sleef_u10logf", Sleef_logf8_u10avx2 },
};
#elif defined(__aarch64__) || defined(_M_ARM64)
enum { vec_d_lanes = 2, vec_f_lanes = 4 };
static exp_vec_d_spec vec_exps[] = {
  { "sleef_exp", Sleef_expd2_u10advsimd },
};
static exp_vec_d_spec vec_logs[] = {
  { "sleef_u35log", Sleef_logd2_u35advsimd },
};
static exp_vec_f_spec vec_expfs[] = {
  { "sleef_expf", Sleef_expf4_u10advsimd },
};
static exp_vec_f_spec vec_logfs[] = {
  { "sleef_u10logf", Sleef_logf4_u10advsimd },
};
#endif
#endif

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

#if defined(BUILD_AVX512)
static void apply_fn_arr_d(double out[], const double in[], int n, arr_d_fn fn) {
  fn(out, in, n);
}
#endif

static bool check_chunk(int chunk) {
#if defined(BUILD_AVX2) || defined(__aarch64__) || defined(_M_ARM64)
  return chunk % vec_d_lanes == 0 && chunk % vec_f_lanes == 0;
#else
  return chunk > 0;
#endif
}

static char *chunk_msg(void) {
#if defined(BUILD_AVX2) || defined(__aarch64__) || defined(_M_ARM64)
  static char msg[64];
  snprintf(msg, sizeof(msg), "chunk must be a multiple of %d and %d",
      vec_d_lanes, vec_f_lanes);
  return msg;
#else
  return "chunk must be positive";
#endif
}

#if defined(USE_ACCEL_VV)
static void apply_fn_vv_d(double out[], const double in[], int n, vvexp_fn fn) {
  fn(out, in, &n);
}

static void apply_fn_vv_f(float out[], const float in[], int n, vvexpf_fn fn) {
  fn(out, in, &n);
}
#endif

#if defined(BUILD_AVX512)
static double time_fn_arr_d(int chunk, double bench_time, arr_d_fn fn, double in[],
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
      apply_fn_arr_d(out, in, chunk, fn);
      consume5(out, chunk);
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
#elif defined(__aarch64__) || defined(_M_ARM64)
static void apply_fn_vec_d(double out[], const double in[], int n, exp_vec_d_fn fn) {
  for (int i = 0; i < n; i += 2) {
    float64x2_t x = vld1q_f64(in + i);
    float64x2_t y = fn(x);
    vst1q_f64(out + i, y);
  }
}

static void apply_fn_vec_f(float out[], const float in[], int n, exp_vec_f_fn fn) {
  for (int i = 0; i < n; i += 4) {
    float32x4_t x = vld1q_f32(in + i);
    float32x4_t y = fn(x);
    vst1q_f32(out + i, y);
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

#if defined(BUILD_AVX2) || defined(__aarch64__) || defined(_M_ARM64)
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

#if defined(BUILD_AVX2) || defined(__aarch64__) || defined(_M_ARM64)
static bool smoke_vec_d(exp_vec_d_fn fn, exp_fn ref_fn, exp_fn libm_fn,
                        double tests[], int ntest) {
  double in[vec_d_lanes];
  double out[vec_d_lanes];
  for (int i = 0; i < ntest; i++) {
    double x = tests[i];
    double ref = ref_fn(x);
    double ol = libm_fn(x);
    for (int j = 0; j < vec_d_lanes; j++)
      in[j] = x;
    apply_fn_vec_d(out, in, vec_d_lanes, fn);
    if (fabs(ref - ol) > 1e-15 || fabs(ref - out[0]) > 1e-15)
      return false;
  }
  return true;
}

static bool smoke_vec_f(exp_vec_f_fn fn, expf_fn ref_fn, expf_fn libm_fn,
                        float tests[], int ntest) {
  float in[vec_f_lanes];
  float out[vec_f_lanes];
  for (int i = 0; i < ntest; i++) {
    float x = tests[i];
    float ref = ref_fn(x);
    float ol = libm_fn(x);
    for (int j = 0; j < vec_f_lanes; j++)
      in[j] = x;
    apply_fn_vec_f(out, in, vec_f_lanes, fn);
    if (fabsf(ref - ol) > 1e-6f || fabsf(ref - out[0]) > 1e-6f)
      return false;
  }
  return true;
}

static bool run_vec_smoke_tests(void) {
  double tests[] = { 0.2, 3.0 };
  float testsf[] = { 0.2f, 3.0f };
  for (int i = 0; i < LEN(vec_exps); i++) {
    if (!smoke_vec_d(vec_exps[i].fn, exp, openlibm_exp, tests, LEN(tests)))
      return false;
  }
  for (int i = 0; i < LEN(vec_logs); i++) {
    if (!smoke_vec_d(vec_logs[i].fn, log, openlibm_log, tests, LEN(tests)))
      return false;
  }
  for (int i = 0; i < LEN(vec_expfs); i++) {
    if (!smoke_vec_f(vec_expfs[i].fn, expf, openlibm_expf, testsf, LEN(testsf)))
      return false;
  }
  for (int i = 0; i < LEN(vec_logfs); i++) {
    if (!smoke_vec_f(vec_logfs[i].fn, logf, openlibm_logf, testsf, LEN(testsf)))
      return false;
  }
  return true;
}

static void print_vec_bench_d(int chunk, double bench_time, double in[],
                              double out[], randompack_rng *rng,
                              exp_vec_d_spec specs[], int n) {
  for (int i = 0; i < n; i++) {
    double ns = time_fn_vec_d(chunk, bench_time, specs[i].fn, in, out, rng);
    printf("%-22s %10.2f\n", specs[i].name, ns);
  }
}

static void print_vec_bench_f(int chunk, double bench_time, float in[],
                              float out[], randompack_rng *rng,
                              exp_vec_f_spec specs[], int n) {
  for (int i = 0; i < n; i++) {
    double ns = time_fn_vec_f(chunk, bench_time, specs[i].fn, in, out, rng);
    printf("%-22s %10.2f\n", specs[i].name, ns);
  }
}
#endif

#if defined(BUILD_AVX512)
static bool smoke_arr_d(arr_d_fn fn, exp_fn ref_fn, exp_fn libm_fn,
                        double tests[], int ntest) {
  double in[8];
  double out[8];
  for (int i = 0; i < ntest; i++) {
    double x = tests[i];
    double ref = ref_fn(x);
    double ol = libm_fn(x);
    for (int j = 0; j < 8; j++)
      in[j] = x;
    apply_fn_arr_d(out, in, 8, fn);
    if (fabs(ref - ol) > 1e-15 || fabs(ref - out[0]) > 1e-15)
      return false;
  }
  return true;
}

static bool run_avx512_smoke_tests(void) {
  double tests[] = { 0.2, 3.0 };
  return smoke_arr_d(sleef_avx512_exp, exp, openlibm_exp, tests, LEN(tests)) &&
      smoke_arr_d(sleef_avx512_log, log, openlibm_log, tests, LEN(tests));
}

static void print_arr_bench_d(int chunk, double bench_time, double in[],
                              double out[], randompack_rng *rng,
                              arr_d_spec specs[], int n) {
  for (int i = 0; i < n; i++) {
    double ns = time_fn_arr_d(chunk, bench_time, specs[i].fn, in, out, rng);
    printf("%-22s %10.2f\n", specs[i].name, ns);
  }
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
    warmup_cpu(0.1);
    t = clock_nsec();
  }
#if defined(BUILD_AVX2) || defined(__aarch64__) || defined(_M_ARM64)
  if (!check_chunk(chunk)) {
    fprintf(stderr, "%s\n", chunk_msg());
    randompack_free(rng);
    return 1;
  }
#endif
#if defined(BUILD_AVX2) || defined(__aarch64__) || defined(_M_ARM64)
  if (!run_vec_smoke_tests()) {
    fprintf(stderr, "smoke test failed\n");
    randompack_free(rng);
    return 1;
  }
#endif
#if defined(BUILD_AVX512)
  if (!run_avx512_smoke_tests()) {
    fprintf(stderr, "AVX-512 smoke test failed\n");
    randompack_free(rng);
    return 1;
  }
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
  };
  vvexpf_spec vvexpfs[] = {
    { "vvexpf", vvexpf },
  };
  vvexp_spec vvlogs[] = {
    { "vvlog", vvlog },
  };
  vvexpf_spec vvlogfs[] = {
    { "vvlogf", vvlogf },
  };
#endif
#if defined(BUILD_AVX512)
  arr_d_spec avx512_exps[] = {
    { "sleef_avx512_exp", sleef_avx512_exp },
  };
  arr_d_spec avx512_logs[] = {
    { "sleef_avx512_u35log", sleef_avx512_log },
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
#if defined(BUILD_AVX2) || defined(__aarch64__) || defined(_M_ARM64)
  print_vec_bench_d(chunk, bench_time, in, out, rng, vec_exps, LEN(vec_exps));
#endif
#if defined(BUILD_AVX512)
  print_arr_bench_d(chunk, bench_time, in, out, rng, avx512_exps, LEN(avx512_exps));
#endif
  double ns = time_fn_scalar(chunk, bench_time, openlibm_log, in, out, rng);
  printf("%-22s %10.2f\n", "openlibm_log", ns);
  ns = time_fn_scalar(chunk, bench_time, log, in, out, rng);
  printf("%-22s %10.2f\n", "log", ns);
#if defined(USE_ACCEL_VV)
  for (int i = 0; i < LEN(vvlogs); i++) {
    ns = time_fn_vv_d(chunk, bench_time, vvlogs[i].fn, in, out, rng);
    printf("%-22s %10.2f\n", vvlogs[i].name, ns);
  }
#endif
#if defined(BUILD_AVX2) || defined(__aarch64__) || defined(_M_ARM64)
  print_vec_bench_d(chunk, bench_time, in, out, rng, vec_logs, LEN(vec_logs));
#endif
#if defined(BUILD_AVX512)
  print_arr_bench_d(chunk, bench_time, in, out, rng, avx512_logs, LEN(avx512_logs));
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
#if defined(BUILD_AVX2) || defined(__aarch64__) || defined(_M_ARM64)
  print_vec_bench_f(chunk, bench_time, inf, outf, rng, vec_expfs, LEN(vec_expfs));
#endif
  double nsf = time_fn_scalar_f(chunk, bench_time, openlibm_logf, inf, outf, rng);
  printf("%-22s %10.2f\n", "openlibm_logf", nsf);
  nsf = time_fn_scalar_f(chunk, bench_time, logf, inf, outf, rng);
  printf("%-22s %10.2f\n", "logf", nsf);
#if defined(USE_ACCEL_VV)
  for (int i = 0; i < LEN(vvlogfs); i++) {
    nsf = time_fn_vv_f(chunk, bench_time, vvlogfs[i].fn, inf, outf, rng);
    printf("%-22s %10.2f\n", vvlogfs[i].name, nsf);
  }
#endif
#if defined(BUILD_AVX2) || defined(__aarch64__) || defined(_M_ARM64)
  print_vec_bench_f(chunk, bench_time, inf, outf, rng, vec_logfs, LEN(vec_logfs));
#endif
  FREE(in);
  FREE(out);
  FREE(inf);
  FREE(outf);
  randompack_free(rng);
  return 0;
}
