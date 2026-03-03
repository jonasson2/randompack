// -*- C -*-
// TimeMul9x9: timing toy 576-bit multiply (ns/value).

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "mul9x9.h"
#include "mul_jirka.h"

#if defined(_WIN32)
  #include <windows.h>
#else
  #include <time.h>
#endif

static inline uint64_t clock_nsec(void) {
#ifdef _WIN32
  LARGE_INTEGER freq, counter;
  QueryPerformanceFrequency(&freq);
  QueryPerformanceCounter(&counter);
  return (uint64_t)((1000000000.0*(double)counter.QuadPart)/(double)freq.QuadPart);
#elif defined(CLOCK_MONOTONIC)
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0)
    return (uint64_t)ts.tv_sec*1000000000ULL + (uint64_t)ts.tv_nsec;
#endif
  return (uint64_t)((1000000000.0*(double)clock())/(double)CLOCKS_PER_SEC);
}

static void warmup_cpu(double seconds) {
  uint64_t t0 = clock_nsec();
  uint64_t deadline = t0 + (uint64_t)(seconds*1e9);
  uint64_t t = t0;
  volatile double sink = 0;
  while (t < deadline) {
    for (int i = 0; i < 1000000; i++)
      sink += (double)i;
    t = clock_nsec();
  }
  (void)sink;
}

static double time_mulmod_rp(const uint64_t *a, uint64_t *x, uint64_t *z,
  int chunk, double bench_time) {
  uint64_t t0 = clock_nsec();
  uint64_t deadline = t0 + (uint64_t)(bench_time*1e9);
  uint64_t t = t0;
  uint64_t calls = 0;
  volatile uint64_t sink = 0;
  while (t < deadline) {
    for (int i = 0; i < chunk; i++) {
      uint64_t *zi = z + (size_t)i*18;
      uint64_t *xi = x + (size_t)i*9;
      for (int r = 0; r < 64; r++) {
        (void)a;
        mul9x9(zi, xi);
        mod9x9(xi, zi);
        sink ^= xi[8];
      }
    }
    calls += (uint64_t)chunk*64;
    t = clock_nsec();
  }
  (void)sink;
  return (calls > 0) ? (t - t0)/((double)calls) : 0;
}

static double time_mulmod_jirka(const uint64_t *a, uint64_t *x, uint64_t *z,
  int chunk, double bench_time) {
  uint64_t t0 = clock_nsec();
  uint64_t deadline = t0 + (uint64_t)(bench_time*1e9);
  uint64_t t = t0;
  uint64_t calls = 0;
  volatile uint64_t sink = 0;
  while (t < deadline) {
    for (int i = 0; i < chunk; i++) {
      uint64_t *zi = z + (size_t)i*18;
      uint64_t *xi = x + (size_t)i*9;
      for (int r = 0; r < 64; r++) {
        multiply9x9_jirka(xi, a, zi);
        mod9x9(xi, zi);
        sink ^= xi[8];
      }
    }
    calls += (uint64_t)chunk*64;
    t = clock_nsec();
  }
  (void)sink;
  return (calls > 0) ? (t - t0)/((double)calls) : 0;
}

static double time_mul_only(uint64_t *x, uint64_t *z, int chunk, double bench_time) {
  uint64_t t0 = clock_nsec();
  uint64_t deadline = t0 + (uint64_t)(bench_time*1e9);
  uint64_t t = t0;
  uint64_t calls = 0;
  volatile uint64_t sink = 0;
  while (t < deadline) {
    for (int i = 0; i < chunk; i++) {
      uint64_t *zi = z + (size_t)i*18;
      uint64_t *xi = x + (size_t)i*9;
      for (int r = 0; r < 64; r++) {
        mul9x9(zi, xi);
        sink ^= zi[17];
      }
    }
    calls += (uint64_t)chunk*64;
    t = clock_nsec();
  }
  (void)sink;
  return (calls > 0) ? (t - t0)/((double)calls) : 0;
}

static double time_mod_only(uint64_t *x, uint64_t *z, int chunk, double bench_time) {
  uint64_t t0 = clock_nsec();
  uint64_t deadline = t0 + (uint64_t)(bench_time*1e9);
  uint64_t t = t0;
  uint64_t calls = 0;
  volatile uint64_t sink = 0;
  while (t < deadline) {
    for (int i = 0; i < chunk; i++) {
      uint64_t *zi = z + (size_t)i*18;
      uint64_t *xi = x + (size_t)i*9;
      for (int r = 0; r < 64; r++) {
        mod9x9(xi, zi);
        sink ^= xi[8];
      }
    }
    calls += (uint64_t)chunk*64;
    t = clock_nsec();
  }
  (void)sink;
  return (calls > 0) ? (t - t0)/((double)calls) : 0;
}

int main(void) {
  static const uint64_t A[9] = {
    0xed7faa90747aaad9ULL,  // word 0
    0x4cec2c78af55c101ULL,  // word 1
    0xe64dcb31c48228ecULL,  // word 2
    0x6d8a15a13bee7cb0ULL,  // word 3
    0x20b2ca60cb78c509ULL,  // word 4
    0x256c3d3c662ea36cULL,  // word 5
    0xff74e54107684ed2ULL,  // word 6
    0x492edfcc0cc8e753ULL,  // word 7
    0xb48c187cf5b22097ULL,  // word 8
  };
  const int chunk = 4096;
  const double bench_time = 0.2;
  const double warmup_time = 0.1;
  uint64_t *x = calloc((size_t)chunk*9, sizeof(*x));
  uint64_t *z = calloc((size_t)chunk*18, sizeof(*z));
  if (!x || !z) {
    fprintf(stderr, "allocation failed\n");
    free(x);
    free(z);
    return 1;
  }
  uint64_t seed = 0x1234567890abcdefULL;
  for (int i = 0; i < chunk*9; i++) {
    seed = seed*6364136223846793005ULL + 1442695040888963407ULL;
    x[i] = seed;
  }
  uint64_t z1[18];
  uint64_t z3[18];
  uint64_t m0[9];
  uint64_t m1[9];
  mul9x9(z3, x);
  mod9x9(m0, z3);
  multiply9x9_jirka(x, A, z1);
  mod9x9(m1, z1);
  for (int i = 0; i < 9; i++) {
    if (m0[i] != m1[i]) {
      fprintf(stderr, "mod mismatch at word %d\n", i);
      free(x);
      free(z);
      return 1;
    }
  }
  warmup_cpu(warmup_time);
  double ns_rp = time_mulmod_rp(A, x, z, chunk, bench_time);
  double ns_j = time_mulmod_jirka(A, x, z, chunk, bench_time);
  double ns_mul = time_mul_only(x, z, chunk, bench_time);
  double ns_mod = time_mod_only(x, z, chunk, bench_time);
  printf("TimeMul9x9:       %.1f ns/value\n", ns_rp);
  printf("TimeMul9x9Jirka:  %.1f ns/value\n", ns_j);
  printf("mul9x9 only:      %.1f ns/value\n", ns_mul);
  printf("mod9x9 only:      %.1f ns/value\n", ns_mod);
  free(x);
  free(z);
  return 0;
}
