#include "random123/include/Random123/philox.h"
#include <stdio.h>
#include <inttypes.h>

int main(void) {
  philox4x64_key_t key = {{1, 2}};
  philox4x64_ctr_t ctr = {{1, 2, 3, 4}};
  philox4x64_ctr_t result = philox4x64(ctr, key);
  printf("Draws:\n");
  printf("%" PRIu64 "\n", result.v[0]);
  printf("%" PRIu64 "\n", result.v[1]);
  printf("%" PRIu64 "\n", result.v[2]);
  printf("%" PRIu64 "\n", result.v[3]);
  ctr.v[0]++;
  result = philox4x64(ctr, key);  
  printf("%" PRIu64 "\n", result.v[0]);
  return 0;
}
