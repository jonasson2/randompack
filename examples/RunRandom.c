// -*- C -*-
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include "getopt.h"
#include <limits.h>
#include "randompack.h"

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
  printf("RunRandom — generate and print random numbers using randompack\n");
  printf("Usage: RunRandom [options]\n\n");
  printf("Options:\n");
  printf("  -h            Show this help message\n");
  printf("  -n N          Number of values to generate (default 3)\n");
  printf("  -e ENGINE     RNG engine name (default: x256++simd)\n");
  printf("  -s SEED       Integer seed (if omitted, RNG is auto-randomized)\n");
  printf("  -d DIST       Distribution (default u01)\n");
  printf("  -p LIST       Distribution parameters (comma-separated)\n\n");
  printf("Engines:\n");
  print_engines();
  printf("\n");
  printf("Distributions:\n");
  printf("  u01                    U(0,1)\n");
  printf("  unif      -p a,b       U(a,b)\n");
  printf("  norm                   N(0,1)\n");
  printf("  normal    -p mu,sigma  N(mu,sigma)\n");
  printf("  exp       -p scale     Exp(scale)\n");
  printf("  gamma     -p k,theta   Gamma(shape=k, scale=theta)\n");
  printf("  beta      -p a,b       Beta(a,b)\n");
  printf("  int       -p m,n       Uniform integers in [m,n]\n");
  printf("  uint64                 Raw uint64 stream\n\n");
  printf("Examples:\n");
  printf("  RunRandom -n3\n");
  printf("  RunRandom -n5 -d norm\n");
  printf("  RunRandom -e pcg64 -s42 -d normal -p3,2\n");
  printf("  RunRandom -s123 -d unif -p -1,1\n");
  printf("  RunRandom -ephilox -s 1 -duint64 -n4\n");
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

static bool parse_int(const char *s, int *out) {
  if (!s || !*s) return false;
  char *end = 0;
  long v = strtol(s, &end, 10);
  if (!end || *end) return false;
  if (v < INT_MIN || v > INT_MAX) return false;
  *out = (int)v;
  return true;
}

static bool parse_double_list(const char *s, double *p, int want) {
  if (want == 0) return (s == 0 || *s == 0);
  if (!s) return false;
  int n = 1;
  for (const char *q = s; *q; q++) if (*q == ',') n++;
  if (n != want) return false;
  const char *t = s;
  for (int i = 0; i < want; i++) {
    char buf[64];
    int k = 0;
    while (*t && *t != ',' && k < (int)(sizeof(buf) - 1)) buf[k++] = *t++;
    buf[k] = 0;
    if (*t == ',') t++;
    char *end = 0;
    double v = strtod(buf, &end);
    if (!end || *end) return false;
    p[i] = v;
  }
  return true;
}

int main(int argc, char **argv) {
  if (argc == 1) {
    printf("RunRandom: generate random numbers using randompack (-h for help)\n");
    return 0;
  }
  int n = 3;
  const char *engine = 0;
  const char *dist = "u01";
  const char *plist = 0;
  char *msg;
  bool have_seed = false;
  int seed = 0;
  opterr = 0;
  optind = 1;
  int c;
  while ((c = getopt(argc, argv, "hn:e:s:d:p:")) != -1) {
    if (c == 'h') { print_help(); return 0; }
    else if (c == 'n') {
      if (!parse_int(optarg, &n) || n <= 0) { print_help(); return 1; }
    }
    else if (c == 'e') engine = optarg;
    else if (c == 's') {
      if (!parse_int(optarg, &seed)) { print_help(); return 1; }
      have_seed = true;
    }
    else if (c == 'd') dist = optarg;
    else if (c == 'p') plist = optarg;
    else { print_help(); return 1; }
  }
  if (optind < argc) { print_help(); return 1; }
  randompack_rng *rng = randompack_create(engine);
  if (!rng) {
    fprintf(stderr, "RunRandom: rng allocation failed\n");
    return 1;
  }
  msg = randompack_last_error(rng);
  if (msg) {
    fprintf(stderr, "RunRandom: %s\n", msg);
    randompack_free(rng);
    return 1;
  }
  if (have_seed) {
    bool ok = randompack_seed(seed, 0, 0, rng);
    if (!ok) goto fail;
  }
  if (streq_ci(dist, "int")) {
    double pp[2];
    if (!parse_double_list(plist, pp, 2)) goto fail_usage;
    int m = (int)pp[0];
    int nn = (int)pp[1];
    int *x = malloc((size_t)n*sizeof(int));
    if (!x) goto fail;
    if (!randompack_int(x, (size_t)n, m, nn, rng)) { free(x); goto fail; }
    for (int i = 0; i < n; i++) printf("%d\n", x[i]);
    free(x);
  }
  else if (streq_ci(dist, "uint64")) {
    uint64_t *x = malloc((size_t)n*sizeof(uint64_t));
    if (!x) goto fail;
    if (!randompack_uint64(x, (size_t)n, 0, rng)) { free(x); goto fail; }
    for (int i = 0; i < n; i++) printf("%llu\n", (unsigned long long)x[i]);
    free(x);
  }
  else {
    double *x = malloc((size_t)n*sizeof(double));
    if (!x) goto fail;
    if (streq_ci(dist, "u01")) {
      if (!randompack_u01(x, (size_t)n, rng)) { free(x); goto fail; }
    }
    else if (streq_ci(dist, "unif")) {
      double pp[2];
      if (!parse_double_list(plist, pp, 2)) { free(x); goto fail_usage; }
      if (!randompack_unif(x, (size_t)n, pp[0], pp[1], rng)) { free(x); goto fail; }
    }
    else if (streq_ci(dist, "norm")) {
      if (!randompack_norm(x, (size_t)n, rng)) { free(x); goto fail; }
    }
    else if (streq_ci(dist, "normal")) {
      double pp[2];
      if (!parse_double_list(plist, pp, 2)) { free(x); goto fail_usage; }
      if (!randompack_normal(x, (size_t)n, pp[0], pp[1], rng)) { free(x); goto fail; }
    }
    else if (streq_ci(dist, "exp")) {
      double pp[1];
      if (!parse_double_list(plist, pp, 1)) { free(x); goto fail_usage; }
      if (!randompack_exp(x, (size_t)n, pp[0], rng)) { free(x); goto fail; }
    }
    else if (streq_ci(dist, "gamma")) {
      double pp[2];
      if (!parse_double_list(plist, pp, 2)) { free(x); goto fail_usage; }
      if (!randompack_gamma(x, (size_t)n, pp[0], pp[1], rng)) { free(x); goto fail; }
    }
    else if (streq_ci(dist, "beta")) {
      double pp[2];
      if (!parse_double_list(plist, pp, 2)) { free(x); goto fail_usage; }
      if (!randompack_beta(x, (size_t)n, pp[0], pp[1], rng)) { free(x); goto fail; }
    }
    else {
      free(x);
      goto fail_usage;
    }
    for (int i = 0; i < n; i++) printf("% .5f\n", x[i]);
    free(x);
  }
  randompack_free(rng);
  return 0;

 fail_usage:
  fprintf(stderr, "RunRandom: bad distribution arguments (use -h)\n");
 fail:
  msg = randompack_last_error(rng);
  if (msg) fprintf(stderr, "RunRandom: %s\n", msg);
  randompack_free(rng);
  return 1;
}
