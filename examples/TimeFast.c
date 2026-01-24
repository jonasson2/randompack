// -*- C -*-
// TimeFast.c: throughput (GB/s) of the fast engine using uint64 and uint32 bulk fill.
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "TimeUtil.h"
#include "Util.h"
#include "randompack.h"
#include "randompack_config.h"
static void fill_u64(uint64_t out[], int n, randompack_rng *rng) {
  size_t len = (size_t)n;
  randompack_uint64(out, len, 0, rng);
}
static void warmup_min_time(double seconds) {
  double t0 = get_time();
  while (get_time() - t0 < seconds) warmup_cpu(10);
}
int main(void) {
  const char *engine = "fast";
  const int chunk = 4096;
  const double warmup_time = 0.2;
  const double bench_time = 0.8;
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
  double ns64 = time_u64(chunk, bench_time, fill_u64, rng);
  printf("Time per draw: %.3f ns\n", ns64);
  randompack_free(rng);
  return 0;
}
