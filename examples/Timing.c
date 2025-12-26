// -*- C -*-
// Timing benchmark: latency (ns/value) across distributions for randompack engines,
// with reference xoshiro256++ kernels for comparison.

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
#include "reference_rng.h"

enum {
  DIST_UINT64,
  DIST_U01,
  DIST_NORMAL,
  DIST_EXP,
  DIST_COUNT
};

typedef struct {
  char *name;
  double ns[DIST_COUNT];
  bool ok[DIST_COUNT];
} row_t;

static inline void consume64(const void *p) {
  static volatile uint64_t sink;
  uint64_t u;
  memcpy(&u, p, sizeof(u));
  sink ^= u;
}

#define BENCH_REFERENCE_TO(OUT, TYPE, CHUNK, BENCH_TIME, REPS, FILL_FN) do {  \
   uint64_t s0 = 1, s1 = 2, s2 = 3, s3 = 4;                                   \
   int calls = 0;                                                             \
   double t0 = get_time(), t = t0;                                            \
   TYPE *buf = 0;                                                             \
   TEST_ALLOC(buf, (CHUNK));                                                  \
   while (t - t0 <= (BENCH_TIME)) {                                           \
     for (int i=0; i<(REPS); i++) {                                           \
       (FILL_FN)(buf, (CHUNK), &s0, &s1, &s2, &s3);                           \
       consume64(&buf[(CHUNK) - 1]);                                          \
     }                                                                        \
     t = get_time();                                                          \
     calls += (REPS);                                                         \
   }                                                                          \
   FREE(buf);                                                                 \
   (OUT) = 1e9*(t - t0)/(calls*(CHUNK));                                      \
 } while (0)

static double bench_randompack_ns(char *engine, int seed, int chunk, int dist,
  double bench_time, int reps) {
  randompack_rng *rng = randompack_create(engine);
  ASSERT(rng);
  ASSERT(randompack_seed(seed, 0, 0, rng));
  uint64_t *buf_u64 = 0;
  double *buf_dble = 0;
  if (dist == DIST_UINT64)
    TEST_ALLOC(buf_u64, chunk);
  else
    TEST_ALLOC(buf_dble, chunk);
  int calls = 0;
  double t0 = get_time(), t = t0;
  while (t - t0 < bench_time) {
    for (int i=0; i<reps; i++) {
      if (dist == DIST_UINT64)      randompack_uint64(buf_u64, chunk, 0, rng);
      else if (dist == DIST_U01)    randompack_u01   (buf_dble, chunk, rng);
      else if (dist == DIST_NORMAL) randompack_norm  (buf_dble, chunk, rng);
      else if (dist == DIST_EXP)    randompack_exp   (buf_dble, chunk, 1.0, rng);
      if (dist == DIST_UINT64) consume64(&buf_u64[chunk - 1]);
      else                     consume64(&buf_dble[chunk - 1]);
    }
    calls += reps;
    t = get_time();
  }
  FREE(buf_u64);
  FREE(buf_dble);
  randompack_free(rng);
  return 1e9*(t - t0)/(calls*chunk);
}

static void print_cell(double ns, bool ok) {
  if (!ok)
    printf(" %8s", "n/a");
  else
    printf(" %8.2f", ns);
}

int main(int argc, char **argv) {
  int seed = 7;
  int chunk = 256;
  double bench_time = 0.1;
  if (argc > 1) {
    int v = atoi(argv[1]);
    if (v > 0)
      chunk = v;
  }
  if (argc > 2) {
    double v = atof(argv[2]);
    if (v > 0.0)
      bench_time = v;
  }
  int reps = max(1, 1000000/chunk);
  double t1 = get_time();
  warmup_cpu(100);
  double t2 = get_time();
  printf("warm-up time:     %.6f s\n", t2 - t1);
  printf("latency:          ns/value\n");
  printf("bench_time:       %.3f s per (engine,dist)\n", bench_time);
  printf("chunk:            %d\n", chunk);
  printf("reps/check:       %d\n", reps);
  row_t rows[32];
  char *dist_names[DIST_COUNT] = { "uint64", "u01", "normal", "exp" };
  struct { char *name; bool reference; } engines[] = {
    { "xoshiro256++ (reference)", true  },
    { "xoshiro256++",             false },
    { "xoshiro256**",             false },
    { "xorshift128+",             false },
    { "pcg64",                    false },
    { "philox",                   false },
    { "system",                   false },
    { "chacha20",                 false },
  };
  int n_engines = LEN(engines);
  for (int i=0; i<n_engines; i++) {
    rows[i].name = engines[i].name;
    for (int d=0; d<DIST_COUNT; d++) {
      double ns = 0.0;
      bool ok = true;
      if (engines[i].reference) {
        if (d == DIST_UINT64)
          BENCH_REFERENCE_TO(ns, uint64_t, chunk, bench_time, reps, fill_x256pp_u64);
        else if (d == DIST_U01)
          BENCH_REFERENCE_TO(ns, double, chunk, bench_time, reps, fill_x256pp_u01);
        else if (d == DIST_NORMAL)
          BENCH_REFERENCE_TO(ns, double, chunk, bench_time, reps, fill_x256pp_norm_polar);
        else if (d == DIST_EXP)
          BENCH_REFERENCE_TO(ns, double, chunk, bench_time, reps, fill_x256pp_exp);
        else
          ok = false;
      }
      else
        ns = bench_randompack_ns(engines[i].name, seed, chunk, d, bench_time, reps);
      rows[i].ns[d] = ns;
      rows[i].ok[d] = ok && (ns > 0.0);
    }
  }
  printf("\n%-26s", "Engine");
  for (int d=0; d<DIST_COUNT; d++)
    printf(" %8s", dist_names[d]);
  printf("\n");
  for (int i=0; i<n_engines; i++) {
    printf("%-26s", rows[i].name);
    for (int d=0; d<DIST_COUNT; d++)
      print_cell(rows[i].ns[d], rows[i].ok[d]);
    printf("\n");
  }
  printf("\n");
  return 0;
}
