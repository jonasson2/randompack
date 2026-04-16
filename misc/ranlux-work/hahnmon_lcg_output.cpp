// Extract LCG state from hahnmon to compare with portable
#include <cstdio>
#include <cstdint>
#include <cstring>
#include "RanluxppEngine.h"
#include "ranluxpp/ranlux_lcg.h"

int main(void) {
  RanluxppEngine rng(1);

  uint64_t buf[9*100];

  // Need to access internal state - but it's private!
  // This won't work without modifying the class

  fprintf(stderr, "Error: Cannot access RanluxppEngine internal state\n");
  fprintf(stderr, "The fState field is private and there's no getter\n");

  return 1;
}
