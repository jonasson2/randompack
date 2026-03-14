// TestU01 driver for randompack RNGs
//
// To build, install TestU01 in <prefix> and build randompack with:
//
//    meson setup -C release -Dbuildtype=release -DTestU01=<prefix>
//    meson compile -C release
//
// Then get help with:
//
//    release/examples/TestU01Driver -h
//
// and run, for example for Small Crush:
//
//    release/examples/TestU01Driver -c

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>

#include "randompack.h"
#include "TestU01.h"

static randompack_rng *rng;
static uint64_t word;
static int half = 0;
static bool reverse_bits = false;
static bool low_only = false;
static bool low_bytes = false;

static uint64_t reverse_bits64(uint64_t x) {
  x = ((x >> 1) & 0x5555555555555555ULL) | ((x & 0x5555555555555555ULL) << 1);
  x = ((x >> 2) & 0x3333333333333333ULL) | ((x & 0x3333333333333333ULL) << 2);
  x = ((x >> 4) & 0x0f0f0f0f0f0f0f0fULL) | ((x & 0x0f0f0f0f0f0f0f0fULL) << 4);
  x = ((x >> 8) & 0x00ff00ff00ff00ffULL) | ((x & 0x00ff00ff00ff00ffULL) << 8);
  x = ((x >> 16) & 0x0000ffff0000ffffULL) | ((x & 0x0000ffff0000ffffULL) << 16);
  x = (x >> 32) | (x << 32);
  return x;
}

static unsigned int GetBits(void) {
  if (low_bytes) {
    unsigned int out = 0;
    for (int i = 0; i < 4; i++) {
      randompack_uint64(&word, 1, 0, rng);
      if (reverse_bits)
        word = reverse_bits64(word);
      out |= ((unsigned int)(word & 0xffULL)) << (8*i);
    }
    return out;
  }
  if (low_only) {
    randompack_uint64(&word, 1, 0, rng);
    if (reverse_bits)
      word = reverse_bits64(word);
    return word;
  }
  if (!half) {
    randompack_uint64(&word, 1, 0, rng);
    if (reverse_bits)
      word = reverse_bits64(word);
    half = 1;
    unsigned int low = word;
    return low;
  }
  else {
    half = 0;
    unsigned int high = word >> 32;
    return high;
  }
}

static void usage(const char *prog) {
  fprintf(stderr,
    "Usage: %s [-h] [-e engine] [-s seed] [-r] [-l] [-8] (-S|-C|-B)\n\n"
    "Run TestU01 on a randompack RNG.\n\n"
    "Options:\n"
    "  -h          Show this help\n"
    "  -e engine   RNG engine to use (default engine if omitted)\n"
    "  -s seed     Integer seed passed to randompack_seed\n"
    "  -r          Reverse bits in each generated uint64\n"
    "  -l          Feed only the low 32 bits of each uint64 to TestU01\n"
    "  -8          Pack the low 8 bits from 4 uint64 values into one uint32\n"
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
  while ((opt = getopt(argc, argv, "e:s:rl8SCBh")) != -1) {
    switch (opt) {
      case 'e': engine = optarg; break;
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
      case 'r': reverse_bits = true; break;
      case 'l': low_only = true; break;
      case '8': low_bytes = true; break;
      case 'S': battery = SMALL; break;
      case 'C': battery = CRUSH; break;
      case 'B': battery = BIG; break;
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
  if (low_only && low_bytes) {
    fprintf(stderr, "-l and -8 cannot be used together\n");
    return 1;
  }

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

  unif01_Gen *gen =
    unif01_CreateExternGenBits("randompack", GetBits);

  switch (battery) {
    case SMALL: bbattery_SmallCrush(gen); break;
    case CRUSH: bbattery_Crush(gen); break;
    case BIG:   bbattery_BigCrush(gen); break;
    default:    break;
  }

  unif01_DeleteExternGenBits(gen);
  randompack_free(rng);
  return 0;
}
