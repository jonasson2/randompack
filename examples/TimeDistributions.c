// -*- C -*-
// TimeDistributions.c: time distributions (ns/value), double and float.

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "TimeUtil.h"
#include "Util.h"
#include "randompack.h"
#include "randompack_config.h"
#include "printX.h"

static void print_help(void) {
  printf("TimeDistributions — time distributions (ns/value), double and float\n");
  printf("Usage: TimeDistributions [options]\n\n");
  printf("Options:\n");
  printf("  -h            Show this help message\n");
  printf("  -e engine     RNG engine (default x256++)\n");
  printf("  -t seconds    Benchmark time per distribution (default 0.1)\n");
  printf("  -c chunk      Chunk size (values per call, default 1024)\n");
  printf("  -s seed       RNG seed (default 7)\n\n");
  printf("Notes:\n");
  printf("  Parameters are fixed to representative values.\n");
}

static bool get_options(int argc, char **argv, char **engine, double *bench_time,
                        int *chunk, int *seed, bool *help) {
  opterr = 0;
  optind = 1;
  int opt;
  *engine = "x256++";
  *bench_time = 0.1;
  *chunk = 1024;
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

typedef enum {
  U01,
  UNIF,
  NORM,
  NORMAL,
  LOGNORMAL,
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
  double param[2];
} dist_spec;

static bool fill_dist(double out[], int n, double param[], randompack_rng *rng) {
  size_t len = (size_t)n;
  switch ((dist_id)(int)param[0]) {
    case U01:       return randompack_u01      (out, len,                     rng);
    case UNIF:      return randompack_unif     (out, len, param[1], param[2], rng);
    case NORM:      return randompack_norm     (out, len,                     rng);
    case NORMAL:    return randompack_normal   (out, len, param[1], param[2], rng);
    case LOGNORMAL: return randompack_lognormal(out, len, param[1], param[2], rng);
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

int main(int argc, char **argv) {
  char *engine;
  double bench_time;
  int chunk, seed;
  bool help;
  if (!get_options(argc, argv, &engine, &bench_time, &chunk, &seed, &help) || help) {
    print_help();
    return help ? 0 : 1;
  }
  randompack_rng *rngd = randompack_create(engine);
  randompack_rng *rngf = randompack_create(engine);
  if (!rngd || !rngf) {
    fprintf(stderr, "randompack_create failed: %s\n", engine);
    if (rngd)
      randompack_free(rngd);
    if (rngf)
      randompack_free(rngf);
    return 1;
  }
  if (!randompack_seed(seed, 0, 0, rngd) || !randompack_seed(seed, 0, 0, rngf)) {
    fprintf(stderr, "randompack_seed failed\n");
    randompack_free(rngd);
    randompack_free(rngf);
    return 1;
  }
  warmup_cpu(100);
  dist_spec dists[] = {
    { U01,       "u01",            0, { 0, 0} },
    { UNIF,      "unif(2,5)",      2, { 2, 5} },
    { NORM,      "norm",           0, { 0, 0} },
    { NORMAL,    "normal(2,3)",    2, { 2, 3} },
    { LOGNORMAL, "lognormal(0,1)", 2, { 0, 1} },
    { GUMBEL,    "gumbel(0,1)",    2, { 0, 1} },
    { PARETO,    "pareto(1,2)",    2, { 1, 2} },
    { EXP1,      "exp(1)",         1, { 1, 0} },
    { EXP2,      "exp(2)",         1, { 2, 0} },
    { GAMMA,     "gamma(2,3)",     2, { 2, 3} },
    { CHI2,      "chi2(5)",        1, { 5, 0} },
    { BETA,      "beta(2,5)",      2, { 2, 5} },
    { T,         "t(10)",          1, {10, 0} },
    { F,         "F(5,10)",        2, { 5,10} },
    { WEIBULL,   "weibull(2,3)",   2, { 2, 3} },
  };
  printf("engine:           %s\n", engine);
  printf("time per value:   ns/value\n");
  printf("bench_time:       %.3f s per distribution\n", bench_time);
  printf("chunk:            %d\n\n", chunk);
  printf("%-18s %9s %9s\n", "Distribution", "double", "float");
  for (int i = 0; i < LEN(dists); i++) {
    double par[3];
    par[0] = (double)dists[i].id;
    par[1] = dists[i].param[0];
    par[2] = dists[i].param[1];
    float parf[3];
    parf[0] = (float)dists[i].id;
    parf[1] = (float)dists[i].param[0];
    parf[2] = (float)dists[i].param[1];
    double x[4];
    float xf[4];
    fill_dist(x, 4, par, rngd);
    fill_distf(xf, 4, parf, rngf);
    double nsd = time_double(chunk, bench_time, fill_wrapper, par, rngd);
    double nsf = time_float(chunk, bench_time, fill_wrapperf, parf, rngf);
    printf("%-18s", dists[i].name);
    printf(" %9.2f %9.2f\n", nsd, nsf);
  }
  randompack_free(rngd);
  randompack_free(rngf);
  return 0;
}
