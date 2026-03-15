// examples/DrawDemo.c
// Purpose: deterministic demo that prints a few u01 draws for selected engines.
// Matching R script: r-package/inst/examples/DrawDemo.R
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "randompack.h"

static char *fmt1 = "%s seed=%d spawnkey=%d u01=%.17g %.17g\n";
static char *fmt2 = "%s counter=%d key=%d u01=%.17g %.17g\n";

static void draw_u01(const char *engine, int seed, int key) {
  randompack_rng *rng = randompack_create(engine);
  if (!rng) {
    printf("%s unavailable\n", engine);
    return;
  }
  randompack_seed(seed, (uint32_t*)&key, 1, rng);
  double x[2];
  randompack_u01(x, 2, rng);
  printf(fmt1, engine, seed, key, x[0], x[1]);
  randompack_free(rng);
}

static void draw_philox(int ctr0, int key0) {
  randompack_rng *rng = randompack_create("philox");
  if (!rng) {
    printf("philox unavailable\n");
    return;
  }
  uint64_t state[6] = {ctr0, 0, 0, 0, 0, 0};
  uint64_t key[2] = {key0, 0};
  randompack_set_state(state, 6, rng);
  randompack_philox_set_key(key, rng);
  double x[2];
  randompack_u01(x, 2, rng);
  printf(fmt2, "philox", (int)ctr0, (int)key0, x[0], x[1]);
  randompack_free(rng);
}

static void draw_squares(uint64_t ctr, int key) {
  randompack_rng *rng = randompack_create("squares");
  uint64_t state[2] = {ctr, 0};
  randompack_set_state(state, 2, rng);
  randompack_squares_set_key(key, rng);
  double x[2];
  randompack_u01(x, 2, rng);
  printf(fmt2, "squares", (int)ctr, key, x[0], x[1]);
  randompack_free(rng);
}

int main(void) {
  draw_u01("x256++", 42, 2);
  draw_u01("pcg64", 42, 2);
  draw_philox(1, 42);
  draw_u01("chacha20", 42, 2);
  draw_squares(1, 123456789);
  return 0;
}
