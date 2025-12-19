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
#include "randompack_config.h"

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
  ALLOC(buf, chunk);
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
    sink ^= buf[0];
    sink ^= buf[n/2];
    sink ^= buf[n-1];
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

static inline uint64_t rotl(uint64_t x, int k) {
  return (x << k) | (x >> (64 - k));
}

static void seed_x256ss(uint64_t *s, int seed) {
  uint64_t x = (uint64_t)seed;
  for (int i = 0; i < 4; i++) s[i] = rand_splitmix64(&x);
  if (s[0] == 0 && s[1] == 0 && s[2] == 0 && s[3] == 0) s[0] = 1;
}

static inline uint64_t nextss_state(uint64_t *s) {
  uint64_t t = s[1] << 17;
  uint64_t result = rotl(s[1] * 5, 7) * 9;
  s[2] ^= s[0];
  s[3] ^= s[1];
  s[1] ^= s[2];
  s[0] ^= s[3];
  s[2] ^= t;
  s[3] = rotl(s[3], 45);
  return result;
}

static double bench_x256ss(int seed, int n_total) {
  uint64_t s[4];
  seed_x256ss(s, seed);
  volatile uint64_t sink = 0;
  uint64_t t0 = now_ns();
  for (int i = 0; i < n_total; i++) sink ^= nextss_state(s);
  uint64_t t1 = now_ns();
  double dt = (double)(t1 - t0)*1e-9;
  return (8.0*(double)n_total/1e9)/dt;
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

int main(void) {
  // Adjust if you want longer/shorter runs.
  const int seed = 7;
  const int n_total = (1<<27) / 10;   // shorter run while debugging
  const int chunk   = 1<<12;

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
  int k = 0;

  rows[k].name = "xoshiro256** (direct)";
  rows[k].gbps = bench_x256ss(seed, n_total);
  rows[k].ok = rows[k].gbps >= 0.0;
  k++;

  for (int i = 0; i < m; i++) {
    rows[k].name = engines[i];
    rows[k].gbps = bench_engine(engines[i], seed, n_total, chunk);
    rows[k].ok = rows[k].gbps >= 0.0;
    k++;
  }

  qsort(rows, (size_t)k, sizeof(rows[0]), cmp_desc);

  printf("\n%-16s %7s\n", "Engine", "GB/s");
  for (int i = 0; i < k; i++) {
    printf("%-16s %7.2f\n", rows[i].name, rows[i].gbps);
  }
  printf("\n");

  return 0;
}
