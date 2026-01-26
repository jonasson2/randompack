// -*- C -*-
// TimeFast.c: u01 throughput: randompack sfc64 vs local sfc64 (4 independent streams).
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <arm_neon.h>
#include "TimeUtil.h"
#include "Util.h"
#include "randompack.h"
#include "randompack_config.h"

typedef struct {
  uint64_t a;
  uint64_t b;
  uint64_t c;
  uint64_t counter;
} sfc64_state;

typedef struct {
  sfc64_state s0;
  sfc64_state s1;
  sfc64_state s2;
  sfc64_state s3;
} sfc64_4stream;

static sfc64_4stream st4;

static inline uint64_t splitmix64_next(uint64_t *x) {
  uint64_t z = (*x += 0x9e3779b97f4a7c15ULL);
  z = (z ^ (z >> 30))*0xbf58476d1ce4e5b9ULL;
  z = (z ^ (z >> 27))*0x94d049bb133111ebULL;
  return z ^ (z >> 31);
}

static inline uint64_t rotl64_24(uint64_t x) {
  return (x << 24)|(x >> 40);
}

static void sfc64_seed_counter(sfc64_state *s, uint64_t seed, uint64_t ctr) {
  uint64_t x = seed ^ (ctr + 0x9e3779b97f4a7c15ULL);
  s->a = splitmix64_next(&x);
  s->b = splitmix64_next(&x);
  s->c = splitmix64_next(&x);
  s->counter = ctr;
}

static void sfc64_4stream_seed(sfc64_4stream *st, uint64_t seed) {
  sfc64_seed_counter(&st->s0, seed, 1);
  sfc64_seed_counter(&st->s1, seed, 2);
  sfc64_seed_counter(&st->s2, seed, 3);
  sfc64_seed_counter(&st->s3, seed, 4);
}

static inline void u01_from_u64pair_53(double out[2], uint64_t r0, uint64_t r1) {
  uint64_t t[2];
  t[0] = r0;
  t[1] = r1;
  uint64x2_t r = vld1q_u64(t);
  r = vshrq_n_u64(r, 11);
  float64x2_t x = vcvtq_f64_u64(r);
  x = vmulq_n_f64(x, 0x1.0p-53);
  vst1q_f64(out, x);
}

static void fill_u01_randompack(double out[], int n, double param[], randompack_rng *rng) {
  size_t len = (size_t)n;
  (void)param;
  randompack_u01(out, len, rng);
}

static void fill_u01_local_4stream(double out[], int n, double param[], randompack_rng *rng) {
  size_t len = (size_t)n;
  (void)param;
  (void)rng;
  uint64_t a0 = st4.s0.a, b0 = st4.s0.b, c0 = st4.s0.c, k0 = st4.s0.counter;
  uint64_t a1 = st4.s1.a, b1 = st4.s1.b, c1 = st4.s1.c, k1 = st4.s1.counter;
  uint64_t a2 = st4.s2.a, b2 = st4.s2.b, c2 = st4.s2.c, k2 = st4.s2.counter;
  uint64_t a3 = st4.s3.a, b3 = st4.s3.b, c3 = st4.s3.c, k3 = st4.s3.counter;
  size_t i = 0;
  for (; i + 4 <= len; i += 4) {
    uint64_t t0 = a0 + b0 + k0++;
    uint64_t t1 = a1 + b1 + k1++;
    uint64_t t2 = a2 + b2 + k2++;
    uint64_t t3 = a3 + b3 + k3++;
    uint64_t na0 = b0 ^ (b0 >> 11);
    uint64_t nb0 = c0 + (c0 << 3);
    uint64_t nc0 = rotl64_24(c0) + t0;
    uint64_t na1 = b1 ^ (b1 >> 11);
    uint64_t nb1 = c1 + (c1 << 3);
    uint64_t nc1 = rotl64_24(c1) + t1;
    uint64_t na2 = b2 ^ (b2 >> 11);
    uint64_t nb2 = c2 + (c2 << 3);
    uint64_t nc2 = rotl64_24(c2) + t2;
    uint64_t na3 = b3 ^ (b3 >> 11);
    uint64_t nb3 = c3 + (c3 << 3);
    uint64_t nc3 = rotl64_24(c3) + t3;
    a0 = na0; b0 = nb0; c0 = nc0;
    a1 = na1; b1 = nb1; c1 = nc1;
    a2 = na2; b2 = nb2; c2 = nc2;
    a3 = na3; b3 = nb3; c3 = nc3;
    u01_from_u64pair_53(out + i, t0, t1);
    u01_from_u64pair_53(out + i + 2, t2, t3);
  }
  for (; i < len; i++) {
    int s = (int)(i & 3);
    uint64_t t;
    if (s == 0) { t = a0 + b0 + k0++; a0 = b0 ^ (b0 >> 11); b0 = c0 + (c0 << 3); c0 = rotl64_24(c0) + t; }
    else if (s == 1) { t = a1 + b1 + k1++; a1 = b1 ^ (b1 >> 11); b1 = c1 + (c1 << 3); c1 = rotl64_24(c1) + t; }
    else if (s == 2) { t = a2 + b2 + k2++; a2 = b2 ^ (b2 >> 11); b2 = c2 + (c2 << 3); c2 = rotl64_24(c2) + t; }
    else { t = a3 + b3 + k3++; a3 = b3 ^ (b3 >> 11); b3 = c3 + (c3 << 3); c3 = rotl64_24(c3) + t; }
    out[i] = (t >> 11)*0x1.0p-53;
  }
  st4.s0.a = a0; st4.s0.b = b0; st4.s0.c = c0; st4.s0.counter = k0;
  st4.s1.a = a1; st4.s1.b = b1; st4.s1.c = c1; st4.s1.counter = k1;
  st4.s2.a = a2; st4.s2.b = b2; st4.s2.c = c2; st4.s2.counter = k2;
  st4.s3.a = a3; st4.s3.b = b3; st4.s3.c = c3; st4.s3.counter = k3;
}

static void warmup_min_time(double seconds) {
  double t0 = get_time();
  while (get_time() - t0 < seconds) warmup_cpu(10);
}

int main(void) {
  const char *engine = "sfc64";
  const int chunk = 4096;
  const double warmup_time = 0.2;
  const double bench_time = 0.8;
#if defined(__linux__)
  pin_to_cpu0();
#endif
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
  double param[1] = {0};
  double ns_ref = time_double(chunk, bench_time, fill_u01_randompack, param, rng);
  printf("u01 (randompack %s): %.3f ns\n", engine, ns_ref);
  sfc64_4stream_seed(&st4, 7);
  double ns_local = time_double(chunk, bench_time, fill_u01_local_4stream, param, rng);
  printf("u01 (local sfc64 4-stream): %.3f ns\n", ns_local);
  randompack_free(rng);
  return 0;
}
