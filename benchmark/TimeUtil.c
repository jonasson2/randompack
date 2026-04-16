// -*- C -*-
// TimeUtil.c: shared timing utilities for benchmarks.

#include "randompack_config.h"
#include "randompack.h"
#include "TimeUtil.h"
#include "Util.h"

#include <math.h>
#include <stdint.h>
#include <string.h>

enum { M = 1000000 };

static inline void consume5(const double *buf, int chunk) {
  static volatile uint64_t sink;
  uint64_t u;
  memcpy(&u, &buf[0], sizeof(u)); sink ^= u;
  memcpy(&u, &buf[chunk/4], sizeof(u)); sink ^= u;
  memcpy(&u, &buf[chunk/2], sizeof(u)); sink ^= u;
  memcpy(&u, &buf[3*chunk/4], sizeof(u)); sink ^= u;
  memcpy(&u, &buf[chunk-1], sizeof(u)); sink ^= u;
}

static inline void consume64(const void *p) {
  static volatile uint64_t sink;
  uint64_t u;
  memcpy(&u, p, sizeof(u));
  sink ^= u;
}

static inline void consume32(const void *p) {
  static volatile uint32_t sink;
  uint32_t u;
  memcpy(&u, p, sizeof(u));
  sink ^= u;
}

void warmup_cpu(double seconds) {
  if (seconds <= 0)
    return;
  volatile double sink = 0;
  double x = 1.000001;
  uint64_t t0 = clock_nsec();
  uint64_t deadline = t0 + (uint64_t)(seconds*1e9);
  uint64_t t = t0;
  while (t < deadline) {
    for (int i = 0; i < 1000; i++) {
      sink += log(x);
      x += 0.000001;
      if (x >= 2)
        x = 1.000001;
    }
    t = clock_nsec();
  }
  (void)sink;
}

double time_u64(int chunk, double bench_time, fill_u64_fn fill, randompack_rng *rng) {
  uint64_t *buf;
  TEST_ALLOC(buf, chunk);
  int reps = maxi(1, M/chunk);
  int64_t calls = 0;
  uint64_t t0 = clock_nsec();
  uint64_t deadline = t0 + (uint64_t)(bench_time*1e9);
  uint64_t t = t0;
  while (t < deadline) {
    for (int i = 0; i < reps; i++) {
      fill(buf, chunk, rng);
      consume64(&buf[chunk-1]);
    }
    calls += reps;
    t = clock_nsec();
  }
  FREE(buf);
  return (calls > 0) ? (t - t0)/((double)calls*chunk) : 0;
}

double time_u32(int chunk, double bench_time, fill_u32_fn fill, randompack_rng *rng) {
  uint32_t *buf;
  TEST_ALLOC(buf, chunk);
  int reps = maxi(1, M/chunk);
  int64_t calls = 0;
  uint64_t t0 = clock_nsec();
  uint64_t deadline = t0 + (uint64_t)(bench_time*1e9);
  uint64_t t = t0;
  while (t < deadline) {
    for (int i = 0; i < reps; i++) {
      fill(buf, chunk, rng);
      consume32(&buf[chunk-1]);
    }
    calls += reps;
    t = clock_nsec();
  }
  FREE(buf);
  return (calls > 0) ? (t - t0)/((double)calls*chunk) : 0;
}

double time_double(int chunk, double bench_time, fill_double_fn fill, double param[],
                   randompack_rng *rng) {
  double *buf;
  TEST_ALLOC(buf, chunk);
  int reps = maxi(1, M/chunk);
  int64_t calls = 0;
  uint64_t t0 = clock_nsec();
  uint64_t deadline = t0 + (uint64_t)(bench_time*1e9);
  uint64_t t = t0;
  while (t < deadline) {
    for (int i = 0; i < reps; i++) {
      fill(buf, chunk, param, rng);
      consume5(buf, chunk);
    }
    calls += reps;
    t = clock_nsec();
  }
  FREE(buf);
  return (calls > 0) ? (t - t0)/((double)calls*chunk) : 0;
}

double time_float(int chunk, double bench_time, fill_float_fn fill, float param[],
                  randompack_rng *rng) {
  float *buf;
  TEST_ALLOC(buf, chunk);
  int reps = maxi(1, M/chunk);
  int64_t calls = 0;
  uint64_t t0 = clock_nsec();
  uint64_t deadline = t0 + (uint64_t)(bench_time*1e9);
  uint64_t t = t0;
  while (t < deadline) {
    for (int i = 0; i < reps; i++) {
      fill(buf, chunk, param, rng);
      consume32(&buf[chunk-1]);
    }
    calls += reps;
    t = clock_nsec();
  }
  FREE(buf);
  return (calls > 0) ? (t - t0)/((double)calls*chunk) : 0;
}
