#include <cstdint>

#define private public
#include "RanluxppEngine.h"
#undef private

extern "C" {

bool hahnmon_state_fill(uint64_t out[], int n, void *rng) {
  RanluxppEngine *r = (RanluxppEngine *)rng;
  int i = 0;
  while (i < n) {
    r->Advance();
    int take = n - i < 9 ? n - i : 9;
    for (int j = 0; j < take; j++) {
      out[i + j] = r->fState[j];
    }
    i += take;
  }
  return true;
}

}
