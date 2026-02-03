// -*- C -*-
// TimeFastDirect.c: AVX2-only proof-of-concept fast u01 fill (4 streams).
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "Util.h"
#include "randompack_config.h"
#if HAVE_AVX2
#include <immintrin.h>
typedef struct {
  uint64_t s[4][4];
} fast_state;
static inline uint64_t rotl64(uint64_t x, int k) {
  return (x << k) | (x >> (64 - k));
}
static inline uint64_t splitmix64(uint64_t *x) {
  uint64_t z;
  *x += 0x9e3779b97f4a7c15ULL;
  z = *x;
  z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
  z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
  return z ^ (z >> 31);
}
static void init_fast_state(fast_state *st, uint64_t seed) {
  uint64_t x = seed;
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) st->s[i][j] = splitmix64(&x);
    if (st->s[i][0] == 0 && st->s[i][1] == 0 &&
        st->s[i][2] == 0 && st->s[i][3] == 0)
      st->s[i][0] = 1;
  }
}
#define FAST_STEP_SCALAR(s0,s1,s2,s3,outv) do { \
  uint64_t r_; \
  uint64_t t_; \
  r_ = rotl64((s0) + (s3), 23) + (s0); \
  t_ = (s1) << 17; \
  (s2) ^= (s0); \
  (s3) ^= (s1); \
  (s1) ^= (s2); \
  (s0) ^= (s3); \
  (s2) ^= t_; \
  (s3) = rotl64((s3), 45); \
  (outv) = r_; \
} while (0)
static inline __m256d u64_to_u01_4x_avx2(uint64_t r0, uint64_t r1,
  uint64_t r2, uint64_t r3) {
  __m256i v = _mm256_set_epi64x((long long)r3, (long long)r2,
    (long long)r1, (long long)r0);
  v = _mm256_srli_epi64(v, 12);
  __m256i ones = _mm256_set1_epi64x((long long)0x3ff0000000000000ULL);
  v = _mm256_or_si256(v, ones);
  __m256d d = _mm256_castsi256_pd(v);
  return _mm256_sub_pd(d, _mm256_set1_pd(1.0));
}
static void fill_fast_u01(double out[], size_t len, fast_state *st) {
  uint64_t (*s)[4] = st->s;
  uint64_t a0 = s[0][0], a1 = s[0][1], a2 = s[0][2], a3 = s[0][3];
  uint64_t b0 = s[1][0], b1 = s[1][1], b2 = s[1][2], b3 = s[1][3];
  uint64_t c0 = s[2][0], c1 = s[2][1], c2 = s[2][2], c3 = s[2][3];
  uint64_t d0 = s[3][0], d1 = s[3][1], d2 = s[3][2], d3 = s[3][3];
  // len must be a multiple of 4 for this direct kernel.
  for (size_t i = 0; i < len; i += 4) {
    uint64_t r0, r1, r2, r3;
    FAST_STEP_SCALAR(a0, a1, a2, a3, r0);
    FAST_STEP_SCALAR(b0, b1, b2, b3, r1);
    FAST_STEP_SCALAR(c0, c1, c2, c3, r2);
    FAST_STEP_SCALAR(d0, d1, d2, d3, r3);
    __m256d x = u64_to_u01_4x_avx2(r0, r1, r2, r3);
    _mm256_storeu_pd(out + i, x);
  }
  s[0][0] = a0; s[0][1] = a1; s[0][2] = a2; s[0][3] = a3;
  s[1][0] = b0; s[1][1] = b1; s[1][2] = b2; s[1][3] = b3;
  s[2][0] = c0; s[2][1] = c1; s[2][2] = c2; s[2][3] = c3;
  s[3][0] = d0; s[3][1] = d1; s[3][2] = d2; s[3][3] = d3;
}
static inline void consume_double(const void *p) {
  static volatile uint64_t sink;
  uint64_t u;
  memcpy(&u, p, sizeof(u));
  sink ^= u;
}
static double time_fast_u01(double out[], size_t len, double bench_time,
  fast_state *st) {
  size_t reps = 1000000 / len;
  size_t calls = 0;
  if (reps < 1) reps = 1;
  double t0 = get_time(), t = t0;
  while (t - t0 < bench_time) {
    for (size_t i = 0; i < reps; i++) {
      fill_fast_u01(out, len, st);
      consume_double(&out[len - 1]);
    }
    calls += reps;
    t = get_time();
  }
  return (calls > 0) ? 1e9*(t - t0)/((double)(calls*len)) : 0;
}
int main(void) {
  enum { LEN = 4096 };
  double out[LEN];
  fast_state st;
  init_fast_state(&st, 7);
  pin_to_cpu0();
  warmup_cpu(50);
  double ns = time_fast_u01(out, LEN, 0.8, &st);
  printf("Time per draw: %.3f ns\n", ns);
  return 0;
}
#else
int main(void) {
  return 0;
}
#endif
