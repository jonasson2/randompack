#include "random123/include/Random123/philox.h"
#include <stdio.h>
int main(void) {
  philox4x64_key_t key = {{1, 2}};
  philox4x64_ctr_t ctr = {{1, 2, 3, 4}};
  philox4x64_ctr_t result = philox4x64(ctr, key);
  printf("draw0: %llu\n", result.v[0]);
  printf("draw1: %llu\n", result.v[1]);
  printf("draw2: %llu\n", result.v[2]);
  return 0;
}
