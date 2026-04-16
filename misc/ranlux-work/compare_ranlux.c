// -*- C -*-
// Compare portable and hahnmon ranlux++ implementations

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include "ranluxpp.h"

int main(void) {
  ranluxpp_t r;
  ranluxpp_init(&r, 1, 2048);

  uint64_t xorsum = 0;
  uint64_t last = 0;

  // Generate 900 values (100 states * 9 values per state)
  for (int i = 0; i < 100; i++) {
    ranluxpp_nextstate(&r);
    for (int j = 0; j < 9; j++) {
      xorsum ^= r.x[j];
      last = r.x[j];
    }
  }

  printf("portable: xorsum = 0x%016" PRIx64 ", last = 0x%016" PRIx64 "\n",
         xorsum, last);

  return 0;
}
