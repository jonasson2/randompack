#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "Util.h"
#include "randompack.h"
#include "randompack_config.h"
#include "pcg64.h"

static inline pcg_ulong_t splitmix128(uint64_t *state) {
  uint64_t lo = rand_splitmix64(state);
  uint64_t hi = rand_splitmix64(state);
  return ((pcg_ulong_t)hi << 64) | lo;
}

static inline void pcg64_local(uint64_t *restrict out, int n,
  pcg64_t *state) {
  uint64_t mul = 15750249268501108917ULL;
  pcg_ulong_t st = state->state;
  pcg_ulong_t inc = state->inc;
  for (int i = 0; i < n; i++) {
    pcg_ulong_t s = st;
    st = s*mul + inc;
    uint64_t hi = s >> 64;
    uint64_t lo = s | 1;
    hi ^= hi >> 32;
    hi *= mul;
    hi ^= hi >> 48;
    hi *= lo;
    out[i] = hi;
  }
  state->state = st;
  state->inc = inc;
}

void pcg64_init(pcg64_t *state, uint64_t seed) {
  uint64_t sm = seed;
  state->state = splitmix128(&sm);
  state->inc = splitmix128(&sm) | 1;
}

static double bench_randompack(int nbuf, int reps, int seed) {
  double t0, t1;
  randompack_rng *rng = randompack_create("pcg64_dxsm");
  ASSERT(rng);
  ASSERT(randompack_seed(seed, 0, 0, rng));
  uint64_t *buf;
  TEST_ALLOC(buf, nbuf);
  t0 = get_time();
  uint64_t s = 0;
  for (int rep = 0; rep < reps; rep++) {
	 randompack_uint64(buf, nbuf, 0, rng);
  }
  printf("s = %d", (int)s);
  t1 = get_time();
  FREE(buf);
  randompack_free(rng);
  return t1 - t0;
}

static void benchmark_bulk_array(int nbuf, int reps) {
  double t1, t2, t3, t4;
  pcg64_t state;
  pcg64_init(&state, 12345);
  int rp_seed = 12345;
  uint64_t spin = 0, last = 0;
  double p = 0.0;
  uint64_t *buf = 0, *buf2 = 0;
  TEST_ALLOC(buf, nbuf);
  TEST_ALLOC(buf2, nbuf);

  t1 = get_time();
  warmup_cpu(100);
  t2 = get_time();

  // Benchmark randompack
  double rp_dt = bench_randompack(nbuf, reps, rp_seed);
  t3 = t2 + rp_dt;

  // Benchmark local pcg64_dxsm
  for (int rep = 0; rep < reps; rep++) {
    pcg64_local(buf, nbuf, &state);
    //last ^= buf[nbuf-1];
  }
  last ^= buf[nbuf-1];
  t4 = get_time();

  double total_bytes = 8.0*nbuf*reps;
  double local_dt = t4 - t3;
  double randompackspeed = total_bytes/1e9/rp_dt;
  double localspeed = total_bytes/1e9/local_dt;
  double t5 = get_time();
  for (int rep = 0; rep < reps; rep++) memcpy(buf2, buf, nbuf*8);
  double t6 = get_time();
  double memcpy_dt = t6 - t5;
  double memcpy_speed = total_bytes/1e9/memcpy_dt;
  printf("spin + last = %" PRIu64 "\n", spin + last);
  printf("p = %.4f\n", p);
  printf("warm-up time:     %.6f s\n", t2 - t1);
  printf("randompack speed: %.2f GB/s\n", randompackspeed);
  printf("local fn speed:   %.2f GB/s\n", localspeed);
  printf("memcpy speed:     %.2f GB/s\n", memcpy_speed);
  printf("randompack time:  %.6f s\n", rp_dt);
  printf("local fn time:    %.6f s\n", local_dt);
  printf("memcpy time:      %.6f s\n", memcpy_dt);
  FREE(buf);
  FREE(buf2);
}

int main(int argc, char **argv) {
  const int K = 1024, M = K*K; 
  int nbuf = argc > 1 ? strtoull(argv[1], 0, 10) : 256;
  if (nbuf <= 0) nbuf = 256;
  long long ntotal = argc > 2 ? strtoull(argv[2], 0, 10) : 256*M;
  if (ntotal < nbuf) ntotal = nbuf;
  int reps = ntotal/nbuf;
  if (reps < 1) reps = 1;
  ntotal = (long long)reps*nbuf;
  printf("nbuf:   %d K\n", nbuf/1000);
  printf("reps:   %d K\n", reps/1000);
  printf("ntotal: %.0f\n", ntotal/1e9);
  benchmark_bulk_array(nbuf, reps);
  return 0;
}
