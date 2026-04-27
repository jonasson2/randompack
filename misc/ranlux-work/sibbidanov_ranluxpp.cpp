#include <cstdint>
#include <new>

#include "ranluxpp.h"

extern "C" {

bool sibbidanov_ranluxpp_fill(double out[], int n, void *rng) {
  ranluxpp *r = (ranluxpp *)rng;
  r->getarray(n, out);
  return true;
}

bool sibbidanov_ranluxpp_fill_u64(uint64_t out[], int n, void *rng) {
  ranluxpp *r = (ranluxpp *)rng;
  int i = 0;
  while (i < n) {
    r->nextstate();
    uint64_t *x = r->getstate();
    int take = n - i < 9 ? n - i : 9;
    for (int j = 0; j < take; j++) {
      out[i + j] = x[j];
    }
    i += take;
  }
  return true;
}

void *sibbidanov_ranluxpp_create(uint64_t seed) {
  return new (std::nothrow) ranluxpp(seed);
}

void sibbidanov_ranluxpp_destroy(void *rng) {
  delete (ranluxpp *)rng;
}

}
