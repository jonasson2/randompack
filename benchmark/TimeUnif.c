// -*- C -*-
// TimeUnif.c: dedicated timing of randompack_u01 and randompack_unif.

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "getopt.h"

#include "TimeUtil.h"
#include "Util.h"
#include "randompack.h"
#include "randompack_config.h"

static void print_help(void) {
  printf("TimeUnif — dedicated timing of uniform doubles\n");
  printf("Usage: TimeUnif [options]\n\n");
  printf("Options:\n");
  printf("  -h            Show this help message\n");
  printf("  -e engine     RNG engine (default x256++simd)\n");
  printf("  -t seconds    Benchmark time per case (default 0.2)\n");
  printf("  -c chunk      Chunk size (default 4096)\n");
  printf("  -s seed       Fixed RNG seed (default random seed per case)\n");
  printf("  -d digits     Decimal places (default 2)\n");
  printf("  -b            Use bitexact log/exp implementations\n");
}

static bool get_options(int argc, char **argv, char **engine, double *bench_time,
  int *chunk, int *seed, bool *have_seed, int *digits, bool *bitexact, bool *help) {
  opterr = 0;
  optind = 1;
  int opt;
  *engine = "x256++simd";
  *bench_time = 0.2;
  *chunk = 4096;
  *seed = 0;
  *have_seed = false;
  *digits = 2;
  *bitexact = false;
  *help = false;
  while ((opt = getopt(argc, argv, "he:t:c:s:d:b")) != -1) {
    switch (opt) {
      case 'h':
        *help = true;
        return true;
      case 'e':
        *engine = optarg;
        break;
      case 't':
        *bench_time = atof(optarg);
        if (*bench_time <= 0)
          return false;
        break;
      case 'c':
        *chunk = atoi(optarg);
        if (*chunk <= 0)
          return false;
        break;
      case 's':
        *seed = atoi(optarg);
        *have_seed = true;
        break;
      case 'd':
        *digits = atoi(optarg);
        if (*digits < 0)
          return false;
        break;
      case 'b':
        *bitexact = true;
        break;
      default:
        return false;
    }
  }
  if (optind < argc)
    return false;
  return true;
}

static inline void consume_double(const double *buf, int chunk) {
  static volatile double sink;
  sink += buf[chunk-1];
}

static int compute_reps(int chunk) {
  int reps = (int)(1000000/chunk);
  return max(1, reps);
}

#define TIME_DOUBLE_CALL(expr) do { \
  double *buf; \
  TEST_ALLOC(buf, chunk); \
  int reps = compute_reps(chunk); \
  int64_t calls = 0; \
  uint64_t t0 = clock_nsec(); \
  uint64_t t = t0; \
  uint64_t limit = (uint64_t)(bench_time*1e9); \
  while ((t - t0) < limit) { \
    for (int i = 0; i < reps; i++) { \
      ASSERT(expr); \
      consume_double(buf, chunk); \
      calls++; \
    } \
    t = clock_nsec(); \
  } \
  FREE(buf); \
  return (calls > 0) ? (t - t0)/((double)calls*chunk) : 0; \
} while (0)

static double time_u01_direct(int chunk, double bench_time, randompack_rng *rng) {
  TIME_DOUBLE_CALL(randompack_u01(buf, (size_t)chunk, rng));
}

static double time_unif_direct(int chunk, double bench_time, randompack_rng *rng) {
  double a = 2;
  double b = 5;
  TIME_DOUBLE_CALL(randompack_unif(buf, (size_t)chunk, a, b, rng));
}

static void warm_u01(randompack_rng *rng) {
  double x[4];
  ASSERT(randompack_u01(x, 4, rng));
}

static void warm_unif(randompack_rng *rng) {
  double x[4];
  ASSERT(randompack_unif(x, 4, 2, 5, rng));
}

static void set_seed(randompack_rng *rng, int seed, bool have_seed) {
  bool ok;
  if (have_seed)
    ok = randompack_seed(seed, 0, 0, rng);
  else
    ok = randompack_randomize(rng);
  ASSERT(ok);
}

int main(int argc, char **argv) {
  char *engine;
  double bench_time;
  int chunk;
  int seed;
  int digits;
  bool have_seed;
  bool bitexact;
  bool help;
  if (!get_options(argc, argv, &engine, &bench_time, &chunk, &seed, &have_seed,
      &digits, &bitexact, &help) || help) {
    print_help();
    return help ? 0 : 1;
  }
  randompack_rng *rng = randompack_create(engine);
  if (!rng) {
    fprintf(stderr, "randompack_create failed: %s\n", engine);
    return 1;
  }
  if (bitexact && !randompack_bitexact(rng, true)) {
    fprintf(stderr, "randompack_bitexact failed\n");
    randompack_free(rng);
    return 1;
  }
  warmup_cpu(0.1);
  printf("engine:           %s\n", engine);
  printf("time per value:   ns/value\n");
  printf("bench_time:       %.3f s per case\n", bench_time);
  printf("chunk:            %d\n\n", chunk);
  printf("%-18s %8s\n", "Distribution", "double");
  set_seed(rng, seed, have_seed);
  warm_u01(rng);
  set_seed(rng, seed, have_seed);
  printf("%-18s %8.*f\n", "u01", digits,
    time_u01_direct(chunk, bench_time, rng));
  set_seed(rng, seed, have_seed);
  warm_unif(rng);
  set_seed(rng, seed, have_seed);
  printf("%-18s %8.*f\n", "unif(2,5)", digits,
    time_unif_direct(chunk, bench_time, rng));
  randompack_free(rng);
  return 0;
}
