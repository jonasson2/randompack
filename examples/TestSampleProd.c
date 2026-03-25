// TestSampleProd.c: run TestU01 svaria_SampleProd on PIT-transformed normal samples.

#include <math.h>
#include <stdbool.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "randompack.h"
#include "TestU01.h"

enum { BUFSIZE = 4096 };

static randompack_rng *rng;
static double buf[BUFSIZE];
static float fbuf[BUFSIZE];
static int idx = BUFSIZE;
static bool use_float = false;

static void fill_buffer(void) {
  int n = BUFSIZE;
  bool ok;
  if (use_float) ok = randompack_normf(fbuf, (size_t)n, rng);
  else ok = randompack_norm(buf, (size_t)n, rng);
  if (!ok) {
    fprintf(stderr, use_float ? "randompack_normf failed\n" :
      "randompack_norm failed\n");
    exit(1);
  }
  for (int i = 0; i < n; i++) {
    double x = use_float ? (double)fbuf[i] : buf[i];
    double u = 0.5*erfc(-x/sqrt(2));
    if (u <= 0) u = nextafter(0, 1);
    else if (u >= 1) u = nextafter(1, 0);
    buf[i] = u;
  }
  idx = 0;
}

static double GetU01(void) {
  if (idx >= BUFSIZE) fill_buffer();
  return buf[idx++];
}

static void usage(const char *prog) {
  fprintf(stderr,
    "Usage: %s [-h] [-g engine] [-s seed] [-f] [-N N] [-n n] [-r r] [-t t]\n\n"
    "Run TestU01 svaria_SampleProd on PIT-transformed N(0,1) samples.\n\n"
    "Defaults match the Crush case:\n"
    "  engine = x256++simd, seed = 42, N = 1, n = 10000000, r = 0, t = 30\n\n"
    "Options:\n"
    "  -h          Show this help\n"
    "  -g engine   RNG engine to use (default x256++simd)\n"
    "  -s seed     Integer seed passed to randompack_seed (default 42)\n"
    "  -f          Use float normals (randompack_normf)\n"
    "  -N N        Number of replications (default 1)\n"
    "  -n n        Sample size per replication (default 10000000)\n"
    "  -r r        Number of leading bits skipped by TestU01 (default 0)\n"
    "  -t t        Product size (default 30)\n",
    prog
  );
}

static bool parse_long(const char *text, long *out) {
  char *end = 0;
  long value = strtol(text, &end, 10);
  if (!text[0] || (end && end[0])) return false;
  *out = value;
  return true;
}

int main(int argc, char **argv) {
  int opt;
  const char *engine = "x256++simd";
  long seed = 42;
  long N = 1;
  long n = 10000000;
  long r = 0;
  long t = 30;

  while ((opt = getopt(argc, argv, "hg:s:fN:n:r:t:")) != -1) {
    switch (opt) {
      case 'g':
        engine = optarg;
        break;
      case 's': {
        long value;
        if (!parse_long(optarg, &value) || value < INT_MIN || value > INT_MAX) {
          fprintf(stderr, "bad seed '%s'\n", optarg);
          return 1;
        }
        seed = value;
        break;
      }
      case 'f':
        use_float = true;
        break;
      case 'N':
        if (!parse_long(optarg, &N) || N <= 0) {
          fprintf(stderr, "bad N '%s'\n", optarg);
          return 1;
        }
        break;
      case 'n':
        if (!parse_long(optarg, &n) || n <= 0) {
          fprintf(stderr, "bad n '%s'\n", optarg);
          return 1;
        }
        break;
      case 'r':
        if (!parse_long(optarg, &r) || r < 0 || r > 31) {
          fprintf(stderr, "bad r '%s'\n", optarg);
          return 1;
        }
        break;
      case 't':
        if (!parse_long(optarg, &t) || t <= 0 || t > INT_MAX) {
          fprintf(stderr, "bad t '%s'\n", optarg);
          return 1;
        }
        break;
      case 'h':
        usage(argv[0]);
        return 0;
      default:
        usage(argv[0]);
        return 1;
    }
  }
  if (optind < argc) {
    usage(argv[0]);
    return 1;
  }

  rng = randompack_create(engine);
  if (!rng) {
    fprintf(stderr, "failed to create RNG\n");
    return 1;
  }
  if (!randompack_seed((int)seed, 0, 0, rng)) {
    fprintf(stderr, "failed to seed RNG\n");
    randompack_free(rng);
    return 1;
  }

  unif01_Gen *gen = unif01_CreateExternGen01("randompack PIT normal", GetU01);
  svaria_SampleProd(gen, 0, N, n, (int)r, (int)t);
  unif01_DeleteExternGen01(gen);
  randompack_free(rng);
  return 0;
}
