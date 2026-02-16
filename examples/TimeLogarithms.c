// -*- C -*-
// TimeLogarithms.c: time log implementations (ns/value).

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "getopt.h"

#include "Util.h"
#include "randompack.h"
#include "randompack_config.h"
#include "openlibm.inc"

static void print_engines(void) {
  int n = 0;
  int elen = 0;
  int dlen = 0;
  if (!randompack_engines(0, 0, &n, &elen, &dlen) || n <= 0 || elen <= 0 ||
      dlen <= 0) {
    printf("  (no engines available)\n");
    return;
  }
  char *names = malloc((size_t)n*(size_t)elen);
  char *descs = malloc((size_t)n*(size_t)dlen);
  if (!names || !descs) {
    free(names);
    free(descs);
    printf("  (allocation failed)\n");
    return;
  }
  if (!randompack_engines(names, descs, &n, &elen, &dlen)) {
    free(names);
    free(descs);
    printf("  (engine listing unavailable)\n");
    return;
  }
  int width = elen - 1;
  for (int i = 0; i < n; i++) {
    printf("  %-*s  %s\n", width, names + i*elen, descs + i*dlen);
  }
  free(names);
  free(descs);
}

static void print_help(void) {
  printf("TimeLogarithms - time log implementations (ns/value)\n");
  printf("Usage: TimeLogarithms [options]\n\n");
  printf("Options:\n");
  printf("  -h            Show this help message\n");
  printf("  -e engine     RNG engine (default x256++simd)\n");
  printf("  -t seconds    Benchmark time per log (default 0.1)\n");
  printf("  -c chunk      Chunk size (values per call, default 4096)\n");
  printf("  -s seed       RNG seed (default 7)\n\n");
  printf("Engines:\n");
  print_engines();
}

static bool get_options(int argc, char **argv, char **engine, double *bench_time,
                        int *chunk, int *seed, bool *help) {
  opterr = 0;
  optind = 1;
  int opt;
  *engine = "x256++simd";
  *bench_time = 0.1;
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

static inline void consume5(const double *buf, int chunk) {
  static volatile uint64_t sink;
  uint64_t u;
  memcpy(&u, &buf[0], sizeof(u)); sink ^= u;
  memcpy(&u, &buf[chunk/4], sizeof(u)); sink ^= u;
  memcpy(&u, &buf[chunk/2], sizeof(u)); sink ^= u;
  memcpy(&u, &buf[3*chunk/4], sizeof(u)); sink ^= u;
  memcpy(&u, &buf[chunk-1], sizeof(u)); sink ^= u;
}

typedef double (*log_fn)(double x);

typedef struct {
  char *name;
  log_fn fn;
} log_spec;

static void apply_log(double out[], const double in[], int n, log_fn fn) {
  for (int i = 0; i < n; i++)
    out[i] = fn(in[i]);
}

static double time_log(int chunk, double bench_time, log_fn fn, double in[],
                       double out[], randompack_rng *rng) {
  int reps = max(1, 1000000/chunk);
  int64_t calls = 0;
  double total = 0;
  size_t len = (size_t)chunk;
  while (total < bench_time) {
    ASSERT(randompack_u01(in, len, rng));
    double t0 = get_time();
    for (int i = 0; i < reps; i++) {
      apply_log(out, in, chunk, fn);
      consume5(out, chunk);
    }
    total += get_time() - t0;
    calls += reps;
  }
  return (calls > 0) ? 1e9*total/((double)calls*chunk) : 0;
}

int main(int argc, char **argv) {
  char *engine;
  double bench_time;
  int chunk, seed;
  bool help;
  if (!get_options(argc, argv, &engine, &bench_time, &chunk, &seed, &help) ||
      help) {
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
  double *in = 0;
  double *out = 0;
  if (!ALLOC(in, chunk) || !ALLOC(out, chunk)) {
    fprintf(stderr, "allocation failed\n");
    FREE(in);
    FREE(out);
    randompack_free(rng);
    return 1;
  }
  log_spec logs[] = {
    { "log", log },
    { "openlibm_log", openlibm_log },
  };
  printf("engine:           %s\n", engine);
  printf("time per value:   ns/value\n");
  printf("bench_time:       %.3f s per log\n", bench_time);
  printf("chunk:            %d\n\n", chunk);
  printf("%-14s %10s\n", "Log", "double");
  for (int i = 0; i < LEN(logs); i++) {
    double ns = time_log(chunk, bench_time, logs[i].fn, in, out, rng);
    printf("%-14s %10.2f\n", logs[i].name, ns);
  }
  FREE(in);
  FREE(out);
  randompack_free(rng);
  return 0;
}
