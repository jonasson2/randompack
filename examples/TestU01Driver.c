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
#include <unistd.h>

#include "randompack.h"
#include "TestU01.h"

static randompack_rng *rng;
static uint64_t word;
static int half = 0;

static unsigned int GetBits(void) {
  if (!half) {
    randompack_uint64(&word, 1, 0, rng);
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
    "Usage: %s [-h] [-e engine] (-s|-c|-b)\n\n"
    "Run TestU01 on a randompack RNG.\n\n"
    "Options:\n"
    "  -h          Show this help\n"
    "  -e engine   RNG engine to use (default engine if omitted)\n"
    "  -s          SmallCrush  (seconds)\n"
    "  -c          Crush       (minutes to tens of minutes)\n"
    "  -b          BigCrush    (hours)\n",
    prog
  );
}

int main(int argc, char **argv) {
  int opt;
  enum { NONE, SMALL, CRUSH, BIG } battery = NONE;

  const char *engine = 0;
  while ((opt = getopt(argc, argv, "e:scbh")) != -1) {
    switch (opt) {
      case 'e': engine = optarg; break;
      case 's': battery = SMALL; break;
      case 'c': battery = CRUSH; break;
      case 'b': battery = BIG; break;
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

  rng = randompack_create(engine);
  if (!rng) {
    fprintf(stderr, "failed to create RNG\n");
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
