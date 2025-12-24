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

#define K 1024
#define M (K*K)
#define G (K*M)

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

static volatile uint64_t bench_sink;

static inline void consume_8bytes(const void *p) {
  uint64_t u;
  memcpy(&u, p, sizeof(u));
  bench_sink ^= u;
}

#define BENCH_REFERENCE(T, BUFNAME, N_TOTAL, FILL_FN) do {  \
   uint64_t s0 = 1, s1 = 2, s2 = 3, s3 = 4;                 \
   T *BUFNAME = 0;                                          \
   TEST_ALLOC(BUFNAME, BUFSIZE);                            \
   double t0 = get_time();                                  \
   int produced = 0;                                        \
   while (produced < (N_TOTAL)) {                           \
     int take = (N_TOTAL) - produced;                       \
     if (take > BUFSIZE) take = BUFSIZE;                    \
     (FILL_FN)(BUFNAME, take, &s0, &s1, &s2, &s3);          \
     consume_8bytes(&BUFNAME[take - 1]);                    \
     produced += take;                                      \
   }                                                        \
   double t1 = get_time();                                  \
   FREE(BUFNAME);                                           \
   return (1.0e9*(t1 - t0))/(double)(N_TOTAL);              \
 } while (0)

static double bench_reference_exp(int n_total) {
  BENCH_REFERENCE(double, buf, n_total, fill_x256pp_exp);
}

static double bench_reference_norm(int n_total) {
  BENCH_REFERENCE(double, buf, n_total, fill_x256pp_norm_polar);
}

static double bench_reference_u01(int n_total) {
  BENCH_REFERENCE(double, buf, n_total, fill_x256pp_u01);
}

static double bench_reference_u64(int n_total) {
  BENCH_REFERENCE(uint64_t, buf, n_total, fill_x256pp_u64);
}

static double bench_randompack_ns(char *engine, int seed, int n_total, int chunk,
  int dist) {
  randompack_rng *rng = randompack_create(engine);
  ASSERT(rng);
  ASSERT(randompack_seed(seed, 0, 0, rng));
  uint64_t *buf_u64 = 0;
  double *buf_d = 0;
  if (dist == DIST_UINT64) TEST_ALLOC(buf_u64, chunk);
  else TEST_ALLOC(buf_d, chunk);
  double t0 = get_time();
  int produced = 0;
  int last_take = 0;
  while (produced < n_total) {
    int take = n_total - produced;
    if (take > chunk) take = chunk;
    bool ok = false;
    if (dist == DIST_UINT64) ok = randompack_uint64(buf_u64, take, 0, rng);
    else if (dist == DIST_U01) ok = randompack_u01(buf_d, take, rng);
    else if (dist == DIST_NORMAL) ok = randompack_norm(buf_d, take, rng);
    else if (dist == DIST_EXP) ok = randompack_exp(buf_d, take, rng);
    else ASSERT(false);
    ASSERT(ok);
    produced += take;
    last_take = take;
  }

  double t1 = get_time();
  if (dist == DIST_UINT64) {
    consume_8bytes(&buf_u64[last_take - 1]);
    FREE(buf_u64);
  } else {
    consume_8bytes(&buf_d[last_take - 1]);
    FREE(buf_d);
  }
  randompack_free(rng);
  return (1.0e9*(t1 - t0))/(double)n_total;
}

static void print_cell(double ns, bool ok) {
  if (!ok) printf(" %8s", "n/a");
  else printf(" %8.2f", ns);
}

int main(int argc, char **argv) {
  int seed = 7;
  int n_total = M/8;
  int chunk = 256;
  if (argc > 1) {
    int v = atoi(argv[1]);
    if (v > 0) chunk = v;
  }
  double t1 = get_time();
  warmup_cpu(100);
  double t2 = get_time();
  printf("warm-up time:     %.6f s\n", t2 - t1);
  printf("latency:          ns/value\n");
  printf("n_total:          %d values per (engine,dist)\n", n_total);
  printf("chunk:            %d\n", chunk);
  row_t rows[32];
  char *dist_names[DIST_COUNT] = {
    "uint64",
    "u01",
    "normal",
    "exp",
  };
  struct {
    char *name;
    bool reference;
  } engines[] = {
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
  for (int i = 0; i < n_engines; i++) {
    rows[i].name = engines[i].name;
    for (int d = 0; d < DIST_COUNT; d++) {
      double ns = 0.0;
      bool ok = true;
      if (engines[i].reference) {
        if (d == DIST_UINT64) ns = bench_reference_u64(n_total);
        else if (d == DIST_U01) ns = bench_reference_u01(n_total);
        else if (d == DIST_NORMAL) ns = bench_reference_norm(n_total);
        else if (d == DIST_EXP) ns = bench_reference_exp(n_total);
        else ok = false;
      } else {
        ns = bench_randompack_ns(engines[i].name, seed, n_total, chunk, d);
      }
      rows[i].ns[d] = ns;
      rows[i].ok[d] = ok && (ns > 0.0);
    }
  }
  printf("\n%-26s", "Engine");
  for (int d = 0; d < DIST_COUNT; d++) printf(" %8s", dist_names[d]);
  printf("\n");
  for (int i = 0; i < n_engines; i++) {
    printf("%-26s", rows[i].name);
    for (int d = 0; d < DIST_COUNT; d++) print_cell(rows[i].ns[d], rows[i].ok[d]);
    printf("\n");
  }
  printf("\n");
  return 0;
}
