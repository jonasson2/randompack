// -*- C -*-
// TimeUtil.c: shared timing utilities for benchmarks.

#include <stdint.h>
#include <string.h>
#if defined(__linux__)
#include <sched.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#endif
#include "randompack.h"
#include "TimeUtil.h"
#include "Util.h"

enum { M = 1000000 };

#include <stdint.h>
#include <stddef.h>

static inline void consume5(const double *buf, int chunk) {
  static volatile uint64_t sink;
  uint64_t u;
  memcpy(&u, &buf[0], sizeof(u)); sink ^= u;
  memcpy(&u, &buf[chunk/4], sizeof(u)); sink ^= u;
  memcpy(&u, &buf[chunk/2], sizeof(u)); sink ^= u;
  memcpy(&u, &buf[3*chunk/4], sizeof(u)); sink ^= u;
  memcpy(&u, &buf[chunk-1], sizeof(u)); sink ^= u;
}

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

double time_u64(int chunk, double bench_time, fill_u64_fn fill, randompack_rng *rng) {
  uint64_t *buf;
  TEST_ALLOC(buf, chunk);
  int reps = max(1, M/chunk); // ~1e6 values between get_time calls (negligible overhead)
  int64_t calls = 0;
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
  return (calls > 0) ? 1e9*(t - t0)/((double)calls*chunk) : 0;
}

double time_u64_cb(int chunk, double bench_time, fill_u64_cb fill, void *ctr, void *key) {
  // Time counter based fill
  uint64_t *buf;
  TEST_ALLOC(buf, chunk);
  int reps = max(1, 1000000/chunk);
  int64_t calls = 0;
  double t0 = get_time();
  double t = t0;
  while (t - t0 < bench_time) {
    for (int i=0; i<reps; i++) {
      fill(buf, chunk, ctr, key);
      consume64(&buf[chunk - 1]);
    }
    calls += reps;
    t = get_time();
  }
  FREE(buf);
  return (calls > 0) ? 1e9*(t - t0)/((double)calls*chunk) : 0;
}

double time_u32(int chunk, double bench_time, fill_u32_fn fill, randompack_rng *rng) {
  uint32_t *buf;
  TEST_ALLOC(buf, chunk);
  int reps = max(1, M/chunk); // ~1e6 values between get_time calls (negligible overhead)
  int64_t calls = 0;
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
  return (calls > 0) ? 1e9*(t - t0)/((double)calls*chunk) : 0;
}

double time_double(int chunk, double bench_time, fill_double_fn fill,
                   double param[], randompack_rng *rng) {
  double *buf;
  TEST_ALLOC(buf, chunk);
  // ~1e6 values between get_time calls (negligible overhead)
  int reps = max(1, 1000000/chunk);
  int64_t calls = 0;
  double t0 = get_time(), t = t0;
  while (t - t0 < bench_time) {
    for (int i = 0; i < reps; i++) {
      fill(buf, chunk, param, rng);
      consume5(buf, chunk);
      // consume64(&buf[chunk - 1]);
    }
    calls += reps;
    t = get_time();
  }
  FREE(buf);
  return (calls > 0) ? 1e9*(t - t0)/((double)calls*chunk) : 0;
}

double time_float(int chunk, double bench_time, fill_float_fn fill,
                  float param[], randompack_rng *rng) {
  float *buf;
  TEST_ALLOC(buf, chunk);
  // ~1e6 values between get_time calls (negligible overhead)
  int reps = max(1, 1000000/chunk);
  int64_t calls = 0;
  double t0 = get_time(), t = t0;
  while (t - t0 < bench_time) {
    for (int i = 0; i < reps; i++) {
      fill(buf, chunk, param, rng);
      consume32(&buf[chunk - 1]);
    }
    calls += reps;
    t = get_time();
  }
  FREE(buf);
  return (calls > 0) ? 1e9*(t - t0)/((double)calls*chunk) : 0;
}
