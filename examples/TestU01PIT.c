// TestU01 driver for PIT-transformed samples (normal or exponential)
//
// To build, install TestU01 in <prefix> and build randompack with:
//
//    meson setup -C release -Dbuildtype=release -DTestU01_prefix=<prefix>
//    meson compile -C release

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
static bool use_exp = false;
static bool stdin_mode = false;

static void fill_buffer(void) {
  int n = BUFSIZE;
  bool ok;
  if (stdin_mode) {
    size_t nread = fread(buf, sizeof(buf[0]), BUFSIZE, stdin);
    if (nread == 0) {
      fprintf(stderr, "stdin ended before TestU01 completed\n");
      exit(1);
    }
    if (nread < BUFSIZE) {
      fprintf(stderr, "short read from stdin\n");
      exit(1);
    }
  }
  else {
    if (use_exp) {
      if (use_float) ok = randompack_expf(fbuf, (size_t)n, 1, rng);
      else ok = randompack_exp(buf, (size_t)n, 1, rng);
    }
    else {
      if (use_float) ok = randompack_normf(fbuf, (size_t)n, rng);
      else ok = randompack_norm(buf, (size_t)n, rng);
    }
    if (!ok) {
      if (use_exp) {
        fprintf(stderr, use_float ? "randompack_expf failed\n" :
          "randompack_exp failed\n");
      }
      else {
        fprintf(stderr, use_float ? "randompack_normf failed\n" :
          "randompack_norm failed\n");
      }
      exit(1);
    }
  }
  for (int i = 0; i < n; i++) {
    double x = use_float ? (double)fbuf[i] : buf[i];
    double u;
    if (use_exp) u = -expm1(-x);
    else u = 0.5*erfc(-x/sqrt(2));
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
    "Usage: %s [-h] [-i] [-g engine] [-s seed] (-S|-C|-B) [-f] [-n|-e]\n\n"
    "Run TestU01 on PIT-transformed normal or exponential samples.\n\n"
    "Options:\n"
    "  -h          Show this help\n"
    "  -i          Read Float64 samples from stdin instead of using randompack\n"
    "  -g engine   RNG engine to use (default engine if omitted)\n"
    "  -s seed     Integer seed passed to randompack_seed\n"
    "  -f          Use float normals (randompack_normf)\n"
    "  -n          PIT of N(0,1) [default]\n"
    "  -e          PIT of Exp(1)\n"
    "  -S          SmallCrush  (seconds)\n"
    "  -C          Crush       (minutes to tens of minutes)\n"
    "  -B          BigCrush    (hours)\n",
    prog
  );
}

int main(int argc, char **argv) {
  int opt;
  enum { NONE, SMALL, CRUSH, BIG } battery = NONE;
  long seed = 1;
  bool have_seed = false;
  const char *engine = 0;
  while ((opt = getopt(argc, argv, "ig:s:SCBhfne")) != -1) {
    switch (opt) {
      case 'i': stdin_mode = true; break;
      case 'g': engine = optarg; break;
      case 's': {
        char *end = 0;
        seed = strtol(optarg, &end, 10);
        if (!optarg[0] || (end && end[0]) || seed < INT_MIN || seed > INT_MAX) {
          fprintf(stderr, "bad seed '%s'\n", optarg);
          return 1;
        }
        have_seed = true;
        break;
      }
      case 'S': battery = SMALL; break;
      case 'C': battery = CRUSH; break;
      case 'B': battery = BIG; break;
      case 'f': use_float = true; break;
      case 'n': use_exp = false; break;
      case 'e': use_exp = true; break;
      case 'h':
      default:
        usage(argv[0]);
        return (opt == 'h') ? 0 : 1;
    }
  }
  if (battery == NONE) {
    usage(argv[0]);
    return 1;
  }
  if (stdin_mode && use_float) {
    fprintf(stderr, "-f is not supported with -i\n");
    return 1;
  }
  if (!stdin_mode) {
    rng = randompack_create(engine);
    if (!rng) {
      fprintf(stderr, "failed to create RNG\n");
      return 1;
    }
    if (have_seed && !randompack_seed((int)seed, 0, 0, rng)) {
      fprintf(stderr, "failed to seed RNG\n");
      randompack_free(rng);
      return 1;
    }
  }
  unif01_Gen *gen = unif01_CreateExternGen01(
    stdin_mode ? (use_exp ? "stdin PIT exp" : "stdin PIT normal") :
    (use_exp ? "randompack PIT exp" : "randompack PIT normal"), GetU01);
  switch (battery) {
    case SMALL: bbattery_SmallCrush(gen); break;
    case CRUSH: bbattery_Crush(gen); break;
    case BIG:   bbattery_BigCrush(gen); break;
    default:    break;
  }
  unif01_DeleteExternGen01(gen);
  randompack_free(rng);
  return 0;
}
