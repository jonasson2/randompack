// RawStream.c
// Stream raw uint64 output from randompack to stdout for RNG testing.

#include <stdio.h>
#include <stdlib.h>
#include "getopt.h"
#include <string.h>
#include "randompack.h"

#define BUFWORDS 1024

static void usage(const char *prog){
  fprintf(stderr,
    "Usage: %s [-e engine]\n"
    "Stream raw uint64 random numbers to stdout.\n\n"
    "Options:\n"
    "  -e ENGINE   RNG engine to use (default engine if omitted)\n"
    "  -h          Show this help\n",
    prog
  );
}

int main(int argc, char **argv){
  const char *engine = 0;
  int opt;
  while ((opt = getopt(argc, argv, "he:")) != -1) {
    if (opt == 'e') engine = optarg;
    else if (opt == 'h') {
      usage(argv[0]);
      return 0;
    }
    else {
      usage(argv[0]);
      return 1;
    }
  }
  randompack_rng *rng = randompack_create(engine);
  if (!rng) {
    fprintf(stderr, "failed to create RNG\n");
    return 1;
  }
  uint64_t buf[BUFWORDS];
  for (;;) {
    if (!randompack_uint64(buf, BUFWORDS, 0, rng)) {
      fprintf(stderr, "randompack_uint64 failed\n");
      break;
    }
    if (fwrite(buf, sizeof(uint64_t), BUFWORDS, stdout) != BUFWORDS) break;
  }
  randompack_free(rng);
  return 0;
}
