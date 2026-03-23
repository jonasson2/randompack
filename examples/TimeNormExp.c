// -*- C -*-
// TimeNormExp.c: time normal and exponential draws (ns/value), double only.

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
  printf("TimeNormExp — time normal and exponential draws (ns/value), double\n");
  printf("Usage: TimeNormExp [options]\n\n");
  printf("Options:\n");
  printf("  -h            Show this help message\n");
  printf("  -e engine     RNG engine (default x256++simd)\n");
  printf("  -t seconds    Benchmark time per run (default 0.5)\n");
  printf("  -c chunk      Chunk size (values per call, default 4096)\n");
  printf("  -s seed       RNG seed (default 7)\n");
}

static bool get_options(int argc, char **argv, char **engine, double *bench_time,
                        int *chunk, int *seed, bool *help) {
  opterr = 0;
  optind = 1;
  int opt;
  *engine = "x256++simd";
  *bench_time = 0.5;
  *chunk = 4096;
  *seed = 7;
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
        break;
      default:
        return false;
    }
  }
  if (optind < argc)
    return false;
  return true;
}

static void fill_norm(double out[], int n, double param[], randompack_rng *rng) {
  (void)param;
  ASSERT(randompack_norm(out, (size_t)n, rng));
}

static void fill_exp(double out[], int n, double param[], randompack_rng *rng) {
  (void)param;
  ASSERT(randompack_exp(out, (size_t)n, 1, rng));
}

int main(int argc, char **argv) {
  char *engine;
  double bench_time;
  int chunk, seed;
  bool help;
  if (!get_options(argc, argv, &engine, &bench_time, &chunk, &seed, &help) || help) {
    print_help();
    return help ? 0 : 1;
  }
#if defined(__linux__)
  pin_to_cpu0();
#endif
  randompack_rng *rng = randompack_create(engine);
  if (!rng) {
    fprintf(stderr, "randompack_create failed: %s\n", engine);
    return 1;
  }
  if (!randompack_seed(seed, 0, 0, rng)) {
    fprintf(stderr, "randompack_seed failed\n");
    randompack_free(rng);
    return 1;
  }
  warmup_cpu(100);
  printf("engine: %s\n", engine);
  printf("chunk:  %d\n\n", chunk);
  randompack_norm_counters_reset();
  double ns_norm = time_double(chunk, bench_time, fill_norm, 0, rng);
  uint64_t draws = 0;
  uint64_t pdf_calls = 0;
  uint64_t tail = 0;
  uint64_t accept = 0;
  uint64_t retry = 0;
  uint64_t pdf_accept = 0;
  uint64_t pdf_reject = 0;
  randompack_norm_counters_get(&draws, &pdf_calls);
  randompack_norm_counters_get_slow(&tail, &accept, &retry, &pdf_accept,
    &pdf_reject);
  double pct = draws ? (100* (double)pdf_calls / (double)draws) : 0;
  double pct_tail = draws ? (100* (double)tail / (double)draws) : 0;
  double pct_accept = draws ? (100* (double)accept / (double)draws) : 0;
  double pct_retry = draws ? (100* (double)retry / (double)draws) : 0;
  double pct_pdf_accept = draws ? (100* (double)pdf_accept / (double)draws) : 0;
  double pct_pdf_reject = draws ? (100* (double)pdf_reject / (double)draws) : 0;
  double ns_exp = time_double(chunk, bench_time, fill_exp, 0, rng);
  printf("norm %8.2f ns/value\n", ns_norm);
  printf("norm %8llu draws, %8llu below_normpdf\n",
    (unsigned long long)draws, (unsigned long long)pdf_calls);
  printf("norm %8.2f%% below_normpdf\n", pct);
  printf("slow %8llu tail, %8llu accept, %8llu retry\n",
    (unsigned long long)tail, (unsigned long long)accept,
    (unsigned long long)retry);
  printf("slow %8llu pdf_accept, %8llu pdf_reject\n",
    (unsigned long long)pdf_accept, (unsigned long long)pdf_reject);
  printf("slow %8.2f%% tail, %8.2f%% accept, %8.2f%% retry\n",
    pct_tail, pct_accept, pct_retry);
  printf("slow %8.2f%% pdf_accept, %8.2f%% pdf_reject\n",
    pct_pdf_accept, pct_pdf_reject);
  printf("exp  %8.2f ns/value\n", ns_exp);
  randompack_free(rng);
  return 0;
}
