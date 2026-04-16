// -*- C++ -*-
// Compare portable and hahnmon ranlux++ implementations

#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <cstring>

extern "C" {
#include "ranluxpp.h"
}
#include "RanluxppEngine.h"

int main(void) {
  // Test 1: Compare using seed=1, p=2048
  ranluxpp_t portable;
  ranluxpp_init(&portable, 1, 2048);

  RanluxppEngine hahnmon(1);

  uint64_t portable_xor = 0;
  uint64_t portable_last = 0;
  uint64_t hahnmon_xor = 0;
  uint64_t hahnmon_last = 0;

  // Generate 900 64-bit values from portable (100 states * 9 values)
  for (int i = 0; i < 100; i++) {
    ranluxpp_nextstate(&portable);
    for (int j = 0; j < 9; j++) {
      portable_xor ^= portable.x[j];
      portable_last = portable.x[j];
    }
  }

  // Generate 1200 48-bit values from hahnmon (to get same number of bits: 900*64/48=1200)
  for (int i = 0; i < 1200; i++) {
    uint64_t val = hahnmon.IntRndm();
    hahnmon_xor ^= val;
    hahnmon_last = val;
  }

  printf("Portable (64-bit values):\n");
  printf("  xorsum = 0x%016" PRIx64 "\n", portable_xor);
  printf("  last   = 0x%016" PRIx64 "\n\n", portable_last);

  printf("Hahnmon (48-bit values):\n");
  printf("  xorsum = 0x%016" PRIx64 "\n", hahnmon_xor);
  printf("  last   = 0x%016" PRIx64 "\n\n", hahnmon_last);

  // More meaningful comparison: Compare first 10 raw state values after seeding
  ranluxpp_t p2;
  ranluxpp_init(&p2, 1, 2048);
  RanluxppEngine h2(1);

  printf("Comparison after seeding with seed=1, p=2048:\n");
  printf("First state from portable: ");
  for (int i = 0; i < 9; i++) {
    printf("%016" PRIx64 " ", p2.x[i]);
  }
  printf("\n");

  // Note: Can't easily access hahnmon's internal state without modifying the class
  printf("(Hahnmon state is private - would need to modify class to compare)\n");

  return 0;
}
