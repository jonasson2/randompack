#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <getopt.h>
#include <ctype.h>

#include "allocate.h"
#include "randompack.h"

#define FAIL(...) do { fprintf(stderr, __VA_ARGS__); \
                       fputc('\n', stderr); return false; } while (0)

static void print_help(void) {
  printf("RunRandom — simple driver for randompack RNGs\n");
  printf("Usage: RunRandom [options]\n\n");
  printf("Options:\n");
  printf("  -h         Show this help message\n");
  printf("  -n number  Number of random values to generate (default 3)\n");
  printf("  -P         Use Park–Miller RNG (default: Xorshift128+)\n");
  printf("  -e engine  RNG engine: x128, x*, x+, pcg, cha, sys\n");
  printf("  -s seed    RNG seed (default randomized; 42 if -P and no seed)\n\n");
  printf("Examples:\n");
  printf("  RunRandom -P          # Park–Miller, seed 42, 3 values\n");
  printf("  RunRandom -P -n 5     # Park–Miller, seed 42, 5 values\n");
  printf("  RunRandom -s 123      # Xorshift128+, seed 123\n");
  printf("  RunRandom -e x+ -s 1  # Xoshiro256++, seed 1\n");
}

static bool get_options(int argc, char **argv, int *n, bool *ParkMiller, bool *help,
                        int *seed, char **engine) {
  opterr = 0;
  optind = 1;
  int opt;
  *ParkMiller = *help = false;
  *seed = -1;  // indicates no seed provided
  *n = 3;      // default: 3 values
  *engine = 0;

  while ((opt = getopt(argc, argv, "hn:Pe:s:")) != -1) {
    switch (opt) {
      case 'h':
        *help = true;
        return true;
      case 'n':
        *n = atoi(optarg);
        if (*n <= 0)
          FAIL("Number of values must be positive");
        break;
      case 'P':
        *ParkMiller = true;
        break;
      case 'e':
        *engine = optarg;
        break;
      case 's':
        *seed = atoi(optarg);
        break;
      case '?':
      default:
        FAIL("Illegal option: %c", optopt);
    }
  }

  if (optind < argc)
    FAIL("Too many arguments");

  return true;
}

static const char *resolve_engine(const char *opt) {
  if (!opt) return 0;
  char t[16];
  int i;
  for (i = 0; opt[i] && i + 1 < (int)sizeof t; i++)
    t[i] = (char)tolower((unsigned char)opt[i]);
  t[i] = 0;
  if (!strcmp(t, "x128")) return "x128+";
  if (!strcmp(t, "x*")) return "x256**";
  if (!strcmp(t, "x+")) return "x256++";
  if (!strcmp(t, "pcg")) return "pcg";
  if (!strcmp(t, "cha")) return "chacha20";
  if (!strcmp(t, "sys")) return "system";
  return 0;
}

int main(int argc, char **argv) {
  bool ok, ParkMiller, help;
  int n, seed;
  char *engine;

  ok = get_options(argc, argv, &n, &ParkMiller, &help, &seed, &engine);
  if (help || !ok) {
    print_help();
    return ok ? 0 : 1;
  }

  const char *rngtype = 0;
  uint64_t rngseed;

  if (ParkMiller)
    rngtype = "PM";
  else if (engine) {
    rngtype = resolve_engine(engine);
    if (!rngtype) {
      fprintf(stderr, "Unknown engine: %s\n", engine);
      return 1;
    }
  }
  else
    rngtype = "Xorshift";

  if (seed == -1) {
    rngseed = ParkMiller ? 42 : 0;  // PM→42, others→randomize (0)
  }
  else {
    rngseed = (uint64_t)seed;
  }

  randompack_rng *rng = randompack_create(rngtype, rngseed);
  if (!rng) {
    fprintf(stderr, "Failed to create RNG (type=\"%s\", seed=%lld)\n",
            rngtype, (long long)rngseed);
    return 1;
  }

  double *x;
  allocate(x, n);

  if (!randompack_u01(x, n, rng)) {
    fprintf(stderr, "randompack_u01 failed\n");
    freem(x);
    randompack_free(rng);
    return 1;
  }

  for (int i = 0; i < n; i++) {
    printf("%.4f\n", x[i]);
  }

  freem(x);
  randompack_free(rng);
  return 0;
}
