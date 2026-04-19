// TimeDistMKL.c
// Compare Intel oneMKL VSL vs randompack distributions (ns/value).
//
// Example build on Linux with clang (after sourcing oneAPI setvars.sh):
//   clang -O3 -std=c11 -I../src -I${MKLROOT}/include TimeDistMKL.c \
//     -L../release/src -lrandompack -Wl,-rpath,../release/src \
//     -Wl,--start-group \
//     ${MKLROOT}/lib/intel64/libmkl_intel_lp64.a \
//     ${MKLROOT}/lib/intel64/libmkl_sequential.a \
//     ${MKLROOT}/lib/intel64/libmkl_core.a \
//     -Wl,--end-group -lpthread -lm -ldl -o TimeDistMKL
//
// If you prefer dynamic MKL linkage, replace the three static libraries with
//   -L${MKLROOT}/lib/intel64 -lmkl_rt

#include <math.h>
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

static void consume(double x) {
  static volatile double sink = 0;
  sink += x;
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

static void fill_mkl_uniform(VSLStreamStatePtr stream, int n, double *x,
  double a, double b) {
  int status = vdRngUniform(VSL_RNG_METHOD_UNIFORM_STD, stream, n, x, a, b);
  if (status != VSL_STATUS_OK) {
    die_mkl("vdRngUniform", status);
  }
}

static void fill_mkl_normal(VSLStreamStatePtr stream, int n, double *x,
  double mu, double sigma) {
  int status = vdRngGaussian(VSL_RNG_METHOD_GAUSSIAN_BOXMULLER2, stream, n, x,
    mu, sigma);
  if (status != VSL_STATUS_OK) {
    die_mkl("vdRngGaussian", status);
  }
}

static void fill_mkl_exp(VSLStreamStatePtr stream, int n, double *x,
  double scale) {
  int status = vdRngExponential(VSL_RNG_METHOD_EXPONENTIAL_ICDF, stream, n, x,
    0, scale);
  if (status != VSL_STATUS_OK) {
    die_mkl("vdRngExponential", status);
  }
}

static void fill_mkl_lognormal(VSLStreamStatePtr stream, int n, double *x,
  double mu, double sigma) {
  int status = vdRngLognormal(VSL_RNG_METHOD_LOGNORMAL_BOXMULLER2, stream, n,
    x, mu, sigma, 0, 1);
  if (status != VSL_STATUS_OK) {
    die_mkl("vdRngLognormal", status);
  }
}

static void fill_mkl_gamma(VSLStreamStatePtr stream, int n, double *x,
  double shape, double scale) {
  int status = vdRngGamma(VSL_RNG_METHOD_GAMMA_GNORM, stream, n, x, shape, 0,
    scale);
  if (status != VSL_STATUS_OK) {
    die_mkl("vdRngGamma", status);
  }
}

static void fill_mkl_beta(VSLStreamStatePtr stream, int n, double *x,
  double a, double b) {
  int status = vdRngBeta(VSL_RNG_METHOD_BETA_CJA, stream, n, x, a, b, 0, 1);
  if (status != VSL_STATUS_OK) {
    die_mkl("vdRngBeta", status);
  }
}

static void fill_mkl_chi2(VSLStreamStatePtr stream, int n, double *x, int nu) {
  int status = vdRngChiSquare(VSL_RNG_METHOD_CHISQUARE_CHI2GAMMA, stream, n,
    x, nu);
  if (status != VSL_STATUS_OK) {
    die_mkl("vdRngChiSquare", status);
  }
}

static void fill_mkl_weibull(VSLStreamStatePtr stream, int n, double *x,
  double shape, double scale) {
  int status = vdRngWeibull(VSL_RNG_METHOD_WEIBULL_ICDF, stream, n, x, shape,
    0, scale);
  if (status != VSL_STATUS_OK) {
    die_mkl("vdRngWeibull", status);
  }
}

typedef void (*fill_fn)(void *ctx);

typedef struct {
  VSLStreamStatePtr stream;
  double *x;
  int chunk;
} mkl_ctx;

typedef struct {
  randompack_rng *rng;
  double *x;
  int chunk;
} rp_ctx;

static double time_dist(int chunk, double bench_time, int reps, fill_fn fill,
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
  printf("TimeDistMKL — compare Intel oneMKL VSL vs randompack (ns/value)\n");
  printf("Usage: TimeDistMKL [options]\n\n");
  printf("Options:\n");
  printf("  -h            Show this help message\n");
  printf("  -e engine     RNG engine (default x256++simd)\n");
  printf("  -t seconds    Benchmark time per distribution (default 0.2)\n");
  printf("  -c chunk      Chunk size (default 4096)\n");
  printf("  -s seed       Fixed RNG seed (default random seed per case)\n");
  printf("  -b            Use bitexact log/exp implementations\n");
}

static void fill_mkl_u01(void *vctx) {
  mkl_ctx *ctx = vctx;
  fill_mkl_uniform(ctx->stream, ctx->chunk, ctx->x, 0, 1);
  consume(ctx->x[ctx->chunk - 1]);
}

static void fill_rp_u01(void *vctx) {
  rp_ctx *ctx = vctx;
  randompack_u01(ctx->x, ctx->chunk, ctx->rng);
  consume(ctx->x[ctx->chunk - 1]);
}

static void fill_mkl_unif_2_5(void *vctx) {
  mkl_ctx *ctx = vctx;
  fill_mkl_uniform(ctx->stream, ctx->chunk, ctx->x, 2, 5);
  consume(ctx->x[ctx->chunk - 1]);
}

static void fill_rp_unif_2_5(void *vctx) {
  rp_ctx *ctx = vctx;
  randompack_unif(ctx->x, ctx->chunk, 2, 5, ctx->rng);
  consume(ctx->x[ctx->chunk - 1]);
}

static void fill_mkl_norm(void *vctx) {
  mkl_ctx *ctx = vctx;
  fill_mkl_normal(ctx->stream, ctx->chunk, ctx->x, 0, 1);
  consume(ctx->x[ctx->chunk - 1]);
}

static void fill_rp_norm(void *vctx) {
  rp_ctx *ctx = vctx;
  randompack_norm(ctx->x, ctx->chunk, ctx->rng);
  consume(ctx->x[ctx->chunk - 1]);
}

static void fill_mkl_normal_2_3(void *vctx) {
  mkl_ctx *ctx = vctx;
  fill_mkl_normal(ctx->stream, ctx->chunk, ctx->x, 2, 3);
  consume(ctx->x[ctx->chunk - 1]);
}

static void fill_rp_normal_2_3(void *vctx) {
  rp_ctx *ctx = vctx;
  randompack_normal(ctx->x, ctx->chunk, 2, 3, ctx->rng);
  consume(ctx->x[ctx->chunk - 1]);
}

static void fill_mkl_exp_1(void *vctx) {
  mkl_ctx *ctx = vctx;
  fill_mkl_exp(ctx->stream, ctx->chunk, ctx->x, 1);
  consume(ctx->x[ctx->chunk - 1]);
}

static void fill_rp_exp_1(void *vctx) {
  rp_ctx *ctx = vctx;
  randompack_exp(ctx->x, ctx->chunk, 1, ctx->rng);
  consume(ctx->x[ctx->chunk - 1]);
}

static void fill_mkl_exp_2(void *vctx) {
  mkl_ctx *ctx = vctx;
  fill_mkl_exp(ctx->stream, ctx->chunk, ctx->x, 2);
  consume(ctx->x[ctx->chunk - 1]);
}

static void fill_rp_exp_2(void *vctx) {
  rp_ctx *ctx = vctx;
  randompack_exp(ctx->x, ctx->chunk, 2, ctx->rng);
  consume(ctx->x[ctx->chunk - 1]);
}

static void fill_mkl_lognormal_0_1(void *vctx) {
  mkl_ctx *ctx = vctx;
  fill_mkl_lognormal(ctx->stream, ctx->chunk, ctx->x, 0, 1);
  consume(ctx->x[ctx->chunk - 1]);
}

static void fill_rp_lognormal_0_1(void *vctx) {
  rp_ctx *ctx = vctx;
  randompack_lognormal(ctx->x, ctx->chunk, 0, 1, ctx->rng);
  consume(ctx->x[ctx->chunk - 1]);
}

static void fill_mkl_gamma_2_3(void *vctx) {
  mkl_ctx *ctx = vctx;
  fill_mkl_gamma(ctx->stream, ctx->chunk, ctx->x, 2, 3);
  consume(ctx->x[ctx->chunk - 1]);
}

static void fill_rp_gamma_2_3(void *vctx) {
  rp_ctx *ctx = vctx;
  randompack_gamma(ctx->x, ctx->chunk, 2, 3, ctx->rng);
  consume(ctx->x[ctx->chunk - 1]);
}

static void fill_mkl_gamma_0_5_2(void *vctx) {
  mkl_ctx *ctx = vctx;
  fill_mkl_gamma(ctx->stream, ctx->chunk, ctx->x, 0.5, 2);
  consume(ctx->x[ctx->chunk - 1]);
}

static void fill_rp_gamma_0_5_2(void *vctx) {
  rp_ctx *ctx = vctx;
  randompack_gamma(ctx->x, ctx->chunk, 0.5, 2, ctx->rng);
  consume(ctx->x[ctx->chunk - 1]);
}

static void fill_mkl_beta_2_5(void *vctx) {
  mkl_ctx *ctx = vctx;
  fill_mkl_beta(ctx->stream, ctx->chunk, ctx->x, 2, 5);
  consume(ctx->x[ctx->chunk - 1]);
}

static void fill_rp_beta_2_5(void *vctx) {
  rp_ctx *ctx = vctx;
  randompack_beta(ctx->x, ctx->chunk, 2, 5, ctx->rng);
  consume(ctx->x[ctx->chunk - 1]);
}

static void fill_mkl_chi2_5(void *vctx) {
  mkl_ctx *ctx = vctx;
  fill_mkl_chi2(ctx->stream, ctx->chunk, ctx->x, 5);
  consume(ctx->x[ctx->chunk - 1]);
}

static void fill_rp_chi2_5(void *vctx) {
  rp_ctx *ctx = vctx;
  randompack_chi2(ctx->x, ctx->chunk, 5, ctx->rng);
  consume(ctx->x[ctx->chunk - 1]);
}

static void fill_mkl_weibull_2_3(void *vctx) {
  mkl_ctx *ctx = vctx;
  fill_mkl_weibull(ctx->stream, ctx->chunk, ctx->x, 2, 3);
  consume(ctx->x[ctx->chunk - 1]);
}

static void fill_rp_weibull_2_3(void *vctx) {
  rp_ctx *ctx = vctx;
  randompack_weibull(ctx->x, ctx->chunk, 2, 3, ctx->rng);
  consume(ctx->x[ctx->chunk - 1]);
}

static void fill_mkl_weibull_3_4(void *vctx) {
  mkl_ctx *ctx = vctx;
  fill_mkl_weibull(ctx->stream, ctx->chunk, ctx->x, 3, 4);
  consume(ctx->x[ctx->chunk - 1]);
}

static void fill_rp_weibull_3_4(void *vctx) {
  rp_ctx *ctx = vctx;
  randompack_weibull(ctx->x, ctx->chunk, 3, 4, ctx->rng);
  consume(ctx->x[ctx->chunk - 1]);
}

static void run_case(const char *name, int case_seed, int chunk,
  double bench_time, int reps, MKL_INT brng, mkl_ctx *mctx,
  rp_ctx *rctx, fill_fn fill_mkl, fill_fn fill_rp) {
  if (vslDeleteStream(&mctx->stream) != VSL_STATUS_OK) {
    die_mkl("vslDeleteStream", -1);
  }
  if (vslNewStream(&mctx->stream, brng, case_seed) != VSL_STATUS_OK) {
    die_mkl("vslNewStream", -1);
  }
  if (!randompack_seed(case_seed, 0, 0, rctx->rng)) {
    fprintf(stderr, "randompack_seed failed\n");
    exit(1);
  }
  double mkl_ns = time_dist(chunk, bench_time, reps, fill_mkl, mctx);
  if (vslDeleteStream(&mctx->stream) != VSL_STATUS_OK) {
    die_mkl("vslDeleteStream", -1);
  }
  if (vslNewStream(&mctx->stream, brng, case_seed) != VSL_STATUS_OK) {
    die_mkl("vslNewStream", -1);
  }
  if (!randompack_seed(case_seed, 0, 0, rctx->rng)) {
    fprintf(stderr, "randompack_seed failed\n");
    exit(1);
  }
  double rp_ns = time_dist(chunk, bench_time, reps, fill_rp, rctx);
  double factor = mkl_ns/rp_ns;
  printf("%-18s %10.2f %11.2f %8.2f\n", name, mkl_ns, rp_ns, factor);
}

int main(int argc, char **argv) {
  char *engine = "x256++simd";
  int chunk = 4096;
  double bench_time = 0.2;
  int seed = 0;
  bool have_seed = false;
  bool bitexact = false;
  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-h")) {
      print_help();
      return 0;
    }
    if (!strcmp(argv[i], "-e") && i + 1 < argc) {
      engine = argv[++i];
    }
    else if (!strcmp(argv[i], "-t") && i + 1 < argc) {
      bench_time = atof(argv[++i]);
    }
    else if (!strcmp(argv[i], "-c") && i + 1 < argc) {
      chunk = atoi(argv[++i]);
    }
    else if (!strcmp(argv[i], "-s") && i + 1 < argc) {
      seed = atoi(argv[++i]);
      have_seed = true;
    }
    else if (!strcmp(argv[i], "-b")) {
      bitexact = true;
    }
    else {
      fprintf(stderr, "Unknown/invalid option: %s\n", argv[i]);
      return 1;
    }
  }
  if (chunk <= 0 || bench_time <= 0) {
    fprintf(stderr, "Invalid -c or -t\n");
    return 1;
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
  int reps = compute_reps(chunk);
  double warm = warmup_cpu(0.1);
  double *x = malloc(chunk*sizeof(*x));
  if (!x) {
    fprintf(stderr, "allocation failed\n");
    free(x);
    randompack_free(rng);
    return 1;
  }
  VSLStreamStatePtr stream = 0;
  MKL_INT brng = VSL_BRNG_MT19937;
  mkl_ctx mctx = {stream, x, chunk};
  rp_ctx rctx = {rng, x, chunk};
  unsigned int s0 = (unsigned int)time(0);
  if (vslNewStream(&mctx.stream, brng, 1) != VSL_STATUS_OK) {
    die_mkl("vslNewStream", -1);
  }
  printf("oneMKL RNG library:  oneMKL VSL\n");
  printf("oneMKL engine:       MT19937\n");
  printf("Randompack engine:   %s\n", engine);
  printf("Chunk size:          %d\n", chunk);
  printf("Warmup:              %.3f s\n", warm);
  printf("\n");
  printf("%-18s %10s %11s %8s\n", "Distribution", "oneMKL",
    "Randompack", "Factor");
  int case_seed;
  case_seed = have_seed ? seed : (int)(rand_r(&s0) & 0x7fffffff);
  run_case("u01", case_seed, chunk, bench_time, reps, brng, &mctx, &rctx,
    fill_mkl_u01, fill_rp_u01);
  case_seed = have_seed ? seed : (int)(rand_r(&s0) & 0x7fffffff);
  run_case("unif(2,5)", case_seed, chunk, bench_time, reps, brng, &mctx,
    &rctx, fill_mkl_unif_2_5, fill_rp_unif_2_5);
  case_seed = have_seed ? seed : (int)(rand_r(&s0) & 0x7fffffff);
  run_case("norm", case_seed, chunk, bench_time, reps, brng, &mctx, &rctx,
    fill_mkl_norm, fill_rp_norm);
  case_seed = have_seed ? seed : (int)(rand_r(&s0) & 0x7fffffff);
  run_case("normal(2,3)", case_seed, chunk, bench_time, reps, brng, &mctx,
    &rctx, fill_mkl_normal_2_3, fill_rp_normal_2_3);
  case_seed = have_seed ? seed : (int)(rand_r(&s0) & 0x7fffffff);
  run_case("exp(1)", case_seed, chunk, bench_time, reps, brng, &mctx, &rctx,
    fill_mkl_exp_1, fill_rp_exp_1);
  case_seed = have_seed ? seed : (int)(rand_r(&s0) & 0x7fffffff);
  run_case("exp(2)", case_seed, chunk, bench_time, reps, brng, &mctx, &rctx,
    fill_mkl_exp_2, fill_rp_exp_2);
  case_seed = have_seed ? seed : (int)(rand_r(&s0) & 0x7fffffff);
  run_case("lognormal(0,1)", case_seed, chunk, bench_time, reps, brng, &mctx,
    &rctx, fill_mkl_lognormal_0_1, fill_rp_lognormal_0_1);
  run_case("gamma(2,3)", case_seed, chunk, bench_time, reps, brng, &mctx,
    &rctx, fill_mkl_gamma_2_3, fill_rp_gamma_2_3);
  case_seed = have_seed ? seed : (int)(rand_r(&s0) & 0x7fffffff);
  run_case("gamma(0.5,2)", case_seed, chunk, bench_time, reps, brng, &mctx,
    &rctx, fill_mkl_gamma_0_5_2, fill_rp_gamma_0_5_2);
  case_seed = have_seed ? seed : (int)(rand_r(&s0) & 0x7fffffff);
  run_case("beta(2,5)", case_seed, chunk, bench_time, reps, brng, &mctx,
    &rctx, fill_mkl_beta_2_5, fill_rp_beta_2_5);
  case_seed = have_seed ? seed : (int)(rand_r(&s0) & 0x7fffffff);
  run_case("chi2(5)", case_seed, chunk, bench_time, reps, brng, &mctx, &rctx,
    fill_mkl_chi2_5, fill_rp_chi2_5);
  case_seed = have_seed ? seed : (int)(rand_r(&s0) & 0x7fffffff);
  run_case("weibull(2,3)", case_seed, chunk, bench_time, reps, brng, &mctx,
    &rctx, fill_mkl_weibull_2_3, fill_rp_weibull_2_3);
  case_seed = have_seed ? seed : (int)(rand_r(&s0) & 0x7fffffff);
  run_case("weibull(3,4)", case_seed, chunk, bench_time, reps, brng, &mctx,
    &rctx, fill_mkl_weibull_3_4, fill_rp_weibull_3_4);
  vslDeleteStream(&mctx.stream);
  free(x);
  randompack_free(rng);
  return 0;
}
