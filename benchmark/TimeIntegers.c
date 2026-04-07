// -*- C -*-
// TimeIntegers.c: time integer draws and permutations (ns/value).
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "getopt.h"
#include "TimeUtil.h"
#include "Util.h"
#include "randompack.h"
#include "randompack_config.h"
static void print_help(void) {
  printf("TimeIntegers - time integer draws and permutations (ns/value)\n");
  printf("Usage: TimeIntegers [options]\n\n");
  printf("Options:\n");
  printf("  -h            Show this help message\n");
  printf("  -e engine     RNG engine (default x256++simd)\n");
  printf("  -t seconds    Benchmark time per case (default 0.1)\n");
  printf("  -c chunk      Chunk size (values per call, default 4096)\n");
  printf("  -s seed       Fixed RNG seed (default random seed per case)\n");
}

static bool get_options(int argc, char **argv, char **engine, double *bench_time,
                        int *chunk, int *seed, bool *have_seed, bool *help) {
  opterr = 0;
  optind = 1;
  int opt;
  *engine = "x256++simd";
  *bench_time = 0.1;
  *chunk = 4096;
  *seed = 0;
  *have_seed = false;
  *help = false;
  while ((opt = getopt(argc, argv, "he:t:c:s:")) != -1) {
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
      default:
        return false;
    }
  }
  if (optind < argc)
    return false;
  return true;
}

static void set_seed(randompack_rng *rng, int seed, bool have_seed) {
  bool ok;
  if (have_seed)
    ok = randompack_seed(seed, 0, 0, rng);
  else
    ok = randompack_randomize(rng);
  ASSERT(ok);
}

static inline void consume_u64(uint64_t x) {
  static volatile uint64_t sink;
  sink ^= x;
}

static double time_int_range(int chunk, double bench_time, int m, int n, randompack_rng
                             *rng) {
  int *buf;
  TEST_ALLOC(buf, chunk);
  int reps = 1000000/chunk;
  if (reps < 1)
    reps = 1;
  int calls = 0;
  uint64_t t0 = clock_nsec();
  uint64_t deadline = t0 + (uint64_t)(bench_time*1e9);
  uint64_t t = t0;
  while (t < deadline) {
    for (int i = 0; i < reps; i++) {
      ASSERT(randompack_int(buf, chunk, m, n, rng));
      consume_u64(buf[chunk - 1]);
    }
    calls += reps;
    t = clock_nsec();
  }
  FREE(buf);
  if (calls == 0)
    return 0;
  return (t - t0)/((double)calls*chunk);
}

static double time_uint8_bound(int chunk, double bench_time, uint8_t bound,
                               randompack_rng *rng) {
  uint8_t *buf;
  TEST_ALLOC(buf, chunk);
  int reps = 1000000/chunk;
  if (reps < 1)
    reps = 1;
  int calls = 0;
  uint64_t t0 = clock_nsec();
  uint64_t deadline = t0 + (uint64_t)(bench_time*1e9);
  uint64_t t = t0;
  while (t < deadline) {
    for (int i = 0; i < reps; i++) {
      ASSERT(randompack_uint8(buf, chunk, bound, rng));
      consume_u64(buf[chunk - 1]);
    }
    calls += reps;
    t = clock_nsec();
  }
  FREE(buf);
  if (calls == 0)
    return 0;
  return (t - t0)/((double)calls*chunk);
}

static double time_uint64_bound(int chunk, double bench_time, uint64_t bound,
                                randompack_rng *rng) {
  uint64_t *buf;
  TEST_ALLOC(buf, chunk);
  int reps = 1000000/chunk;
  if (reps < 1)
    reps = 1;
  int calls = 0;
  uint64_t t0 = clock_nsec();
  uint64_t deadline = t0 + (uint64_t)(bench_time*1e9);
  uint64_t t = t0;
  while (t < deadline) {
    for (int i = 0; i < reps; i++) {
      ASSERT(randompack_uint64(buf, chunk, bound, rng));
      consume_u64(buf[chunk - 1]);
    }
    calls += reps;
    t = clock_nsec();
  }
  FREE(buf);
  if (calls == 0)
    return 0;
  return (t - t0)/((double)calls*chunk);
}

static double time_perm(int n, double bench_time, randompack_rng *rng) {
  int *buf;
  TEST_ALLOC(buf, n);
  int reps = 100000/n;
  if (reps < 1)
    reps = 1;
  int calls = 0;
  uint64_t t0 = clock_nsec();
  uint64_t deadline = t0 + (uint64_t)(bench_time*1e9);
  uint64_t t = t0;
  while (t < deadline) {
    for (int i = 0; i < reps; i++) {
      ASSERT(randompack_perm(buf, n, rng));
      consume_u64(buf[n - 1]);
    }
    calls += reps;
    t = clock_nsec();
  }
  FREE(buf);
  if (calls == 0)
    return 0;
  return (t - t0)/calls;
}

static double time_sample(int n, int k, double bench_time, randompack_rng *rng) {
  int *buf;
  TEST_ALLOC(buf, k);
  int reps = 100000/n;
  if (reps < 1)
    reps = 1;
  int calls = 0;
  uint64_t t0 = clock_nsec();
  uint64_t deadline = t0 + (uint64_t)(bench_time*1e9);
  uint64_t t = t0;
  while (t < deadline) {
    for (int i = 0; i < reps; i++) {
      ASSERT(randompack_sample(buf, n, k, rng));
      consume_u64(buf[k - 1]);
    }
    calls += reps;
    t = clock_nsec();
  }
  FREE(buf);
  if (calls == 0)
    return 0;
  return (t - t0)/calls;
}

static double time_long_long_range(int chunk, double bench_time, long long m,
                                   long long n, randompack_rng *rng) {
  long long *buf;
  TEST_ALLOC(buf, chunk);
  int reps = 1000000/chunk;
  if (reps < 1)
    reps = 1;
  int calls = 0;
  uint64_t t0 = clock_nsec();
  uint64_t deadline = t0 + (uint64_t)(bench_time*1e9);
  uint64_t t = t0;
  while (t < deadline) {
    for (int i = 0; i < reps; i++) {
      ASSERT(randompack_long_long(buf, chunk, m, n, rng));
      consume_u64((uint64_t)buf[chunk - 1]);
    }
    calls += reps;
    t = clock_nsec();
  }
  FREE(buf);
  if (calls == 0)
    return 0;
  return (t - t0)/((double)calls*chunk);
}

int main(int argc, char **argv) {
  char *engine;
  double bench_time;
  int chunk, seed;
  bool have_seed;
  bool help;
  if (!get_options(argc, argv, &engine, &bench_time, &chunk, &seed, &have_seed,
      &help) || help) {
    print_help();
    return help ? 0 : 1;
  }
#if defined(__linux__)
#endif
  randompack_rng *rng = randompack_create(engine);
  if (!rng) {
    fprintf(stderr, "randompack_create failed: %s\n", engine);
    return 1;
  }
  warmup_cpu(0.1);
  struct { int n; char *label; } int_ranges[] = {
    { 3, "1-3" },
    { 20, "1-20" },
    { 1000, "1-1000" },
    { 100000, "1-1e5" },
    { 10000000, "1-1e7" },
    { 1000000000, "1-1e9" },
  };
  struct { long long n; char *label; } ll_ranges[] = {
    { 10, "1-10" },
    { 1000, "1-1e3" },
    { 1000000, "1-1e6" },
    { 10000000000LL, "1-1e10" },
    { 1000000000000000000LL, "1-1e18" },
  };
  struct { uint8_t bound; char *label; } u8_specs[] = {
    { 2, "bound 2" },
    { 10, "bound 10" },
  };
  struct { int n; char *label; } perm_specs[] = {
    { 100, "100" },
    { 100000, "100000" },
  };
  struct { int n; int k; char *label; } sample_specs[] = {
    { 1000, 10, "1000/10" },
    { 1000, 499, "1000/499" },
    { 1000, 501, "1000/501" },
    { 1000, 990, "1000/990" },
  };
  uint64_t u64_bound = UINT64_MAX/3;
  printf("engine:           %s\n", engine);
  printf("time per value:   ns/value\n");
  printf("bench_time:       %.3f s per case\n", bench_time);
  printf("chunk:            %d\n", chunk);
  printf("\n%-14s %8s\n", "int range", "ns/value");
  for (int i = 0; i < LEN(int_ranges); i++) {
    set_seed(rng, seed, have_seed);
    double ns = time_int_range(chunk, bench_time, 1, int_ranges[i].n, rng);
    printf("%-14s %8.2f\n", int_ranges[i].label, ns);
  }
  printf("\n%-14s %8s\n", "long long", "ns/value");
  for (int i = 0; i < LEN(ll_ranges); i++) {
    set_seed(rng, seed, have_seed);
    double ns = time_long_long_range(chunk, bench_time, 1, ll_ranges[i].n, rng);
    printf("%-14s %8.2f\n", ll_ranges[i].label, ns);
  }
  printf("\n%-14s %8s\n", "uint8", "ns/value");
  for (int i = 0; i < LEN(u8_specs); i++) {
    set_seed(rng, seed, have_seed);
    double ns = time_uint8_bound(chunk, bench_time, u8_specs[i].bound, rng);
    printf("%-14s %8.2f\n", u8_specs[i].label, ns);
  }
  printf("\n%-14s %8s\n", "uint64", "ns/value");
  set_seed(rng, seed, have_seed);
  double ns_u64 = time_uint64_bound(chunk, bench_time, u64_bound, rng);
  printf("%-14s %8.2f\n", "UINT64_MAX/3", ns_u64);
  printf("\n%-14s %10s\n", "perm n", "ns/value");
  for (int i = 0; i < LEN(perm_specs); i++) {
    int n = perm_specs[i].n;
    set_seed(rng, seed, have_seed);
    double ns = time_perm(n, bench_time, rng);
    printf("%-14s %10.2f\n", perm_specs[i].label, ns/n);
  }
  printf("\n%-14s %10s\n", "sample", "ns/value");
  for (int i = 0; i < LEN(sample_specs); i++) {
    int n = sample_specs[i].n;
    int k = sample_specs[i].k;
    set_seed(rng, seed, have_seed);
    double ns = time_sample(n, k, bench_time, rng);
    printf("%-14s %10.2f\n", sample_specs[i].label, ns/k);
  }
  randompack_free(rng);
  return 0;
}
