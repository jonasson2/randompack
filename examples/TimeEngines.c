// -*- C -*-
// TimeEngines.c: throughput (GB/s) of randompack engines using uint64 and uint32 bulk fill.

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "TimeUtil.h"
#include "Util.h"
#include "randompack.h"
#include "randompack_config.h"

static void print_help(void) {
  printf("TimeEngines — measure RNG engine throughput\n");
  printf("Usage: TimeEngines [options]\n\n");
  printf("Options:\n");
  printf("  -h            Show this help message\n");
  printf("  -t seconds    Benchmark time per engine (default 0.1)\n");
  printf("  -c chunk      Chunk size (values per fill, default 1024)\n");
  printf("  -s seed       RNG seed (default 7)\n\n");
  printf("Notes:\n");
  printf("  Reports GB/s as bytes/ns: uint64 uses 8/ns, uint32 uses 4/ns.\n");
}

static bool get_options(int argc, char **argv,
                        double *bench_time, int *chunk, int *seed, bool *help) {
  opterr = 0;
  optind = 1;
  int opt;
  *bench_time = 0.1;
  *chunk = 1024;
  *seed = 7;
  *help = false;
  while ((opt = getopt(argc, argv, "ht:c:s:")) != -1) {
    switch (opt) {
      case 'h':
        *help = true;
        return true;
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
        break;
      default:
        return false;
    }
  }
  if (optind < argc)
    return false;
  return true;
}

static void fill_u64(uint64_t out[], int n, randompack_rng *rng) {
  randompack_uint64(out, (size_t)n, 0, rng);
}

static void fill_u32(uint32_t out[], int n, randompack_rng *rng) {
  randompack_uint32(out, (size_t)n, 0, rng);
}

int main(int argc, char **argv) {
  double bench_time;
  int chunk, seed;
  bool help;
  if (!get_options(argc, argv, &bench_time, &chunk, &seed, &help) || help) {
    print_help();
    return help ? 0 : 1;
  }
  warmup_cpu(100);
  printf("throughput:       GB/s (decimal), computed as bytes/ns\n");
  printf("bench_time:       %.3f s per engine\n", bench_time);
  printf("chunk:            %d\n\n", chunk);
  struct { char *name; } engines[] = {
    { "xoshiro256++" },
    { "xoshiro256**" },
    { "xorshift128+" },
    { "pcg64" },
    { "philox" },
    { "system" },
    { "chacha20" },
  };
  printf("%-18s %8s %8s %8s %8s\n", "Engine", "ns64", "GB/s64", "ns32", "GB/s32");
  for (int i = 0; i < LEN(engines); i++) {
    randompack_rng *rng = randompack_create(engines[i].name);
    randompack_seed(seed, 0, 0, rng);
    double ns64 = time_u64(chunk, bench_time, fill_u64, rng);
    double gb64 = 8/ns64;
    double ns32 = time_u32(chunk, bench_time, fill_u32, rng);
    double gb32 = 4/ns32;
    printf("%-18s", engines[i].name);
	 printf(" %8.2f", ns64);
	 printf(" %8.2f", gb64);
	 printf(" %8.2f", ns32);
	 printf(" %8.2f", gb32);
    printf("\n");
	 randompack_free(rng);
  }
  return 0;
}
