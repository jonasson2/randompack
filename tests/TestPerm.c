// -*- C -*-
#include <stdbool.h>
#include "randompack.h"
#include "TestUtil.h"
#include "xCheck.h"
#include "printX.h"

static bool is_permutation(int *x, int n) {
  bool *seen;
  bool ok = true;
  TEST_ALLOC(seen, n);
  for (int i = 0; i < n; i++) seen[i] = false;
  for (int i = 0; i < n; i++) {
    int v = x[i];
    if (v < 0 || v >= n || seen[v]) {
      ok = false;
      break;
    }
    seen[v] = true;
  }
  FREE(seen);
  return ok;
}

static void test_perm_api(void) {
  int len, lens[] = {1, 2, 3, 10};
  int perm[10];
  bool ok;
  int n = 0;
  char **engines = get_engines(&n);
  for (int i = 0; i < n; i++) {
	 randompack_rng *rng = create_seeded_rng(engines[i], 77);
	 ok = randompack_perm(perm, 0, rng);
	 check_success(ok, rng);
	 for (int j = 0; j < LEN(lens); j++) {
		len = lens[j];
		bool ok = randompack_perm(perm, len, rng);
		check_success(ok, rng);
		printIV("perm", perm, len);		
		xCheck(minv(perm, len) == 0 && maxv(perm, len) == len - 1);
		xCheck(is_permutation(perm, len));
	 }
	 randompack_free(rng);
  }
  randompack_rng *rng = create_seeded_rng(engines[0], 77);
  // Check frequency in first and third entry in several permutations.
  int counts[7] = {0};
  for (int i = 0; i < 4; i += 2) {
	 for (int j = 0; j < 10000; j++) {
		randompack_perm(perm, 7, rng);
		counts[perm[i]]++;
	 }
	 xCheckMsg(check_balanced_counts(counts, 7), "Permutation");
  }
  randompack_free(rng);
  free_engines(engines, n);
}

void TestPerm(void) {
  test_perm_api();
}
