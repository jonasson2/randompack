// -*- C++ -*-
// Reference values from Hahnfeld/Moneta ranlux++ implementation

#include <cinttypes>
#include <cstdint>
#include <cstdio>

#include "RanluxppEngine.h"

int main(void) {
  RanluxppEngine r(1);
  // Set state to {1, 2, 3, 4, 5, 6, 7, 8, 9} to match portable version
  // Note: RanluxppEngine doesn't expose direct state setting, so we'll
  // use SetSeed(1) and note the difference

  uint64_t xorsum = 0;
  uint64_t last = 0;

  // Generate 900 values (100 states * 9 values each)
  for (int i = 0; i < 900; i++) {
    uint64_t val = r.IntRndm();
    xorsum ^= val;
    last = val;
  }

  printf("xorsum = 0x%016" PRIx64 "\n", xorsum);
  printf("last   = 0x%016" PRIx64 "\n", last);

  return 0;
}
