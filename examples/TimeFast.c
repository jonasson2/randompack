// -*- C -*-
// TimeFast.c: u01 throughput for the fast engine.
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "TimeUtil.h"
#include "Util.h"
#include "randompack.h"
#include "randompack_config.h"
static void fill_u01(double out[], int n, double param[], randompack_rng *rng) {
  size_t len = (size_t)n;
  (void)param;
  randompack_u01(out, len, rng);
}
static void warmup_min_time(double seconds) {
  uint64_t t0 = clock_nsec();
  uint64_t deadline = t0 + (uint64_t)(seconds*1e9);
  uint64_t t = t0;
  while (t < deadline) {
    warmup_cpu(10);
    t = clock_nsec();
  }
}
int main(void) {
  const char *engine = "fast";
  const int chunk = 4096;
  const double warmup_time = 0.2;
  const double bench_time = 0.8;
#if defined(__linux__)
#endif
  randompack_rng *rng = randompack_create(engine);
  if (!rng) {
    fprintf(stderr, "randompack_create failed: %s\n", engine);
    return 1;
  }
  if (!randompack_seed(7, 0, 0, rng)) {
    if (!randompack_randomize(rng)) {
      fprintf(stderr, "randompack_seed/randomize failed: %s\n", engine);
      randompack_free(rng);
      return 1;
    }
  }
  warmup_min_time(warmup_time);
  double param[1] = {0};
  double ns = time_double(chunk, bench_time, fill_u01, param, rng);
  printf("%.2f ns\n", ns);
  randompack_free(rng);
  return 0;
}
