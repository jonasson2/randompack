#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define PREVENT_OPTIMIZE(x) __asm__ __volatile__("" : : "r,m"(x) : "memory")

typedef struct { uint64_t s[4]; } xoshiro256ss_state;

#define rotl(x, k) (((x) << (k)) | ((x) >> (64 - (k))))
// static inline uint64_t rotl(uint64_t x, int k) {
//   return (x << k) | (x >> (64 - k));
// }

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

void xoshiro256ss_init(xoshiro256ss_state *state, uint64_t seed) {
  state->s[0] = seed;
  state->s[1] = seed*0x9e3779b97f4a7c15;
  state->s[2] = seed + 0x6a09e667f3bcc908;
  state->s[3] = seed ^ 0xbb67ae8584caa73b;
  for (int i=0; i<16; i++) xoshiro256ss(state);
}

double get_time(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec + ts.tv_nsec/1e9;
}

void benchmark_bulk_array(int nbuf, int reps) {
  double t1, t2, t3;
  xoshiro256ss_state state;
  xoshiro256ss_init(&state, 12345);
  
  unsigned long long spin, last;
  uint64_t *buffer = malloc(nbuf*sizeof(uint64_t));
  
  // Long warmup to get CPU to full speed (especially important on ARM)
  t1 = get_time();
  int warmupreps = 1e8/nbuf;
  for (int w = 0; w < warmupreps; w++) {
    for (int i = 0; i < nbuf; i++) {
      spin = xoshiro256ss(&state);
    }
  }
  t2 = get_time();
  double twarmup = t2 - t1;
  double p = 0;
  // Benchmark
  double x = 0;
  for (int rep = 0; rep < reps; rep++) {
    for (int i = 0; i < nbuf; i++) {
      last = xoshiro256ss(&state);
      //p += sqrt((double)last);
    }
  }
  t3 = get_time();
  printf("spin + last = %llu\n", spin + last);
  printf("p = %.4f\n", p);
  double total_bytes = 8.0*nbuf*reps;
  double gps = total_bytes/1e9/(t3 - t2);
  printf("Warm-up time: %.2f s\n", t2 - t1);
  printf("Speed:        %.2f GB/s\n", gps);
  printf("Time:         %.2f s\n", t3 - t2);
  free(buffer);
}

int main(int argc, char **argv) {
  int nbuf = (argc > 1) ? strtoull(argv[1], 0, 10) : 1000000;
  const int ntotal = 1e9;
  int nreps = ntotal/nbuf;
  printf("nbuf:   %d K\n", nbuf/1000);
  printf("ntotal: %d K\n", ntotal/1000);  
  benchmark_bulk_array(nbuf, nreps);
  return 0;
}
