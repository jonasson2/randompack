// -*- C -*-
// TimeMul9x9: timing toy 576-bit multiply (ns/value).

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "mul9x9.h"
#include "mul_jirka.h"
#include "mod_m.h"

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
    for (int i = 0; i < 10000; i++)
      sink += cos((double)i);
    t = clock_nsec();
  }
  (void)sink;
}

static void mul_jirka_wrap(uint64_t *out, const uint64_t *in, const uint64_t *a) {
  multiply9x9_jirka(in, a, out);
}

static void mul9x9_wrap(uint64_t *out, const uint64_t *in, const uint64_t *a) {
  (void)a;
  mul9x9(out, in);
}

static void mod9x9_wrap(uint64_t *x) {
  mod9x9(x);
}

static void mod_m_wrap0(const uint64_t *x, uint64_t *out) {
  mod_m_wrap(x, out);
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
      mul9x9_wrap(z + (size_t)i*18, x + (size_t)i*9, a);
      mod9x9_wrap(z + (size_t)i*18);
      sink ^= z[(size_t)i*18 + 17];
    }
    calls += (uint64_t)chunk;
    t = clock_nsec();
  }
  (void)sink;
  return (calls > 0) ? (t - t0)/((double)calls) : 0;
}

static double time_mulmod_jirka(const uint64_t *a, uint64_t *x, uint64_t *z,
  uint64_t *out, int chunk, double bench_time) {
  uint64_t t0 = clock_nsec();
  uint64_t deadline = t0 + (uint64_t)(bench_time*1e9);
  uint64_t t = t0;
  uint64_t calls = 0;
  volatile uint64_t sink = 0;
  while (t < deadline) {
    for (int i = 0; i < chunk; i++) {
      mul_jirka_wrap(z + (size_t)i*18, x + (size_t)i*9, a);
      mod_m_wrap0(z + (size_t)i*18, out + (size_t)i*9);
      sink ^= out[(size_t)i*9 + 8];
    }
    calls += (uint64_t)chunk;
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
  uint64_t *mout = calloc((size_t)chunk*9, sizeof(*mout));
  if (!x || !z || !mout) {
    fprintf(stderr, "allocation failed\n");
    free(x);
    free(z);
    free(mout);
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
  mul9x9_wrap(z3, x, A);
  mod9x9_wrap(z3);
  mul_jirka_wrap(z1, x, A);
  mod_m_wrap0(z1, m1);
  for (int i = 0; i < 9; i++) m0[i] = z3[i];
  for (int i = 0; i < 9; i++) {
    if (m0[i] != m1[i]) {
      fprintf(stderr, "mod mismatch at word %d\n", i);
      free(x);
      free(z);
      free(mout);
      return 1;
    }
  }
  warmup_cpu(warmup_time);
  double ns_rp = time_mulmod_rp(A, x, z, chunk, bench_time);
  double ns_j = time_mulmod_jirka(A, x, z, mout, chunk, bench_time);
  printf("TimeMul9x9:       %.1f ns/value\n", ns_rp);
  printf("TimeMul9x9Jirka:  %.1f ns/value\n", ns_j);
  free(x);
  free(z);
  free(mout);
  return 0;
}
