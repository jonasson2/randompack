// -*- C -*-
#include <ctype.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "getopt.h"
#include "randompack.h"

typedef enum {
  DIST_U01,
  DIST_UNIF,
  DIST_NORM,
  DIST_NORMAL,
  DIST_EXP,
  DIST_LOGNORMAL,
  DIST_GAMMA,
  DIST_BETA,
  DIST_CHI2,
  DIST_T,
  DIST_F,
  DIST_GUMBEL,
  DIST_PARETO,
  DIST_WEIBULL,
  DIST_SKEW_NORMAL,
} dist_t;

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
  for (int i = 0; i < n; i++) printf("  %-*s  %s\n", width, names + i*elen, descs + i*dlen);
  free(names);
  free(descs);
}

static void print_help(void) {
  printf("TestBitexact - summarize deterministic bitexact draws\n");
  printf("Usage: TestBitexact [options]\n\n");
  printf("Options:\n");
  printf("  -h            Show this help message\n");
  printf("  -x            Do not set bitexact mode\n");
  printf("  -s SEED       Seed (default 123)\n");
  printf("  -n N          Number of draws (default 1000000000)\n");
  printf("  -d DIST       Distribution (default N(0,1))\n");
  printf("  -p LIST       Parameters (default depends on distribution)\n");
  printf("  -e ENGINE     RNG engine (default x256++simd)\n");
  printf("  -P PREC       Precision: double or float (default double)\n\n");
  printf("Engines:\n");
  print_engines();
  printf("\n");
  printf("Distributions:\n");
  printf("  u01\n");
  printf("  unif         -p a,b\n");
  printf("  norm         (or N(0,1))\n");
  printf("  normal       -p mu,sigma\n");
  printf("  exp          -p scale\n");
  printf("  lognormal    -p mu,sigma\n");
  printf("  gamma        -p shape,scale\n");
  printf("  beta         -p a,b\n");
  printf("  chi2         -p nu\n");
  printf("  t            -p nu\n");
  printf("  f            -p nu1,nu2\n");
  printf("  gumbel       -p mu,beta\n");
  printf("  pareto       -p xm,alpha\n");
  printf("  weibull      -p shape,scale\n");
  printf("  skew_normal  -p mu,sigma,alpha\n");
}

static bool streq_ci(const char *a, const char *b) {
  if (!a || !b) return false;
  while (*a && *b) {
    int ca = tolower((unsigned char)*a++);
    int cb = tolower((unsigned char)*b++);
    if (ca != cb) return false;
  }
  return *a == 0 && *b == 0;
}

static bool parse_u64(const char *s, uint64_t *out) {
  if (!s || !*s) return false;
  char *end = 0;
  double v = strtod(s, &end);
  if (!end || *end || v < 0 || v > (double)UINT64_MAX) return false;
  uint64_t n = (uint64_t)v;
  if ((double)n != v) return false;
  *out = n;
  return true;
}

static bool parse_double_list(const char *s, double *p, int want) {
  if (want == 0) return s == 0 || *s == 0;
  if (!s) return false;
  int n = 1;
  for (const char *q = s; *q; q++) if (*q == ',') n++;
  if (n != want) return false;
  const char *t = s;
  for (int i = 0; i < want; i++) {
    char buf[64];
    int k = 0;
    while (*t && *t != ',' && k < (int)sizeof(buf) - 1) buf[k++] = *t++;
    buf[k] = 0;
    if (*t == ',') t++;
    char *end = 0;
    double v = strtod(buf, &end);
    if (!end || *end) return false;
    p[i] = v;
  }
  return true;
}

static bool parse_precision(const char *s, bool *use_float) {
  if (streq_ci(s, "double")) {
    *use_float = false;
    return true;
  }
  if (streq_ci(s, "float")) {
    *use_float = true;
    return true;
  }
  return false;
}

static bool parse_dist(const char *s, dist_t *dist, int *npar) {
  if (streq_ci(s, "u01")) {
    *dist = DIST_U01;
    *npar = 0;
  }
  else if (streq_ci(s, "unif")) {
    *dist = DIST_UNIF;
    *npar = 2;
  }
  else if (streq_ci(s, "norm") || streq_ci(s, "n(0,1)")) {
    *dist = DIST_NORM;
    *npar = 0;
  }
  else if (streq_ci(s, "normal")) {
    *dist = DIST_NORMAL;
    *npar = 2;
  }
  else if (streq_ci(s, "exp")) {
    *dist = DIST_EXP;
    *npar = 1;
  }
  else if (streq_ci(s, "lognormal")) {
    *dist = DIST_LOGNORMAL;
    *npar = 2;
  }
  else if (streq_ci(s, "gamma")) {
    *dist = DIST_GAMMA;
    *npar = 2;
  }
  else if (streq_ci(s, "beta")) {
    *dist = DIST_BETA;
    *npar = 2;
  }
  else if (streq_ci(s, "chi2")) {
    *dist = DIST_CHI2;
    *npar = 1;
  }
  else if (streq_ci(s, "t")) {
    *dist = DIST_T;
    *npar = 1;
  }
  else if (streq_ci(s, "f")) {
    *dist = DIST_F;
    *npar = 2;
  }
  else if (streq_ci(s, "gumbel")) {
    *dist = DIST_GUMBEL;
    *npar = 2;
  }
  else if (streq_ci(s, "pareto")) {
    *dist = DIST_PARETO;
    *npar = 2;
  }
  else if (streq_ci(s, "weibull")) {
    *dist = DIST_WEIBULL;
    *npar = 2;
  }
  else if (streq_ci(s, "skew_normal")) {
    *dist = DIST_SKEW_NORMAL;
    *npar = 3;
  }
  else {
    return false;
  }
  return true;
}

static bool fill_double(double *x, size_t len, dist_t dist, double *p, randompack_rng *rng) {
  switch (dist) {
    case DIST_U01: return randompack_u01(x, len, rng);
    case DIST_UNIF: return randompack_unif(x, len, p[0], p[1], rng);
    case DIST_NORM: return randompack_norm(x, len, rng);
    case DIST_NORMAL: return randompack_normal(x, len, p[0], p[1], rng);
    case DIST_EXP: return randompack_exp(x, len, p[0], rng);
    case DIST_LOGNORMAL: return randompack_lognormal(x, len, p[0], p[1], rng);
    case DIST_GAMMA: return randompack_gamma(x, len, p[0], p[1], rng);
    case DIST_BETA: return randompack_beta(x, len, p[0], p[1], rng);
    case DIST_CHI2: return randompack_chi2(x, len, p[0], rng);
    case DIST_T: return randompack_t(x, len, p[0], rng);
    case DIST_F: return randompack_f(x, len, p[0], p[1], rng);
    case DIST_GUMBEL: return randompack_gumbel(x, len, p[0], p[1], rng);
    case DIST_PARETO: return randompack_pareto(x, len, p[0], p[1], rng);
    case DIST_WEIBULL: return randompack_weibull(x, len, p[0], p[1], rng);
    case DIST_SKEW_NORMAL: return randompack_skew_normal(x, len, p[0], p[1], p[2], rng);
  }
  return false;
}

static bool fill_float(float *x, size_t len, dist_t dist, double *p, randompack_rng *rng) {
  switch (dist) {
    case DIST_U01: return randompack_u01f(x, len, rng);
    case DIST_UNIF: return randompack_uniff(x, len, (float)p[0], (float)p[1], rng);
    case DIST_NORM: return randompack_normf(x, len, rng);
    case DIST_NORMAL: return randompack_normalf(x, len, (float)p[0], (float)p[1], rng);
    case DIST_EXP: return randompack_expf(x, len, (float)p[0], rng);
    case DIST_LOGNORMAL: return randompack_lognormalf(x, len, (float)p[0], (float)p[1], rng);
    case DIST_GAMMA: return randompack_gammaf(x, len, (float)p[0], (float)p[1], rng);
    case DIST_BETA: return randompack_betaf(x, len, (float)p[0], (float)p[1], rng);
    case DIST_CHI2: return randompack_chi2f(x, len, (float)p[0], rng);
    case DIST_T: return randompack_tf(x, len, (float)p[0], rng);
    case DIST_F: return randompack_ff(x, len, (float)p[0], (float)p[1], rng);
    case DIST_GUMBEL: return randompack_gumbelf(x, len, (float)p[0], (float)p[1], rng);
    case DIST_PARETO: return randompack_paretof(x, len, (float)p[0], (float)p[1], rng);
    case DIST_WEIBULL: return randompack_weibullf(x, len, (float)p[0], (float)p[1], rng);
    case DIST_SKEW_NORMAL: return randompack_skew_normalf(
      x, len, (float)p[0], (float)p[1], (float)p[2], rng
    );
  }
  return false;
}

int main(int argc, char **argv) {
  uint64_t seed = 123;
  uint64_t ndraws = 1000000000ull;
  char *dist_name = "N(0,1)";
  char *par_text = "";
  char *engine = "x256++simd";
  char *prec_text = "double";
  bool use_float = false;
  bool set_bitexact = true;
  bool help = false;
  bool badopt = false;
  opterr = 0;
  optind = 1;
  int c;
  while ((c = getopt(argc, argv, "hxs:n:d:p:e:P:")) != -1) {
    if (c == 'h') help = true;
    else if (c == 'x') set_bitexact = false;
    else if (c == 's') {
      if (!parse_u64(optarg, &seed)) badopt = true;
    }
    else if (c == 'n') {
      if (!parse_u64(optarg, &ndraws)) badopt = true;
    }
    else if (c == 'd') dist_name = optarg;
    else if (c == 'p') par_text = optarg;
    else if (c == 'e') engine = optarg;
    else if (c == 'P') {
      if (!parse_precision(optarg, &use_float)) badopt = true;
      prec_text = optarg;
    }
    else {
      badopt = true;
    }
  }
  if (optind < argc) badopt = true;
  if (help) {
    print_help();
    return 0;
  }
  if (badopt) {
    print_help();
    return 1;
  }
  dist_t dist;
  int npar = 0;
  if (!parse_dist(dist_name, &dist, &npar)) {
    fprintf(stderr, "TestBitexact: unknown distribution %s\n", dist_name);
    return 1;
  }
  double p[3] = {0, 0, 0};
  if (!parse_double_list(par_text, p, npar)) {
    fprintf(stderr, "TestBitexact: bad parameter list %s\n", par_text);
    return 1;
  }
  randompack_rng *rng = randompack_create(engine);
  if (!rng) {
    fprintf(stderr, "TestBitexact: rng allocation failed\n");
    return 1;
  }
  char *msg = randompack_last_error(rng);
  if (msg) {
    fprintf(stderr, "TestBitexact: %s\n", msg);
    randompack_free(rng);
    return 1;
  }
  if (!randompack_seed(seed, 0, 0, rng)) {
    msg = randompack_last_error(rng);
    fprintf(stderr, "TestBitexact: %s\n", msg ? msg : "seed failed");
    randompack_free(rng);
    return 1;
  }
  if (set_bitexact) {
    if (!randompack_bitexact(rng, true)) {
      msg = randompack_last_error(rng);
      fprintf(stderr, "TestBitexact: %s\n", msg ? msg : "bitexact failed");
      randompack_free(rng);
      return 1;
    }
  }
  if (!use_float) {
    double *x = malloc(65536*sizeof(double));
    if (!x) {
      randompack_free(rng);
      return 1;
    }
    uint64_t xorsum = 0;
    double last = 0;
    uint64_t done = 0;
    while (done < ndraws) {
      size_t take = (size_t)(ndraws - done < 65536 ? ndraws - done : 65536);
      if (!fill_double(x, take, dist, p, rng)) {
        msg = randompack_last_error(rng);
        fprintf(stderr, "TestBitexact: %s\n", msg ? msg : "draw failed");
        free(x);
        randompack_free(rng);
        return 1;
      }
      for (size_t i = 0; i < take; i++) {
        uint64_t u;
        memcpy(&u, x + i, sizeof(u));
        xorsum ^= u;
      }
      last = x[take - 1];
      done += take;
    }
    printf("%-10s %s\n", "engine:", engine);
    printf("%-10s %s\n", "dist:", dist_name);
    printf("%-10s %s\n", "params:", par_text);
    printf("%-10s %s\n", "precision:", prec_text);
    printf("%-10s %" PRIu64 "\n", "draws:", ndraws);
    printf("%-10s %" PRIu64 "\n", "seed:", seed);
    printf("%-10s 0x%016" PRIx64 "\n", "xor:", xorsum);
    printf("%-10s %.17g\n", "last:", last);
    free(x);
  }
  else {
    float *x = malloc(65536*sizeof(float));
    if (!x) {
      randompack_free(rng);
      return 1;
    }
    uint32_t xorsum = 0;
    float last = 0;
    uint64_t done = 0;
    while (done < ndraws) {
      size_t take = (size_t)(ndraws - done < 65536 ? ndraws - done : 65536);
      if (!fill_float(x, take, dist, p, rng)) {
        msg = randompack_last_error(rng);
        fprintf(stderr, "TestBitexact: %s\n", msg ? msg : "draw failed");
        free(x);
        randompack_free(rng);
        return 1;
      }
      for (size_t i = 0; i < take; i++) {
        uint32_t u;
        memcpy(&u, x + i, sizeof(u));
        xorsum ^= u;
      }
      last = x[take - 1];
      done += take;
    }
    printf("%-10s %s\n", "engine:", engine);
    printf("%-10s %s\n", "dist:", dist_name);
    printf("%-10s %s\n", "params:", par_text);
    printf("%-10s %s\n", "precision:", prec_text);
    printf("%-10s %" PRIu64 "\n", "draws:", ndraws);
    printf("%-10s %" PRIu64 "\n", "seed:", seed);
    printf("%-10s 0x%08" PRIx32 "\n", "xor:", xorsum);
    printf("%-10s %.9g\n", "last:", last);
    free(x);
  }
  randompack_free(rng);
  return 0;
}
