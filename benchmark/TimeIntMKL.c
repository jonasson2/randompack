// TimeIntMKL.c
// Compare Intel oneMKL VSL vs randompack for bounded integer draws (ns/value).

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <mkl.h>

#include "randompack.h"

static double now_sec(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec + 1e-9*ts.tv_nsec;
}

static void consume(int x) {
  static volatile int sink = 0;
  sink ^= x;
}

static int compute_reps(int chunk) {
  int reps = 1000000/chunk;
  return reps > 0 ? reps : 1;
}

static double warmup_cpu(double seconds) {
  double t0 = now_sec();
  double t = t0;
  volatile double x = 0;
  while (t - t0 < seconds) {
    for (int i = 0; i < 1024; i++) {
      x += i;
    }
    t = now_sec();
  }
  if (x < 0) {
    printf("warmup\n");
  }
  return t - t0;
}

static void die_mkl(const char *what, int status) {
  fprintf(stderr, "%s failed, status=%d\n", what, status);
  exit(1);
}

static void fill_mkl_int(VSLStreamStatePtr stream, int n, int *x, int a, int b) {
  int status = viRngUniform(VSL_RNG_METHOD_UNIFORM_STD, stream, n, x, a, b + 1);
  if (status != VSL_STATUS_OK) {
    die_mkl("viRngUniform", status);
  }
}

typedef struct {
  VSLStreamStatePtr stream;
  int *x;
  int chunk;
  int a;
  int b;
} mkl_ctx;

typedef struct {
  randompack_rng *rng;
  int *x;
  int chunk;
  int a;
  int b;
} rp_ctx;

static double time_dist(int chunk, double bench_time, int reps, void (*fill)(void *),
                        void *ctx) {
  int calls = 0;
  double t0 = now_sec();
  double t = t0;
  while (t - t0 < bench_time) {
    for (int i = 0; i < reps; i++) {
      fill(ctx);
      calls++;
    }
    t = now_sec();
  }
  int total = calls*chunk;
  if (total <= 0) {
    return 0;
  }
  return 1e9*(t - t0)/total;
}

static void print_help(void) {
  printf("TimeIntMKL — compare Intel oneMKL VSL vs randompack (ns/value)\n");
  printf("Usage: TimeIntMKL [options]\n\n");
  printf("Options:\n");
  printf("  -h            Show this help message\n");
  printf("  -e engine     RNG engine (default x256++simd)\n");
  printf("  -t seconds    Benchmark time per case (default 0.2)\n");
  printf("  -c chunk      Chunk size (default 4096)\n");
  printf("  -s seed       Fixed RNG seed (default random seed per case)\n");
}

static void fill_mkl_range(void *vctx) {
  mkl_ctx *ctx = vctx;
  fill_mkl_int(ctx->stream, ctx->chunk, ctx->x, ctx->a, ctx->b);
  consume(ctx->x[ctx->chunk - 1]);
}

static void fill_rp_range(void *vctx) {
  rp_ctx *ctx = vctx;
  randompack_int(ctx->x, ctx->chunk, ctx->a, ctx->b, ctx->rng);
  consume(ctx->x[ctx->chunk - 1]);
}

static void run_case(char *name, uint64_t case_seed, int chunk, double bench_time,
                     int reps, int brng, mkl_ctx *mctx, rp_ctx *rctx) {
  int status = vslDeleteStream(&mctx->stream);
  if (status != VSL_STATUS_OK) {
    die_mkl("vslDeleteStream", status);
  }
  status = vslNewStream(&mctx->stream, brng, case_seed);
  if (status != VSL_STATUS_OK) {
    die_mkl("vslNewStream", status);
  }
  if (!randompack_seed(case_seed, 0, 0, rctx->rng)) {
    fprintf(stderr, "randompack_seed failed\n");
    exit(1);
  }
  double mkl_ns = time_dist(chunk, bench_time, reps, fill_mkl_range, mctx);
  status = vslDeleteStream(&mctx->stream);
  if (status != VSL_STATUS_OK) {
    die_mkl("vslDeleteStream", status);
  }
  status = vslNewStream(&mctx->stream, brng, case_seed);
  if (status != VSL_STATUS_OK) {
    die_mkl("vslNewStream", status);
  }
  if (!randompack_seed(case_seed, 0, 0, rctx->rng)) {
    fprintf(stderr, "randompack_seed failed\n");
    exit(1);
  }
  double rp_ns = time_dist(chunk, bench_time, reps, fill_rp_range, rctx);
  double factor = mkl_ns/rp_ns;
  printf("%-18s %10.2f %11.2f %8.2f\n", name, mkl_ns, rp_ns, factor);
}

int main(int argc, char **argv) {
  char *engine = "x256++simd";
  double bench_time = 0.2;
  int chunk = 4096;
  uint64_t seed = 0;
  bool have_seed = false;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      print_help();
      return 0;
    }
    else if (strcmp(argv[i], "-e") == 0) {
      if (i + 1 >= argc) {
        fprintf(stderr, "missing value for -e\n");
        return 1;
      }
      engine = argv[++i];
    }
    else if (strcmp(argv[i], "-t") == 0) {
      if (i + 1 >= argc) {
        fprintf(stderr, "missing value for -t\n");
        return 1;
      }
      bench_time = atof(argv[++i]);
      if (bench_time <= 0) {
        fprintf(stderr, "time per case must be positive\n");
        return 1;
      }
    }
    else if (strcmp(argv[i], "-c") == 0) {
      if (i + 1 >= argc) {
        fprintf(stderr, "missing value for -c\n");
        return 1;
      }
      chunk = atoi(argv[++i]);
      if (chunk <= 0) {
        fprintf(stderr, "chunk must be positive\n");
        return 1;
      }
    }
    else if (strcmp(argv[i], "-s") == 0) {
      if (i + 1 >= argc) {
        fprintf(stderr, "missing value for -s\n");
        return 1;
      }
      seed = strtoull(argv[++i], 0, 10);
      have_seed = true;
    }
    else {
      fprintf(stderr, "Unknown option: %s\n", argv[i]);
      return 1;
    }
  }

  warmup_cpu(0.1);

  int *mx = malloc(sizeof(*mx)*chunk);
  int *rx = malloc(sizeof(*rx)*chunk);
  if (mx == 0 || rx == 0) {
    fprintf(stderr, "allocation failed\n");
    free(mx);
    free(rx);
    return 1;
  }

  randompack_rng *rng = randompack_create(engine);
  if (rng == 0) {
    fprintf(stderr, "randompack_create failed: %s\n", engine);
    free(mx);
    free(rx);
    return 1;
  }

  VSLStreamStatePtr stream = 0;
  uint64_t init_seed = have_seed ? seed : (uint64_t)time(0);
  int brng = VSL_BRNG_MT19937;
  int status = vslNewStream(&stream, brng, init_seed);
  if (status != VSL_STATUS_OK) {
    die_mkl("vslNewStream", status);
  }

  mkl_ctx mctx = { stream, mx, chunk, 1, 10 };
  rp_ctx rctx = { rng, rx, chunk, 1, 10 };
  int reps = compute_reps(chunk);

  printf("time per value:   ns/value\n");
  printf("bench_time:       %.3f s per case\n", bench_time);
  printf("chunk:            %d\n", chunk);
  printf("engine:           %s\n", engine);
  printf("\n");
  printf("%-18s %10s %11s %8s\n", "Benchmark", "MKL", "Randompack", "Factor");

  uint64_t case_seed = have_seed ? seed : (uint64_t)rand();
  mctx.a = 1;
  mctx.b = 10;
  rctx.a = 1;
  rctx.b = 10;
  run_case("int 1-10", case_seed, chunk, bench_time, reps, brng, &mctx, &rctx);

  case_seed = have_seed ? seed : (uint64_t)rand();
  mctx.a = 1;
  mctx.b = 100000;
  rctx.a = 1;
  rctx.b = 100000;
  run_case("int 1-1e5", case_seed, chunk, bench_time, reps, brng, &mctx, &rctx);

  case_seed = have_seed ? seed : (uint64_t)rand();
  mctx.a = 1;
  mctx.b = 2000000000;
  rctx.a = 1;
  rctx.b = 2000000000;
  run_case("int 1-2e9", case_seed, chunk, bench_time, reps, brng, &mctx, &rctx);

  status = vslDeleteStream(&stream);
  if (status != VSL_STATUS_OK) {
    die_mkl("vslDeleteStream", status);
  }
  randompack_free(rng);
  free(mx);
  free(rx);
  return 0;
}
