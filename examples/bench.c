#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

typedef struct { uint64_t s[4]; } xoshiro256ss_state;

#include "randompack.h"
#include "randompack_config.h"

static inline uint64_t rotl(uint64_t x, int k) {
  return (x << k) | (x >> (64 - k));
}

static inline uint64_t xoshiro256ss(xoshiro256ss_state *state) {
  uint64_t result = rotl(state->s[1]*5, 7)*9;
  uint64_t t = state->s[1] << 17;
  state->s[2] ^= state->s[0];
  state->s[3] ^= state->s[1];
  state->s[1] ^= state->s[2];
  state->s[0] ^= state->s[3];
  state->s[2] ^= t;
  state->s[3] = rotl(state->s[3], 45);
  return result;
}

static inline void fill_local(uint64_t *out, int n, xoshiro256ss_state *state) {
  uint64_t s0 = state->s[0], s1 = state->s[1], s2 = state->s[2], s3 = state->s[3];
  for (int i = 0; i < n; i++) {
    uint64_t t = s1 << 17;
    uint64_t result = rotl(s1*5, 7)*9;
    s2 ^= s0;
    s3 ^= s1;
    s1 ^= s2;
    s0 ^= s3;
    s2 ^= t;
    s3 = rotl(s3, 45);
    out[i] = result;
  }
  state->s[0] = s0;
  state->s[1] = s1;
  state->s[2] = s2;
  state->s[3] = s3;
}

void xoshiro256ss_init(xoshiro256ss_state *state, uint64_t seed) {
  state->s[0] = seed;
  state->s[1] = seed*0x9e3779b97f4a7c15;
  state->s[2] = seed + 0x6a09e667f3bcc908;
  state->s[3] = seed ^ 0xbb67ae8584caa73b;
  for (int i = 0; i < 16; i++) xoshiro256ss(state);
}

static double get_time(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (double)ts.tv_sec + (double)ts.tv_nsec/1e9;
}

static void benchmark_bulk_array(int nbuf, int reps) {
  double t1, t2, t3, t4;
  xoshiro256ss_state state;
  xoshiro256ss_init(&state, 12345);
  randompack_rng *rng = randompack_create("xoshiro256**");
  assert(rng);
  uint64_t spin = 0, last = 0;
  double p = 0.0;
  bool ok;
  uint64_t *buf;
  assert(ALLOC(buf, nbuf));

  int warmupreps = max(1, (int)1e8/nbuf);

  // Long warmup to get CPU to full speed (especially important on ARM)
  t1 = get_time();
  for (int w = 0; w < warmupreps; w++) {
    fill_local(buf, nbuf, &state);
    spin = buf[nbuf - 1];
  }
  t2 = get_time();

  // Benchmark local xoshiro256**
  for (int rep = 0; rep < reps; rep++) {
    fill_local(buf, nbuf, &state);
    last ^= buf[nbuf-1];
  }
  t3 = get_time();

  // Benchmark randompack
  for (int rep = 0; rep < reps; rep++) {
    ok = randompack_uint64(buf, nbuf, 0, rng);
    assert(ok);
    last ^= buf[nbuf - 1];
  }
  t4 = get_time();

  double total_bytes = 8.0*nbuf*reps;
  double randompackspeed = total_bytes/1e9/(t3 - t2);
  double localspeed = total_bytes/1e9/(t4 - t3);
  printf("spin + last = %" PRIu64 "\n", (uint64_t)(spin + last));
  printf("p = %.4f\n", p);
  printf("warm-up time:     %.2f s\n", t2 - t1);
  printf("randompack speed: %.2f GB/s\n", randompackspeed);
  printf("local fn speed:   %.2f GB/s\n", localspeed);
  printf("randompack time:  %.2f s\n", t3 - t2);
  printf("local fn time:    %.2f s\n", t4 - t3);
  FREE(buf);
  randompack_free(rng);
}

int main(int argc, char **argv) {
  int nbuf = argc > 1 ? (int)strtoull(argv[1], 0, 10) : 1000000;
  if (nbuf <= 0) nbuf = (int)1e6;
  const long long ntotal = (long long)1e9;
  int reps = ntotal/nbuf;
  if (argc > 2) reps = (int)strtoull(argv[2], 0, 10);
  if (reps < 1) reps = 1;
  printf("nbuf:   %d K\n", nbuf/1000);
  printf("reps:   %d K\n", reps/1000);
  printf("ntotal: %d G\n", (int)(ntotal/1e9));
  benchmark_bulk_array(nbuf, reps);
  return 0;
}
