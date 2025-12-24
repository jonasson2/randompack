// -*- C -*-
// Timing benchmark: throughput (GB/s) across distributions for randompack engines.

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
  double gbps[DIST_COUNT];
  double secs[DIST_COUNT];
  bool ok[DIST_COUNT];
} row_t;

static volatile uint64_t sink;

static inline void consume_double(double x) {
  uint64_t u;
  memcpy(&u, &x, sizeof(u));   // bitwise, no FP work
  sink ^= u;
}

static double bench_randompack(char *engine, int seed, int n_total, int chunk,
  int dist, double *secs) {
  randompack_rng *rng = randompack_create(engine);
  ASSERT(rng);
  ASSERT(randompack_seed(seed, 0, 0, rng));
  double t0 = get_time();
  if (dist == DIST_UINT64) {
    uint64_t *buf;
    TEST_ALLOC(buf, chunk);
    int produced = 0;
    while (produced < n_total) {
      int take = n_total - produced;
      if (take > chunk) take = chunk;
      bool ok = randompack_uint64(buf, take, 0, rng);
      ASSERT(ok);
      produced += take;
    }
    FREE(buf);
  }
  else {
    double *buf;
    TEST_ALLOC(buf, chunk);
    int produced = 0;
    while (produced < n_total) {
      int take = n_total - produced;
      if (take > chunk) take = chunk;
      bool ok = false;
      if (dist == DIST_U01) ok = randompack_u01(buf, take, rng);
      else if (dist == DIST_NORMAL) ok = randompack_norm(buf, take, rng);
      else if (dist == DIST_EXP) ok = randompack_exp(buf, take, rng);
      else ASSERT(false);
      ASSERT(ok);
      consume_double(buf[take-1]);
      produced += take;
    }
    FREE(buf);
  }
  double t1 = get_time();
  randompack_free(rng);
  double dt = t1 - t0;
  if (secs) *secs = dt;
  double bytes = 8.0*n_total;
  return (bytes/1e9)/dt; // GB/s (decimal)
}

static volatile uint64_t direct_sink;

typedef void (*fill_x256pp_double)(double *buf, int n, uint64_t *s0,
  uint64_t *s1, uint64_t *s2, uint64_t *s3, uint64_t *accum);

typedef void (*fill_x256pp_u64)(uint64_t *buf, int n, uint64_t *s0,
  uint64_t *s1, uint64_t *s2, uint64_t *s3, uint64_t *accum);

static double bench_direct_x256pp_double(int n_total, double *secs,
  fill_x256pp_double fill) {
  uint64_t s0 = 1, s1 = 2, s2 = 3, s3 = 4;
  uint64_t accum = 0;
  double *buf = 0;
  TEST_ALLOC(buf, BUFSIZE);
  double t0 = get_time();
  int produced = 0;
  while (produced < n_total) {
    int take = n_total - produced;
    if (take > BUFSIZE) take = BUFSIZE;
    fill(buf, take, &s0, &s1, &s2, &s3, &accum);
    produced += take;
  }
  double t1 = get_time();
  direct_sink ^= accum ^ s0 ^ s1 ^ s2 ^ s3;
  FREE(buf);
  double dt = t1 - t0;
  if (secs) *secs = dt;
  double bytes = 8.0*n_total;
  return (bytes/1e9)/dt;
}

static double bench_direct_x256pp_u64fill(int n_total, double *secs,
  fill_x256pp_u64 fill) {
  uint64_t s0 = 1, s1 = 2, s2 = 3, s3 = 4;
  uint64_t accum = 0;
  uint64_t *buf = 0;
  TEST_ALLOC(buf, BUFSIZE);
  double t0 = get_time();
  int produced = 0;
  while (produced < n_total) {
    int take = n_total - produced;
    if (take > BUFSIZE) take = BUFSIZE;
    fill(buf, take, &s0, &s1, &s2, &s3, &accum);
    produced += take;
  }
  double t1 = get_time();
  direct_sink ^= accum ^ s0 ^ s1 ^ s2 ^ s3;
  FREE(buf);
  double dt = t1 - t0;
  if (secs) *secs = dt;
  double bytes = 8.0*n_total;
  return (bytes/1e9)/dt;
}

static double bench_direct_x256pp_exp(int n_total, double *secs) {
  return bench_direct_x256pp_double(n_total, secs, fill_x256pp_exp);
}

static double bench_direct_x256pp_norm(int n_total, double *secs) {
  return bench_direct_x256pp_double(n_total, secs, fill_x256pp_norm_polar);
}

static double bench_direct_x256pp_u64(int n_total, double *secs) {
  return bench_direct_x256pp_u64fill(n_total, secs, fill_x256pp_u64);
}

static double bench_direct_x256pp_u01(int n_total, double *secs) {
  return bench_direct_x256pp_double(n_total, secs, fill_x256pp_u01);
}

static void print_cell(double secs, bool ok, int n_total) {
  if (!ok) printf(" %8s", "n/a");
  else printf(" %8.2f", secs * 1.0e9 / n_total);
}

int main(int argc, char **argv) {
  // Adjust if you want longer/shorter runs.
  int seed = 7;
  int n_total = M/8;
  int chunk   = 256;
  if (argc > 1) {
    int v = atoi(argv[1]);
    if (v > 0) chunk = v;
  }

  double t1 = get_time();
  warmup_cpu(100);
  double t2 = get_time();
  printf("warm-up time:     %.6f s\n", t2 - t1);
  printf("latency:          ns/value\n");

  row_t rows[32];

  char *dist_names[DIST_COUNT] = {
    "uint64",
    "u01",
    "normal",
    "exp",
  };

  struct {
    char *name;
    bool direct;
  } engines[] = {
    { "xoshiro256++ (direct)", true  },
    { "xoshiro256++",          false },
    { "xoshiro256**",          false },
    { "xorshift128+",          false },
    { "pcg64",                 false },
    { "philox",                false },
    { "system",                false },
    { "chacha20",              false },
  };
  int n_engines = LEN(engines);

  for (int i = 0; i < n_engines; i++) {
    rows[i].name = engines[i].name;
    for (int j = 0; j < DIST_COUNT; j++) {
      double secs = 0.0;
      double gbps = -1.0;
      if (engines[i].direct) {
        if (j == DIST_UINT64) {
          gbps = bench_direct_x256pp_u64(n_total, &secs);
        }
        else if (j == DIST_U01) {
          gbps = bench_direct_x256pp_u01(n_total, &secs);
        }
        else if (j == DIST_NORMAL) {
          gbps = bench_direct_x256pp_norm(n_total, &secs);
        }
        else if (j == DIST_EXP) {
          gbps = bench_direct_x256pp_exp(n_total, &secs);
        }
      }
      else {
        gbps = bench_randompack(engines[i].name, seed, n_total, chunk,
          j, &secs);
      }
      rows[i].gbps[j] = gbps;
      rows[i].secs[j] = secs;
      rows[i].ok[j] = gbps >= 0;
    }
  }

  printf("\n%-22s", "Engine");
  for (int j = 0; j < DIST_COUNT; j++) {
    printf(" %8s", dist_names[j]);
  }
  printf("\n");
  for (int i = 0; i < n_engines; i++) {
    printf("%-22s", rows[i].name);
    for (int j = 0; j < DIST_COUNT; j++) {
      print_cell(rows[i].secs[j], rows[i].ok[j], n_total);
    }
    printf("\n");
  }
  printf("\n");

  return 0;
}
