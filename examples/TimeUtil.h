// -*- C -*-
// TimeUtil.h: shared timing utilities for benchmarks.

#ifndef TIMEUTIL_H
#define TIMEUTIL_H

#include <stdbool.h>
#include <stdint.h>

typedef struct randompack_rng randompack_rng;

typedef void (*fill_u64_fn)( // Callback type: fill out with random uint64s
  uint64_t out[],      // output buffer (length n)
  int n,               // number of values to generate
  randompack_rng *rng  // RNG handle
);

typedef void (*fill_u32_fn)( // Callback type: fill out with random uint32s
  uint32_t out[],      // output buffer (length n)
  int n,               // number of values to generate
  randompack_rng *rng  // RNG handle
);

typedef void (*fill_double_fn)( // Callback type: fill out with random doubles
  double out[],        // output buffer (length n)
  int n,               // number of values to generate
  double param[],      // parameters (length depends on distribution; may be 0)
  randompack_rng *rng  // RNG handle
);

double time_u64( // Benchmark uint64 fill amortizing time-keeping overhead, return ns
  int chunk,             // number of values generated per fill call
  double bench_time,     // repeat until this time (in seconds) has elapsed
  fill_u64_fn fill, // fill callback producing uint64 output
  randompack_rng *rng    // RNG handle
);

double time_u32( // Benchmark uint32 fill amortizing time-keeping overhead, return ns
  int chunk,             // number of values generated per fill call
  double bench_time,     // repeat until this time (in seconds) has elapsed
  fill_u32_fn fill, // fill callback producing uint32 output
  randompack_rng *rng    // RNG handle
);

double time_double( // Benchmark double fill amortizing time-keeping overhead, return ns
  int chunk,                 // number of values generated per fill call
  double bench_time,         // repeat until this time (in seconds) has elapsed
  fill_double_fn fill,  // fill callback producing double output
  double param[],            // parameters passed to fill (may be 0)
  randompack_rng *rng        // RNG handle
);

double time_norm(int chunk, double bench_time, randompack_rng *rng);

#endif
