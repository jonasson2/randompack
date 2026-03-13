// -*- C -*-

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#include "ranluxpp.h"

int main(void) {
  ranluxpp_t r;
  uint64_t init[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
  uint64_t xorsum = 0;
  uint64_t last = 0;
  ranluxpp_init(&r, 1, 2048);
  for (int i = 0; i < 9; i++) {
    r.x[i] = init[i];
  }
  for (int i = 0; i < 100; i++) {
    ranluxpp_nextstate(&r);
    for (int j = 0; j < 9; j++) {
      xorsum ^= r.x[j];
      last = r.x[j];
    }
  }
  printf("xorsum = 0x%016" PRIx64 "\n", xorsum);
  printf("last   = 0x%016" PRIx64 "\n", last);
  return 0;
}
