// -*- C -*-
// TimeRanlux.c: time ranluxpp portable vs randompack ranlux (ns/value).
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
#include "ranluxpp.h"

static void print_help(void) {
  printf("TimeRanlux — time ranluxpp portable and randompack ranlux\n");
  printf("Usage: TimeRanlux [options]\n\n");
  printf("Options:\n");
  printf("  -h            Show this help message\n");
  printf("  -t seconds    Benchmark time (default 0.1)\n");
  printf("  -c chunk      Values per fill (default 4096)\n");
  printf("  -s seed       RNG seed (default 1)\n\n");
  printf("Notes:\n");
  printf("  Warmup time is 0.1 s.\n");
}

static bool get_options(int argc, char **argv, double *bench_time, int *chunk,
                        int *seed, bool *help) {
  opterr = 0;
  optind = 1;
  int opt;
  *bench_time = 0.1;
  *chunk = 4096;
  *seed = 1;
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

static void fill_ranluxpp(uint64_t out[], int n, ranluxpp_t *r) {
  int i = 0;
  while (i < n) {
    ranluxpp_nextstate(r);
    for (int j = 0; j < 9 && i < n; j++, i++)
      out[i] = r->x[j];
  }
}

static void consume64(const void *p) {
  static volatile uint64_t sink;
  uint64_t u;
  memcpy(&u, p, sizeof(u));
  sink ^= u;
}

static void warmup_ranluxpp(double seconds, int seed) {
  ranluxpp_t r;
  uint64_t seed64 = seed;
  ranluxpp_init(&r, seed64, 2048);
  double t0 = get_time();
  double t = t0;
  uint64_t buf[9];
  while (t - t0 < seconds) {
    for (int i = 0; i < 4096; i++) {
      ranluxpp_nextstate(&r);
      for (int j = 0; j < 9; j++) buf[j] = r.x[j];
    }
    consume64(&buf[0]);
    t = get_time();
  }
}

static double time_ranluxpp(int chunk, double bench_time, int seed) {
  uint64_t *buf;
  TEST_ALLOC(buf, chunk);
  ranluxpp_t r;
  uint64_t seed64 = seed;
  ranluxpp_init(&r, seed64, 2048);
  int reps = max(1, 1000000/chunk);
  int64_t calls = 0;
  double t0 = get_time();
  double t = t0;
  while (t - t0 < bench_time) {
    for (int i = 0; i < reps; i++) {
      fill_ranluxpp(buf, chunk, &r);
      consume64(&buf[chunk - 1]);
    }
    calls += reps;
    t = get_time();
  }
  FREE(buf);
  return (calls > 0) ? 1e9*(t - t0)/((double)calls*chunk) : 0;
}

static void fill_u64(uint64_t out[], int n, randompack_rng *rng) {
  randompack_uint64(out, (size_t)n, 0, rng);
}

int main(int argc, char **argv) {
  double bench_time;
  int chunk, seed;
  bool help;
  if (!get_options(argc, argv, &bench_time, &chunk, &seed, &help) || help) {
    print_help();
    return help ? 0 : 1;
  }
#if defined(__linux__)
  pin_to_cpu0();
#endif
  warmup_cpu(50);
  double warmup_time = 0.1;
  warmup_ranluxpp(warmup_time, seed);
  randompack_rng *rng = randompack_create("ranlux");
  randompack_rng *rng_p = randompack_create("ranluxpp_portable");
  if (!rng) {
    fprintf(stderr, "randompack_create failed: ranlux\n");
    return 1;
  }
  if (!rng_p) {
    fprintf(stderr, "randompack_create failed: ranluxpp_portable\n");
    randompack_free(rng);
    return 1;
  }
  if (!randompack_seed(seed, 0, 0, rng)) {
    if (!randompack_randomize(rng)) {
      fprintf(stderr, "randompack_seed/randomize failed: ranlux\n");
      randompack_free(rng);
      randompack_free(rng_p);
      return 1;
    }
  }
  if (!randompack_seed(seed, 0, 0, rng_p)) {
    if (!randompack_randomize(rng_p)) {
      fprintf(stderr, "randompack_seed/randomize failed: ranluxpp_portable\n");
      randompack_free(rng);
      randompack_free(rng_p);
      return 1;
    }
  }
  printf("bench_time:       %.3f s\n", bench_time);
  printf("chunk:            %d\n", chunk);
  printf("seed:             %d\n\n", seed);
  printf("%-14s %8s %8s\n", "Generator", "ns64", "GB/s");
  double ns_rpp = time_ranluxpp(chunk, bench_time, seed);
  printf("%-14s %8.2f %8.2f\n", "ranluxpp", ns_rpp, 8/ns_rpp);
  double ns_rp = time_u64(chunk, bench_time, fill_u64, rng);
  printf("%-14s %8.2f %8.2f\n", "ranlux", ns_rp, 8/ns_rp);
  double ns_rp_p = time_u64(chunk, bench_time, fill_u64, rng_p);
  printf("%-14s %8.2f %8.2f\n", "ranluxpp_port", ns_rp_p, 8/ns_rp_p);
  randompack_free(rng);
  randompack_free(rng_p);
  return 0;
}
