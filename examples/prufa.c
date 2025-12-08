#include <stdio.h>
#include <stdlib.h>
#include "randompack.h"

int main(void)
{
  randompack_rng *rng = create_seeded_rng("Park-Miller", 42);
  if (!rng) {
    fprintf(stderr, "Failed to create RNG\n");
    return 1;
  }

  double x[3];
  if (!randompack_u01(x, 3, rng)) {
    fprintf(stderr, "randompack_u01 failed\n");
    randompack_free(rng);
    return 1;
  }

  for (int i = 0; i < 3; i++) {
    printf("%.4f\n", x[i]);
  }

  randompack_free(rng);
  return 0;
}
