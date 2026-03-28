// -*- C -*-
// TimeDistributionsMkl.c: time MKL VSL distributions (ns/value), double and float.
// Subset only: uniform, gaussian, exponential, gamma, beta, chi-square, weibull.
//
// Build:
//   icx -O3 TimeDistributionsMkl.c -qmkl
//
// Notes:
// - Uses MKL default method (method=0) for each distribution.
// - Uses sequential MKL via -qmkl (you can force sequential at runtime too).
//
// Optional (pin to 1 thread):
//   export MKL_NUM_THREADS=1
//   export OMP_NUM_THREADS=1

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "getopt.h"
#include "mkl.h"
#include "mkl_vsl.h"

static uint64_t now_ns(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (uint64_t)ts.tv_sec*1000000000ULL+(uint64_t)ts.tv_nsec;
}

static void warmup_cpu(int iters) {
  volatile double x = 0;
  for (int i=0; i < iters; i++) x += (double)i*0.5;
  if (x == 1234567.0) fprintf(stderr, "warmup: %.1f\n", x);
}

static int parse_brng(const char *s) {
  if (!s) return VSL_BRNG_MT19937;
  if (!strcmp(s, "mt19937")) return VSL_BRNG_MT19937;
#ifdef VSL_BRNG_MRG32K3A
  if (!strcmp(s, "mrg32k3a")) return VSL_BRNG_MRG32K3A;
#endif
#ifdef VSL_BRNG_SFMT19937
  if (!strcmp(s, "sfmt19937")) return VSL_BRNG_SFMT19937;
#endif
#ifdef VSL_BRNG_PHILOX4X32X10
  if (!strcmp(s, "philox4x32x10")) return VSL_BRNG_PHILOX4X32X10;
#endif
  return -1;
}

static void print_engines(void) {
  printf("  mt19937\n");
#ifdef VSL_BRNG_MRG32K3A
  printf("  mrg32k3a\n");
#endif
#ifdef VSL_BRNG_SFMT19937
  printf("  sfmt19937\n");
#endif
#ifdef VSL_BRNG_PHILOX4X32X10
  printf("  philox4x32x10\n");
#endif
}

static void print_help(void) {
  printf("TimeDistributionsMkl — time MKL VSL distributions (ns/value), double and float\n");
  printf("Usage: TimeDistributionsMkl [options]\n\n");
  printf("Options:\n");
  printf("  -h            Show this help message\n");
  printf("  -e engine     MKL BRNG (default mt19937)\n");
  printf("  -t seconds    Benchmark time per distribution (default 0.1)\n");
  printf("  -c chunk      Chunk size (values per call, default 4096)\n");
  printf("  -s seed       RNG seed (default 7)\n\n");
  printf("Engines:\n");
  print_engines();
}

static bool get_options(int argc, char **argv, const char **engine, double *bench_time,
                        int *chunk, int *seed, bool *help) {
  opterr = 0;
  optind = 1;
  int opt;
  *engine = "mt19937";
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
        if (*bench_time <= 0) return false;
        break;
      case 'c':
        *chunk = atoi(optarg);
        if (*chunk <= 0) return false;
        break;
      case 's':
        *seed = atoi(optarg);
        break;
      default:
        return false;
    }
  }
  if (optind < argc) return false;
  return true;
}

typedef enum {
  U01,
  UNIF,
  NORM,
  NORMAL,
  EXP1,
  EXP2,
  GAMMA,
  CHI2,
  BETA,
  WEIBULL
} dist_id;

typedef struct {
  dist_id id;
  const char *name;
  double p0;
  double p1;
} dist_spec;

static bool fill_dist(double out[], int n, const dist_spec *d, VSLStreamStatePtr s) {
  int method = 0;
  switch (d->id) {
    case U01:     return vdRngUniform    (method, s, n, out, 0.0, 1.0) == VSL_STATUS_OK;
    case UNIF:    return vdRngUniform    (method, s, n, out, d->p0, d->p1) == VSL_STATUS_OK;
    case NORM:    return vdRngGaussian   (method, s, n, out, 0.0, 1.0) == VSL_STATUS_OK;
    case NORMAL:  return vdRngGaussian   (method, s, n, out, d->p0, d->p1) == VSL_STATUS_OK;
    case EXP1:    return vdRngExponential(method, s, n, out, 0.0, d->p0) == VSL_STATUS_OK;
    case EXP2:    return vdRngExponential(method, s, n, out, 0.0, d->p0) == VSL_STATUS_OK;
    case CHI2:    return vdRngChiSquare  (method, s, n, out, d->p0) == VSL_STATUS_OK;
    case GAMMA:   return vdRngGamma  (method, s, n, out, d->p0, 0.0, d->p1) == VSL_STATUS_OK;
    case BETA:    return vdRngBeta   (method, s, n, out, d->p0, d->p1, 0.0, 1.0) == VSL_STATUS_OK;
    case WEIBULL: return vdRngWeibull(method, s, n, out, d->p0, 0.0, d->p1) == VSL_STATUS_OK;  }
  return false;
}

static bool fill_distf(float out[], int n, const dist_spec *d, VSLStreamStatePtr s) {
  int method = 0;
  switch (d->id) {
    case U01:     return vsRngUniform    (method, s, n, out, 0.0f, 1.0f) == VSL_STATUS_OK;
    case UNIF:    return vsRngUniform    (method, s, n, out, (float)d->p0, (float)d->p1) == VSL_STATUS_OK;
    case NORM:    return vsRngGaussian   (method, s, n, out, 0.0f, 1.0f) == VSL_STATUS_OK;
    case NORMAL:  return vsRngGaussian   (method, s, n, out, (float)d->p0, (float)d->p1) == VSL_STATUS_OK;
    case EXP1:    return vsRngExponential(method, s, n, out, 0.0f, (float)d->p0) == VSL_STATUS_OK;
    case EXP2:    return vsRngExponential(method, s, n, out, 0.0f, (float)d->p0) == VSL_STATUS_OK;
    case CHI2:    return vsRngChiSquare  (method, s, n, out, (float)d->p0) == VSL_STATUS_OK;
    case GAMMA:   return vsRngGamma  (method, s, n, out, (float)d->p0, 0.0f, (float)d->p1) == VSL_STATUS_OK;
    case BETA:    return vsRngBeta   (method, s, n, out, (float)d->p0, (float)d->p1, 0.0f, 1.0f) == VSL_STATUS_OK;
    case WEIBULL: return vsRngWeibull(method, s, n, out, (float)d->p0, 0.0f, (float)d->p1) == VSL_STATUS_OK;  }
  return false;
}

static double time_double(int chunk, double bench_time, const dist_spec *d, VSLStreamStatePtr s) {
  double *buf = (double *)malloc((size_t)chunk*sizeof(double));
  if (!buf) return -1;
  volatile double sink = 0;
  uint64_t t0 = now_ns();
  uint64_t t1 = t0;
  int iters = 0;
  while ((double)(t1-t0)*1e-9 < bench_time) {
    if (!fill_dist(buf, chunk, d, s)) {
      free(buf);
      return -1;
    }
    sink += buf[iters & (chunk-1)];
    iters++;
    t1 = now_ns();
  }
  if (sink == 1234567.0) fprintf(stderr, "sink: %.1f\n", sink);
  uint64_t dt = t1-t0;
  free(buf);
  return (double)dt/(double)(iters*(uint64_t)chunk);
}

static double time_float(int chunk, double bench_time, const dist_spec *d, VSLStreamStatePtr s) {
  float *buf = (float *)malloc((size_t)chunk*sizeof(float));
  if (!buf) return -1;
  volatile float sink = 0;
  uint64_t t0 = now_ns();
  uint64_t t1 = t0;
  int iters = 0;
  while ((double)(t1-t0)*1e-9 < bench_time) {
    if (!fill_distf(buf, chunk, d, s)) {
      free(buf);
      return -1;
    }
    sink += buf[iters & (chunk-1)];
    iters++;
    t1 = now_ns();
  }
  if (sink == 1234567.0f) fprintf(stderr, "sink: %.1f\n", (double)sink);
  uint64_t dt = t1-t0;
  free(buf);
  return (double)dt/(double)(iters*(uint64_t)chunk);
}

int main(int argc, char **argv) {
  const char *engine;
  double bench_time;
  int chunk, seed;
  bool help;
  if (!get_options(argc, argv, &engine, &bench_time, &chunk, &seed, &help) || help) {
    print_help();
    return help ? 0 : 1;
  }
  if ((chunk & (chunk-1)) != 0) {
    fprintf(stderr, "chunk must be a power of two (for the tiny anti-opt sink)\n");
    return 1;
  }
  int brng = parse_brng(engine);
  if (brng < 0) {
    fprintf(stderr, "unknown MKL engine: %s\n", engine);
    return 1;
  }
  VSLStreamStatePtr sd = 0;
  VSLStreamStatePtr sf = 0;
  if (vslNewStream(&sd, brng, (unsigned)seed) != VSL_STATUS_OK ||
      vslNewStream(&sf, brng, (unsigned)seed) != VSL_STATUS_OK) {
    fprintf(stderr, "vslNewStream failed\n");
    if (sd) vslDeleteStream(&sd);
    if (sf) vslDeleteStream(&sf);
    return 1;
  }
  warmup_cpu(1000000);
  dist_spec dists[] = {
    { U01,     "u01",         0, 0 },
    { UNIF,    "unif(2,5)",   2, 5 },
    { NORM,    "norm",        0, 0 },
    { NORMAL,  "normal(2,3)", 2, 3 },
    { EXP1,    "exp(1)",      1, 0 },
    { EXP2,    "exp(2)",      2, 0 },
    { GAMMA,   "gamma(2,3)",  2, 3 },
    { GAMMA,   "gamma(0.5,2)",  0.5, 2 },
    { BETA,    "beta(2,5)",   2, 5 },
    { CHI2,    "chi2(5)",     5, 0 },
    { WEIBULL, "weibull(2,3)",2, 3 },
  };
  printf("engine:           %s\n", engine);
  printf("time per value:   ns/value\n");
  printf("bench_time:       %.3f s per distribution\n", bench_time);
  printf("chunk:            %d\n\n", chunk);
  printf("%-18s %9s %9s\n", "Distribution", "double", "float");
  for (int i=0; i < (int)(sizeof(dists)/sizeof(dists[0])); i++) {
    double nsd = time_double(chunk, bench_time, &dists[i], sd);
    double nsf = time_float (chunk, bench_time, &dists[i], sf);
    if (nsd < 0 || nsf < 0) {
      fprintf(stderr, "timing failed for %s\n", dists[i].name);
      vslDeleteStream(&sd);
      vslDeleteStream(&sf);
      return 1;
    }
    printf("%-18s %9.3f %9.2f\n", dists[i].name, nsd, nsf);
  }
  vslDeleteStream(&sd);
  vslDeleteStream(&sf);
  return 0;
}
