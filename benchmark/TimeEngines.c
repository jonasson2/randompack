// -*- C -*-
// TimeEngines.c: throughput (GB/s) of randompack engines using uint64 bulk fill.

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "getopt.h"

#include "TimeUtil.h"
#include "Util.h"
#include "randompack.h"
#include "randompack_config.h"

static void print_help(void) {
  printf("TimeEngines — measure RNG engine throughput\n");
  printf("Usage: TimeEngines [options]\n\n");
  printf("Options:\n");
  printf("  -h            Show this help message\n");
  printf("  -t seconds    Benchmark time per engine (default 0.2)\n");
  printf("  -w seconds    CPU warmup time before timing (default 0.1)\n");
  printf("  -c chunk      Chunk size (values per fill, default 4096)\n");
  printf("  -s seed       Fixed RNG seed (default random seed per engine)\n\n");
  printf("  -d digits     Decimal places for ns output (default 2)\n");
  printf("  -S            Benchmark SIMD engines only\n");
  printf("  -n            Benchmark non-SIMD engines only\n\n");
  printf("Notes:\n");
  printf("  Reports GB/s as bytes/ns: uint64 uses 8/ns.\n");
}

static bool get_options(int argc, char **argv,
  double *bench_time, double *warmup_time, int *chunk, int *seed, bool *have_seed,
  int *digits, bool *simd_only, bool *nonsimd_only, bool *help) {
  opterr = 0;
  optind = 1;
  int opt;
  *bench_time = 0.2;
  *warmup_time = 0.1;
  *chunk = 4096;
  *seed = 0;
  *have_seed = false;
  *digits = 2;
  *simd_only = false;
  *nonsimd_only = false;
  *help = false;
  while ((opt = getopt(argc, argv, "hSt:w:c:s:d:n")) != -1) {
    switch (opt) {
      case 'h':
        *help = true;
        return true;
      case 'S':
        *simd_only = true;
        break;
      case 't':
        *bench_time = atof(optarg);
        if (*bench_time <= 0)
          return false;
        break;
      case 'w':
        *warmup_time = atof(optarg);
        if (*warmup_time < 0)
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
      case 'n':
        *nonsimd_only = true;
        break;
      default:
        return false;
    }
  }
  if (*simd_only && *nonsimd_only)
    return false;
  if (optind < argc)
    return false;
  return true;
}

static void fill_u64(uint64_t out[], int n, randompack_rng *rng) {
  randompack_uint64(out, (size_t)n, 0, rng);
}

static bool is_simd_engine(const char *name) {
  return !strcmp(name, "x256++simd") || !strcmp(name, "x256**simd") ||
    !strcmp(name, "sfc64simd");
}

int main(int argc, char **argv) {
  double bench_time;
  double warmup_time;
  int chunk, seed, digits;
  bool have_seed;
  bool simd_only, nonsimd_only, help;
  if (!get_options(argc, argv, &bench_time, &warmup_time, &chunk, &seed,
      &have_seed, &digits, &simd_only, &nonsimd_only, &help) || help) {
    print_help();
    return help ? 0 : 1;
  }
#if defined(__linux__)
#endif
  warmup_cpu(warmup_time);
  printf("throughput:       GB/s (decimal), computed as bytes/ns\n");
  printf("bench_time:       %.3f s per engine\n", bench_time);
  printf("warmup_time:      %.3f s\n", warmup_time);
  printf("chunk:            %d\n\n", chunk);
  fflush(stdout);
  int n = 0;
  int emax = 0;
  int dmax = 0;
  bool ok = randompack_engines(0, 0, &n, &emax, &dmax);
  if (!ok) {
    fprintf(stderr, "randompack_engines query failed\n");
    return 1;
  }
  char *engines = 0;
  char *descriptions = 0;
  if (!ALLOC(engines, n*emax) || !ALLOC(descriptions, n*dmax)) {
    fprintf(stderr, "allocation failed\n");
    FREE(engines);
    FREE(descriptions);
    return 1;
  }
  ok = randompack_engines(engines, descriptions, &n, &emax, &dmax);
  if (!ok) {
    fprintf(stderr, "randompack_engines fill failed\n");
    FREE(engines);
    FREE(descriptions);
    return 1;
  }
  printf("%-18s %10s %8s\n", "Engine", "ns/64bits", "GB/s");
  for (int i = 0; i < n; i++) {
    char *name = engines + i*emax;
    bool is_simd = is_simd_engine(name);
    if (simd_only && !is_simd)
      continue;
    if (nonsimd_only && is_simd)
      continue;
    randompack_rng *rng = randompack_create(name);
    if (!rng) {
      fprintf(stderr, "randompack_create failed: %s\n", name);
      continue;
    }
    if (have_seed)
      ok = randompack_seed(seed, 0, 0, rng);
    else
      ok = randompack_randomize(rng);
    if (!ok) {
      fprintf(stderr, "randompack_seed/randomize failed: %s\n", name);
      randompack_free(rng);
      continue;
    }
    double ns64 = time_u64(chunk, bench_time, fill_u64, rng);
    double gb64 = 8/ns64;
    printf("%-18s", name);
    printf(" %10.*f", digits, ns64);
    printf(" %8.2f", gb64);
    printf("\n");
    randompack_free(rng);
  }
  FREE(engines);
  FREE(descriptions);
  return 0;
}
