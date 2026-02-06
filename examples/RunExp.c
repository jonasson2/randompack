// -*- C -*-
// RunExp.c: call randompack_exp n times and report counters.

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "getopt.h"
#include "randompack.h"

static void print_help(void) {
  printf("RunExp — call randompack_exp n times and report counters\n");
  printf("Usage: RunExp -n N\n\n");
  printf("Options:\n");
  printf("  -h            Show this help message\n");
  printf("  -n N          Number of calls (required)\n");
}

static bool parse_int(const char *s, int *out) {
  if (!s || !*s) return false;
  char *end = 0;
  long v = strtol(s, &end, 10);
  if (!end || *end) return false;
  if (v < 0 || v > INT_MAX) return false;
  *out = (int)v;
  return true;
}

int main(int argc, char **argv) {
  int n = 0;
  opterr = 0;
  optind = 1;
  int c;
  while ((c = getopt(argc, argv, "hn:")) != -1) {
    if (c == 'h') { print_help(); return 0; }
    else if (c == 'n') {
      if (!parse_int(optarg, &n) || n <= 0) { print_help(); return 1; }
    }
    else { print_help(); return 1; }
  }
  if (optind < argc || n <= 0) { print_help(); return 1; }
  randompack_rng *rng = randompack_create(0);
  if (!rng) {
    fprintf(stderr, "RunExp: rng allocation failed\n");
    return 1;
  }
  double x[1];
  for (int i = 0; i < n; i++) {
    if (!randompack_exp(x, 1, 1, rng)) {
      char *msg = randompack_last_error(rng);
      if (msg) fprintf(stderr, "RunExp: %s\n", msg);
      randompack_free(rng);
      return 1;
    }
  }
  int cas, cbt, cc, cbc, ctail;
  randompack_get_counters(&cas, &cbt, &cc, &cbc, &ctail);
  double dn = (double)n;
  printf("n: %d\n", n);
  printf("above_secant: %d (%.4f%%)\n", cas, 100*cas/dn);
  printf("below_tangent: %d (%.4f%%)\n", cbt, 100*cbt/dn);
  printf("curve: %d (%.4f%%)\n", cc, 100*cc/dn);
  printf("below_curve: %d (%.4f%%)\n", cbc, 100*cbc/dn);
  printf("tail: %d (%.4f%%)\n", ctail, 100*ctail/dn);
  randompack_free(rng);
  return 0;
}
