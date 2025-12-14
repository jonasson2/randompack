// -*- C -*-
#include <stdbool.h>

#include "randompack.h"
#include "TestUtil.h"
#include "xCheck.h"

static void test_sample_api(void) {
  enum { N = 50 };
  int n = N;
  int k = 10;
  int sample[10];
  bool *seen;
  xCheck(ALLOC(seen, n));
  randompack_rng *rng = create_seeded_rng("xoshiro256++", 11);
  bool ok = randompack_sample(sample, n, k, rng);
  check_success(ok, rng);
  for (int i = 0; i < n; i++) seen[i] = false;
  for (int i = 0; i < k; i++) {
    int v = sample[i];
    xCheck(v >= 0 && v < n);
    xCheck(!seen[v]);
    seen[v] = true;
  }
  // Check frequency in first and third entry in several samples.
  int counts[7] = {0};
  for (int i = 0; i < 4; i += 2) {
	 for (int j = 0; j < 10000; j++) {
		randompack_sample(sample, 7, 4, rng);
      counts[sample[i]]++;
    }
    xCheckMsg(check_balanced_counts(counts, 7), "Sample");
  }
  randompack_free(rng);
  FREE(seen);
}

void TestSample(void) {
  test_sample_api();
}
