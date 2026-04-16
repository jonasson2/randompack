// Verify that hahnmon RanluxppEngine and portable ranluxpp agree
#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include "RanluxppEngine.h"

extern "C" {
#include "ranluxpp.h"
}

// Extract 48-bit value from portable state (matching how hahnmon does it)
static uint64_t portable_to_48bit(ranluxpp_t *r, int *pos) {
  if (*pos >= 9) {
    ranluxpp_nextstate(r);
    *pos = 0;
  }
  uint64_t val = r->x[*pos];
  (*pos)++;
  // Extract 48 bits (matching RanluxppEngine::IntRndm())
  return val & 0xFFFFFFFFFFFFULL;
}

int main(void) {
  // Test with seed=314159265 (from hahnmon test suite)
  uint64_t seed = 314159265;
  RanluxppEngine hahnmon(seed);
  ranluxpp_t portable;
  ranluxpp_init(&portable, seed, 2048);

  printf("Comparing hahnmon and portable implementations (seed=%" PRIu64 "):\n\n", seed);

  // Compare first 100 48-bit values
  int pos = 0;
  int mismatches = 0;
  for (int i = 0; i < 100; i++) {
    uint64_t h_val = hahnmon.IntRndm();
    uint64_t p_val = portable_to_48bit(&portable, &pos);

    if (h_val != p_val) {
      if (mismatches < 5) {
        printf("MISMATCH at position %d:\n", i+1);
        printf("  hahnmon: 0x%012" PRIx64 "\n", h_val);
        printf("  portable: 0x%012" PRIx64 "\n\n", p_val);
      }
      mismatches++;
    }
  }

  if (mismatches == 0) {
    printf("✓ All 100 values match!\n");
    printf("  Hahnmon and portable implementations agree.\n");
  }
  else {
    printf("✗ Found %d mismatches out of 100 values\n", mismatches);
    printf("  Implementations DO NOT agree.\n");
  }

  return mismatches > 0 ? 1 : 0;
}
