// -*- C -*-
// TimeUtil.c: shared timing utilities for benchmarks.

#include <stdint.h>
#include <string.h>

#include "TimeUtil.h"
#include "Util.h"

static inline void consume64(const void *p) { // to make sure loops are not optimized away
  static volatile uint64_t sink;
  uint64_t u;
  memcpy(&u, p, sizeof(u));
  sink ^= u;
}

static inline void consume32(const void *p) { // to make sure loops are not optimized away
  static volatile uint32_t sink;
  uint32_t u;
  memcpy(&u, p, sizeof(u));
  sink ^= u;
}

double time_u64(int chunk, double bench_time, fill_u64_fn fill,
                randompack_rng *rng) {
  uint64_t *buf;
  TEST_ALLOC(buf, chunk);
  int reps = max(1, 1000000/chunk); // ~1e6 values between get_time calls (negligible overhead)
  int calls = 0;
  double t0 = get_time(), t = t0;
  while (t - t0 < bench_time) {
    for (int i = 0; i < reps; i++) {
      fill(buf, chunk, rng);
      consume64(&buf[chunk - 1]);
    }
    calls += reps;
    t = get_time();
  }
  FREE(buf);
  return (calls > 0) ? 1e9*(t - t0)/(calls*chunk) : 0;
}

double time_u32(int chunk, double bench_time, fill_u32_fn fill,
                randompack_rng *rng) {
  uint32_t *buf;
  TEST_ALLOC(buf, chunk);
  int reps = max(1, 1000000/chunk); // ~1e6 values between get_time calls (negligible overhead)
  int calls = 0;
  double t0 = get_time(), t = t0;
  while (t - t0 < bench_time) {
    for (int i = 0; i < reps; i++) {
      fill(buf, chunk, rng);
      consume32(&buf[chunk - 1]);
    }
    calls += reps;
    t = get_time();
  }
  FREE(buf);
  return (calls > 0) ? 1e9*(t - t0)/(calls*chunk) : 0;
}

double time_double(int chunk, double bench_time, fill_double_fn fill,
                   double param[], randompack_rng *rng) {
  double *buf;
  TEST_ALLOC(buf, chunk);
  int reps = max(1, 1000000/chunk); // ~1e6 values between get_time calls (negligible overhead)
  int calls = 0;
  double t0 = get_time(), t = t0;
  while (t - t0 < bench_time) {
    for (int i = 0; i < reps; i++) {
      fill(buf, chunk, param, rng);
      consume64(&buf[chunk - 1]);
    }
    calls += reps;
    t = get_time();
  }
  FREE(buf);
  return (calls > 0) ? 1e9*(t - t0)/(calls*chunk) : 0;
}

