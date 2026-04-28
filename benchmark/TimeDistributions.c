// -*- C -*-
// TimeDistributions.c: time distributions (ns/value), double and float.

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "getopt.h"

#include "TimeUtil.h"
#include "Util.h"
#include "randompack.h"
#include "randompack_config.h"
#include "printX.h"

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
  printf("TimeDistributions — time distributions (ns/value), double and float\n");
  printf("Usage: TimeDistributions [options]\n\n");
  printf("Options:\n");
  printf("  -h            Show this help message\n");
  printf("  -e engine     RNG engine (default x256++simd)\n");
  printf("  -t seconds    Benchmark time per distribution (default 0.2)\n");
  printf("  -w seconds    CPU warmup time before timing (default 0.1)\n");
  printf("  -c chunk      Chunk size (values per call, default 4096)\n");
  printf("  -s seed       Fixed RNG seed (default random seed per distribution)\n");
  printf("  -d digits     Decimal places for ns output (default 2)\n");
  printf("  -b            Use bitexact log/exp implementations\n\n");
  printf("Engines:\n");
  print_engines();
  printf("\nNotes:\n");
  printf("  Parameters are fixed to representative values.\n");
}

static bool get_options(int argc, char **argv, char **engine, double *bench_time,
                        double *warmup_time, int *chunk, int *seed,
                        bool *have_seed, int *digits, bool *bitexact,
                        bool *help) {
  opterr = 0;
  optind = 1;
  int opt;
  *engine = "x256++simd";
  *bench_time = 0.2;
  *warmup_time = 0.1;
  *chunk = 4096;
  *seed = 0;
  *have_seed = false;
  *digits = 2;
  *bitexact = false;
  *help = false;
  while ((opt = getopt(argc, argv, "he:t:w:c:s:d:b")) != -1) {
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
      case 'w':
        *warmup_time = atof(optarg);
        if (*warmup_time < 0)
          return false;
        break;
      case 'c':
        *chunk = atoi(optarg);
        if (*chunk <= 0)
          return false;
        break;
      case 's':
        *seed = atoi(optarg);
        *have_seed = true;
        break;
      case 'd':
        *digits = atoi(optarg);
        if (*digits < 0)
          return false;
        break;
      case 'b':
        *bitexact = true;
        break;
      default:
        return false;
    }
  }
  if (optind < argc)
    return false;
  return true;
}

typedef enum {
  U01,
  UNIF,
  NORM,
  NORMAL,
  LOGNORMAL,
  SKEW_NORMAL,
  GUMBEL,
  PARETO,
  EXP1,
  EXP2,
  GAMMA,
  CHI2,
  BETA,
  T,
  F,
  WEIBULL
} dist_id;

typedef struct {
  dist_id id;
  char *name;
  int nparam;
  double param[3];
} dist_spec;

static bool fill_dist(double out[], int n, double param[], randompack_rng *rng) {
  size_t len = (size_t)n;
  switch ((dist_id)(int)param[0]) {
    case U01:       return randompack_u01      (out, len,                     rng);
    case UNIF:      return randompack_unif     (out, len, param[1], param[2], rng);
    case NORM:      return randompack_norm     (out, len,                     rng);
    case NORMAL:    return randompack_normal   (out, len, param[1], param[2], rng);
    case LOGNORMAL: return randompack_lognormal(out, len, param[1], param[2], rng);
    case SKEW_NORMAL:
      return randompack_skew_normal(out, len, param[1], param[2], param[3], rng);
    case GUMBEL:    return randompack_gumbel   (out, len, param[1], param[2], rng);
    case PARETO:    return randompack_pareto   (out, len, param[1], param[2], rng);
    case EXP1:      return randompack_exp      (out, len, param[1],           rng);
    case EXP2:      return randompack_exp      (out, len, param[1],           rng);
    case GAMMA:     return randompack_gamma    (out, len, param[1], param[2], rng);
    case CHI2:      return randompack_chi2     (out, len, param[1],           rng);
    case BETA:      return randompack_beta     (out, len, param[1], param[2], rng);
    case T:         return randompack_t        (out, len, param[1],           rng);
    case F:         return randompack_f        (out, len, param[1], param[2], rng);
    case WEIBULL:   return randompack_weibull  (out, len, param[1], param[2], rng);
  }
  return false;
}

static void fill_wrapper(double out[], int n, double param[], randompack_rng *rng) {
  ASSERT(fill_dist(out, n, param, rng));
}

static bool fill_distf(float out[], int n, float param[], randompack_rng *rng) {
  size_t len = (size_t)n;
  switch ((dist_id)(int)param[0]) {
    case U01:       return randompack_u01f      (out, len,                     rng);
    case UNIF:      return randompack_uniff     (out, len, param[1], param[2], rng);
    case NORM:      return randompack_normf     (out, len,                     rng);
    case NORMAL:    return randompack_normalf   (out, len, param[1], param[2], rng);
    case LOGNORMAL: return randompack_lognormalf(out, len, param[1], param[2], rng);
    case SKEW_NORMAL:
      return randompack_skew_normalf(out, len, param[1], param[2], param[3], rng);
    case GUMBEL:    return randompack_gumbelf   (out, len, param[1], param[2], rng);
    case PARETO:    return randompack_paretof   (out, len, param[1], param[2], rng);
    case EXP1:      return randompack_expf      (out, len, param[1],           rng);
    case EXP2:      return randompack_expf      (out, len, param[1],           rng);
    case GAMMA:     return randompack_gammaf    (out, len, param[1], param[2], rng);
    case CHI2:      return randompack_chi2f     (out, len, param[1],           rng);
    case BETA:      return randompack_betaf     (out, len, param[1], param[2], rng);
    case T:         return randompack_tf        (out, len, param[1],           rng);
    case F:         return randompack_ff        (out, len, param[1], param[2], rng);
    case WEIBULL:   return randompack_weibullf  (out, len, param[1], param[2], rng);
  }
  return false;
}

static void fill_wrapperf(float out[], int n, float param[], randompack_rng *rng) {
  ASSERT(fill_distf(out, n, param, rng));
}

static void set_seed(randompack_rng *rng, int seed, bool have_seed, bool bitexact) {
  bool ok;
  if (have_seed)
    ok = randompack_seed(seed, 0, 0, rng);
  else
    ok = randompack_randomize(rng);
  ASSERT(ok);
  if (bitexact) {
    ok = randompack_bitexact(rng, true);
    ASSERT(ok);
  }
}

int main(int argc, char **argv) {
  char *engine;
  double bench_time;
  double warmup_time;
  int chunk, seed, digits;
  bool have_seed;
  bool bitexact;
  bool help;
  if (!get_options(argc, argv, &engine, &bench_time, &warmup_time, &chunk,
      &seed, &have_seed, &digits, &bitexact, &help) || help) {
    print_help();
    return help ? 0 : 1;
  }
#if defined(__linux__)
#endif
  randompack_rng *rngd = randompack_create(engine);
  randompack_rng *rngf = randompack_create(engine);
  randompack_rng *rngd_fast = bitexact ? randompack_create(engine) : 0;
  if (!rngd || !rngf || (bitexact && !rngd_fast)) {
    fprintf(stderr, "randompack_create failed: %s\n", engine);
    if (rngd)
      randompack_free(rngd);
    if (rngf)
      randompack_free(rngf);
    if (rngd_fast)
      randompack_free(rngd_fast);
    return 1;
  }
  if (bitexact) {
    if (!randompack_bitexact(rngd, true) ||
        !randompack_bitexact(rngf, true)) {
      fprintf(stderr, "randompack_bitexact failed\n");
      randompack_free(rngd);
      randompack_free(rngf);
      randompack_free(rngd_fast);
      return 1;
    }
  }
  warmup_cpu(warmup_time);
  dist_spec dists[] = {
    { U01,       "u01",            0, { 0, 0} },
    { UNIF,      "unif(2,5)",      2, { 2, 5} },
    { NORM,      "norm",           0, { 0, 0} },
    { NORMAL,    "normal(2,3)",    2, { 2, 3} },
    { EXP1,      "exp(1)",         1, { 1, 0} },
    { EXP2,      "exp(2)",         1, { 2, 0} },
    { LOGNORMAL, "lognormal(0,1)", 2, { 0, 1} },
    { SKEW_NORMAL, "skew-normal(0,1,5)", 3, { 0, 1, 5 } },
    { GUMBEL,    "gumbel(0,1)",    2, { 0, 1} },
    { PARETO,    "pareto(1,2)",    2, { 1, 2} },
    { GAMMA,     "gamma(2,3)",     2, { 2, 3} },
    { GAMMA,     "gamma(0.5,2)",   2, { 0.5, 2} },
    { BETA,      "beta(2,5)",      2, { 2, 5} },
    { CHI2,      "chi2(5)",        1, { 5, 0} },
    { T,         "t(10)",          1, {10, 0} },
    { F,         "F(5,10)",        2, { 5,10} },
    { WEIBULL,   "weibull(2,3)",   2, { 2, 3} },
    { WEIBULL,   "weibull(3,4)",   2, { 3, 4} },
  };
  printf("engine:           %s\n", engine);
  printf("time per value:   ns/value\n");
  printf("warmup_time:      %.3f s\n", warmup_time);
  printf("bench_time:       %.3f s per distribution\n", bench_time);
  printf("chunk:            %d\n\n", chunk);
  if (bitexact)
    printf("%-18s %8s %8s %7s %8s\n",
      "Distribution", "bitexact", "fast", "ratio", "float");
  else
    printf("%-18s %8s %8s\n", "Distribution", "double", "float");
  for (int i = 0; i < LEN(dists); i++) {
    double par[4];
    par[0] = (double)dists[i].id;
    par[1] = dists[i].param[0];
    par[2] = dists[i].param[1];
    par[3] = dists[i].param[2];
    float parf[4];
    parf[0] = (float)dists[i].id;
    parf[1] = (float)dists[i].param[0];
    parf[2] = (float)dists[i].param[1];
    parf[3] = (float)dists[i].param[2];
    double x[4];
    float xf[4];
    set_seed(rngd, seed, have_seed, bitexact);
    set_seed(rngf, seed, have_seed, bitexact);
    fill_dist(x, 4, par, rngd);
    fill_distf(xf, 4, parf, rngf);
    set_seed(rngd, seed, have_seed, bitexact);
    set_seed(rngf, seed, have_seed, bitexact);
    double nsd = time_double(chunk, bench_time, fill_wrapper, par, rngd);
    double nsf = time_float(chunk, bench_time, fill_wrapperf, parf, rngf);
    double nsd_fast = 0;
    if (bitexact) {
      set_seed(rngd_fast, seed, have_seed, false);
      fill_dist(x, 4, par, rngd_fast);
      set_seed(rngd_fast, seed, have_seed, false);
      nsd_fast = time_double(chunk, bench_time, fill_wrapper, par, rngd_fast);
    }
    printf("%-18s", dists[i].name);
    if (bitexact)
      printf(" %8.*f %8.*f %7.2f %8.*f\n",
        digits, nsd, digits, nsd_fast, nsd_fast > 0 ? nsd/nsd_fast : 0,
        digits, nsf);
    else
      printf(" %8.*f %8.*f\n", digits, nsd, digits, nsf);
  }
  randompack_free(rngd);
  randompack_free(rngf);
  if (rngd_fast)
    randompack_free(rngd_fast);
  return 0;
}
