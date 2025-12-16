// -*- C -*-
// Timing benchmark: uint64 throughput (GB/s) for randompack engines.

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "randompack.h"

typedef struct {
  const char *name;
  double gbps;
  bool ok;
} row_t;

static uint64_t now_ns(void) {
#if defined(CLOCK_MONOTONIC_RAW)
  const clockid_t clk = CLOCK_MONOTONIC_RAW;
#else
  const clockid_t clk = CLOCK_MONOTONIC;
#endif
  struct timespec ts;
  clock_gettime(clk, &ts);
  return (uint64_t)ts.tv_sec*1000000000ull + (uint64_t)ts.tv_nsec;
}

static double bench_engine(const char *engine, int seed, int n_total, int chunk) {
  randompack_rng *rng = randompack_create(engine);
  if (!rng) return -1.0;

  if (randompack_last_error(rng)) {
    randompack_free(rng);
    return -1.0;
  }

  bool ok = randompack_seed(seed, 0, 0, rng);
  if (!ok) {
    randompack_free(rng);
    return -1.0;
  }

  uint64_t *buf = (uint64_t *)malloc((size_t)chunk*sizeof(uint64_t));
  if (!buf) {
    randompack_free(rng);
    return -1.0;
  }

  volatile uint64_t sink = 0;

  // Small warm-up (burn-in) to settle caches/branch predictors.
  for (int i = 0; i < 4; i++) {
    ok = randompack_uint64(buf, chunk, 0, rng);
    if (!ok) break;
    sink ^= buf[i];
  }
  if (!ok) {
    free(buf);
    randompack_free(rng);
    return -1.0;
  }

  uint64_t t0 = now_ns();
  int left = n_total;
  while (left > 0) {
    int n = left < chunk ? left : chunk;
    ok = randompack_uint64(buf, n, 0, rng);
    if (!ok) break;
    for (int i = 0; i < n; i++) sink ^= buf[i];
    left -= n;
  }
  uint64_t t1 = now_ns();

  // Prevent the compiler from getting clever.
  if (sink == 0x123456789abcdef0ull) printf("sink=%" PRIu64 "\n", sink);

  free(buf);
  randompack_free(rng);

  if (!ok) return -1.0;

  double dt = (double)(t1 - t0)*1e-9;          // seconds
  double bytes = 8.0*(double)n_total;
  return (bytes/1e9)/dt;                       // GB/s (decimal)
}

static int cmp_desc(const void *a, const void *b) {
  const row_t *ra = (const row_t *)a;
  const row_t *rb = (const row_t *)b;
  if (ra->ok != rb->ok) return ra->ok ? -1 : 1;
  if (!ra->ok) return strcmp(ra->name, rb->name);
  if (ra->gbps < rb->gbps) return 1;
  if (ra->gbps > rb->gbps) return -1;
  return strcmp(ra->name, rb->name);
}

static void print_gbps(double x) {
  if (x < 0.0) {
    printf("   (fail)");
  }
  else if (x < 1.0) {
    printf("%7.3f", x);
  }
  else {
    printf("%7.1f", x);
  }
}

int main(void) {
  // Adjust if you want longer/shorter runs.
  const int seed = 7;
  const int n_total = 1<<27;   // ~134M uint64 => ~1.07 GB output
  const int chunk = 1<<16;     // 65536 uint64 per call

  // Engine names must match what randompack_create expects in your build.
  const char *engines[] = {
    "xorshift128+",
    "xoshiro256++",
    "xoshiro256**",
    "pcg64",
    "philox",
    "chacha20",
    "system",
  };
  const int m = (int)(sizeof(engines)/sizeof(engines[0]));
  row_t rows[32];

  for (int i=0; i<m; i++) {
    rows[i].name = engines[i];
    rows[i].gbps = bench_engine(engines[i], seed, n_total, chunk);
    rows[i].ok = rows[i].gbps >= 0.0;
  }

  qsort(rows, (size_t)m, sizeof(rows[0]), cmp_desc);

  printf("\n%-16s %7s\n", "Engine", "GB/s");
  for (int i=0; i<m; i++) {
    printf("%-16s ", rows[i].name);
    print_gbps(rows[i].gbps);
    printf("\n");
  }
  printf("\n");

  return 0;
}
