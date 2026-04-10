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

#if defined(BUILD_AVX512) && (defined(__x86_64__) || defined(_M_X64))
#if defined(_MSC_VER)
#include <intrin.h>
#else
#include <cpuid.h>
#endif
#endif

#if defined(USE_ACCEL_VV)
void vvexp(double *y, const double *x, const int *n);
void vvlog(double *y, const double *x, const int *n);
void vvexpf(float *y, const float *x, const int *n);
void vvlogf(float *y, const float *x, const int *n);
#endif
#if defined(BUILD_AVX512)
void sleef_exp_inplace_avx512(double *x, size_t len);
void sleef_log_inplace_avx512(double *x, size_t len);
#endif
#if defined(BUILD_AVX2) || (defined(__aarch64__) || defined(_M_ARM64)) && !defined(__APPLE__)
void sleef_exp_inplace(double *x, size_t len);
void sleef_log_inplace(double *x, size_t len);
void sleef_expf_inplace(float *x, size_t len);
void sleef_logf_inplace(float *x, size_t len);
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
  printf("  -t seconds    Benchmark time per function (default 0.2)\n");
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
  *bench_time = 0.2;
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

static inline void consume64(const void *p) {
  static volatile uint64_t sink;
  uint64_t u;
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

typedef void (*inplace_d_fn)(double *x, size_t len);
typedef void (*inplace_f_fn)(float *x, size_t len);

typedef struct {
  char *name;
  inplace_d_fn fn;
} inplace_d_spec;

typedef struct {
  char *name;
  inplace_f_fn fn;
} inplace_f_spec;

#if defined(BUILD_AVX512) && (defined(__x86_64__) || defined(_M_X64))
static bool cpu_has_avx512_local(void) {
#if defined(_MSC_VER)
  int info[4];
  __cpuid(info, 1);
  int osxsave = (info[2] & (1 << 27)) != 0;
  int avx = (info[2] & (1 << 28)) != 0;
  if (!(osxsave && avx))
    return false;
  unsigned long long xcr = _xgetbv(0);
  if ((xcr & 0xe6) != 0xe6)
    return false;
  __cpuidex(info, 7, 0);
  return (info[1] & (1 << 16)) != 0 && (info[1] & (1 << 17)) != 0;
#else
  unsigned int eax, ebx, ecx, edx;
  if (!__get_cpuid(1, &eax, &ebx, &ecx, &edx))
    return false;
  if (!(ecx & (1 << 27)) || !(ecx & (1 << 28)))
    return false;
  unsigned int xcr0_lo, xcr0_hi;
  __asm__ volatile ("xgetbv" : "=a"(xcr0_lo), "=d"(xcr0_hi) : "c"(0));
  if ((xcr0_lo & 0xe6) != 0xe6)
    return false;
  if (__get_cpuid_max(0, 0) < 7)
    return false;
  __cpuid_count(7, 0, eax, ebx, ecx, edx);
  return (ebx & (1 << 16)) != 0 && (ebx & (1 << 17)) != 0;
#endif
}
#endif

#if defined(BUILD_AVX2) || (defined(__aarch64__) || defined(_M_ARM64)) && !defined(__APPLE__)
static inplace_d_spec inplace_exps[] = {
  { "sleef_exp_inplace", sleef_exp_inplace },
};
static inplace_d_spec inplace_logs[] = {
  { "sleef_log_inplace", sleef_log_inplace },
};
static inplace_f_spec inplace_expfs[] = {
  { "sleef_expf_inplace", sleef_expf_inplace },
};
static inplace_f_spec inplace_logfs[] = {
  { "sleef_logf_inplace", sleef_logf_inplace },
};
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

static void fill_linspace_d(double out[], int n, double lo, double hi) {
  if (n <= 0)
    return;
  if (n == 1) {
    out[0] = lo;
    return;
  }
  double step = (hi - lo)/(double)(n - 1);
  for (int i = 0; i < n; i++)
    out[i] = lo + step*(double)i;
}

static void fill_linspace_f(float out[], int n, float lo, float hi) {
  if (n <= 0)
    return;
  if (n == 1) {
    out[0] = lo;
    return;
  }
  float step = (hi - lo)/(float)(n - 1);
  for (int i = 0; i < n; i++)
    out[i] = lo + step*(float)i;
}

#if defined(BUILD_AVX512)
static void sleef_avx512_exp_adapter(double out[], const double in[], int n) {
  if (out != in)
    memcpy(out, in, (size_t)n*sizeof(double));
  sleef_exp_inplace_avx512(out, (size_t)n);
}

static void sleef_avx512_log_adapter(double out[], const double in[], int n) {
  if (out != in)
    memcpy(out, in, (size_t)n*sizeof(double));
  sleef_log_inplace_avx512(out, (size_t)n);
}

static void apply_fn_arr_d(double out[], const double in[], int n, arr_d_fn fn) {
  fn(out, in, n);
}
#endif

static bool check_chunk(int chunk) {
  return chunk > 0;
}

static char *chunk_msg(void) {
  return "chunk must be positive";
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
static double time_fn_arr_d(int chunk, double bench_time, arr_d_fn fn, double out[],
                            randompack_rng *rng) {
  int reps = max(1, 1000000/chunk);
  int64_t calls = 0;
  uint64_t total = 0;
  size_t len = (size_t)chunk;
  uint64_t t0 = clock_nsec();
  uint64_t deadline = t0 + (uint64_t)(bench_time*1e9);
  uint64_t t = t0;
  while (t < deadline) {
    ASSERT(randompack_u01(out, len, rng));
    uint64_t t1 = clock_nsec();
    for (int i = 0; i < reps; i++) {
      apply_fn_arr_d(out, out, chunk, fn);
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

static double time_single_log_d(const double in[], int n, double bench_time,
                                exp_fn fn) {
  int64_t calls = 0;
  uint64_t total = 0;
  uint64_t t0 = clock_nsec();
  uint64_t deadline = t0 + (uint64_t)(bench_time*1e9);
  uint64_t t = t0;
  while (t < deadline) {
    uint64_t t1 = clock_nsec();
    for (int i = 0; i < n; i++) {
      double y = fn(in[i]);
      consume64(&y);
    }
    uint64_t t2 = clock_nsec();
    total += t2 - t1;
    calls += n;
    t = clock_nsec();
  }
  return (calls > 0) ? total/(double)calls : 0;
}

static double time_single_log_f(const float in[], int n, double bench_time,
                                expf_fn fn) {
  int64_t calls = 0;
  uint64_t total = 0;
  uint64_t t0 = clock_nsec();
  uint64_t deadline = t0 + (uint64_t)(bench_time*1e9);
  uint64_t t = t0;
  while (t < deadline) {
    uint64_t t1 = clock_nsec();
    for (int i = 0; i < n; i++) {
      float y = fn(in[i]);
      consume32(&y);
    }
    uint64_t t2 = clock_nsec();
    total += t2 - t1;
    calls += n;
    t = clock_nsec();
  }
  return (calls > 0) ? total/(double)calls : 0;
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

#if defined(BUILD_AVX2) || (defined(__aarch64__) || defined(_M_ARM64)) && !defined(__APPLE__)
static double time_fn_inplace_d(int chunk, double bench_time, inplace_d_fn fn,
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
      memcpy(out, in, len*sizeof(*out));
      fn(out, len);
      consume5(out, chunk);
    }
    uint64_t t2 = clock_nsec();
    total += t2 - t1;
    calls += reps;
    t = clock_nsec();
  }
  return (calls > 0) ? total/((double)calls*chunk) : 0;
}

static double time_fn_inplace_f(int chunk, double bench_time, inplace_f_fn fn,
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
      memcpy(out, in, len*sizeof(*out));
      fn(out, len);
      consume32(&out[chunk - 1]);
    }
    uint64_t t2 = clock_nsec();
    total += t2 - t1;
    calls += reps;
    t = clock_nsec();
  }
  return (calls > 0) ? total/((double)calls*chunk) : 0;
}

static void print_inplace_bench_d(int chunk, double bench_time, double in[],
                                  double out[], randompack_rng *rng,
                                  inplace_d_spec specs[], int n) {
  for (int i = 0; i < n; i++) {
    double ns = time_fn_inplace_d(chunk, bench_time, specs[i].fn, in, out, rng);
    printf("%-22s %10.2f\n", specs[i].name, ns);
  }
}

static void print_inplace_bench_f(int chunk, double bench_time, float in[],
                                  float out[], randompack_rng *rng,
                                  inplace_f_spec specs[], int n) {
  for (int i = 0; i < n; i++) {
    double ns = time_fn_inplace_f(chunk, bench_time, specs[i].fn, in, out, rng);
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
  return smoke_arr_d(sleef_avx512_exp_adapter, exp, openlibm_exp, tests, LEN(tests)) &&
      smoke_arr_d(sleef_avx512_log_adapter, log, openlibm_log, tests, LEN(tests));
}

static void print_arr_bench_d(int chunk, double bench_time, double in[],
                              double out[], randompack_rng *rng,
                              arr_d_spec specs[], int n) {
  (void)in;
  for (int i = 0; i < n; i++) {
    double ns = time_fn_arr_d(chunk, bench_time, specs[i].fn, out, rng);
    printf("%-22s %10.2f\n", specs[i].name, ns);
  }
}
#endif

int main(int argc, char **argv) {
  char *engine;
  double bench_time;
  int chunk, seed;
  bool help;
#if defined(BUILD_AVX512) && (defined(__x86_64__) || defined(_M_X64))
  bool has_avx512 = cpu_has_avx512_local();
#endif
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
  if (!check_chunk(chunk)) {
    fprintf(stderr, "%s\n", chunk_msg());
    randompack_free(rng);
    return 1;
  }
#if defined(BUILD_AVX512)
  if (has_avx512 && !run_avx512_smoke_tests()) {
    fprintf(stderr, "AVX-512 smoke test failed\n");
    randompack_free(rng);
    return 1;
  }
#endif
  double *in = 0;
  double *out = 0;
  float *inf = 0;
  float *outf = 0;
  enum { SINGLE_LOG_N = 4096 };
  double single_log_in[SINGLE_LOG_N];
  float single_log_inf[SINGLE_LOG_N];
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
  fill_linspace_d(single_log_in, SINGLE_LOG_N, 0.5, 5.0);
  fill_linspace_f(single_log_inf, SINGLE_LOG_N, 0.5f, 5.0f);
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
    { "sleef_avx512_exp", sleef_avx512_exp_adapter },
  };
  arr_d_spec avx512_logs[] = {
    { "sleef_avx512_u35log", sleef_avx512_log_adapter },
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
#if defined(BUILD_AVX2) || (defined(__aarch64__) || defined(_M_ARM64)) && !defined(__APPLE__)
  print_inplace_bench_d(chunk, bench_time, in, out, rng, inplace_exps,
    LEN(inplace_exps));
#endif
#if defined(BUILD_AVX512)
  if (has_avx512)
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
#if defined(BUILD_AVX2) || (defined(__aarch64__) || defined(_M_ARM64)) && !defined(__APPLE__)
  print_inplace_bench_d(chunk, bench_time, in, out, rng, inplace_logs,
    LEN(inplace_logs));
#endif
#if defined(BUILD_AVX512)
  if (has_avx512)
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
#if defined(BUILD_AVX2) || (defined(__aarch64__) || defined(_M_ARM64)) && !defined(__APPLE__)
  print_inplace_bench_f(chunk, bench_time, inf, outf, rng, inplace_expfs,
    LEN(inplace_expfs));
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
#if defined(BUILD_AVX2) || (defined(__aarch64__) || defined(_M_ARM64)) && !defined(__APPLE__)
  print_inplace_bench_f(chunk, bench_time, inf, outf, rng, inplace_logfs,
    LEN(inplace_logfs));
#endif
  printf("\nsingle scalar logs: ns/call\n");
  printf("grid:             linspace(0.5, 5.0, %d)\n\n", SINGLE_LOG_N);
  printf("%-22s %10s\n", "Function", "double");
  ns = time_single_log_d(single_log_in, SINGLE_LOG_N, bench_time, openlibm_log);
  printf("%-22s %10.2f\n", "openlibm_log", ns);
  ns = time_single_log_d(single_log_in, SINGLE_LOG_N, bench_time, log);
  printf("%-22s %10.2f\n", "log", ns);
  printf("\n%-22s %10s\n", "Function", "float");
  nsf = time_single_log_f(single_log_inf, SINGLE_LOG_N, bench_time, openlibm_logf);
  printf("%-22s %10.2f\n", "openlibm_logf", nsf);
  nsf = time_single_log_f(single_log_inf, SINGLE_LOG_N, bench_time, logf);
  printf("%-22s %10.2f\n", "logf", nsf);
  FREE(in);
  FREE(out);
  FREE(inf);
  FREE(outf);
  randompack_free(rng);
  return 0;
}
