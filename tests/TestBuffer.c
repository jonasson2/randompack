// -*- C -*-
#include <stdbool.h>
#include <stdint.h>

#include "randompack.h"
#include "randompack_config.h"
#include "TestUtil.h"
#include "xCheck.h"

static void test_split_u64(char *engine) {
  const int n = BUFSIZE + 13;
  const int n1 = BUFSIZE - 3;
  uint64_t *a, *b;
  TEST_ALLOC(a, n);
  TEST_ALLOC(b, n);
  randompack_rng *r1 = create_seeded_rng(engine, 7);
  randompack_rng *r2 = create_seeded_rng(engine, 7);
  xCheck(r1 && r2);
  xCheck(randompack_uint64(a, n, 0, r1));
  xCheck(randompack_uint64(b, n1, 0, r2));
  xCheck(randompack_uint64(b + n1, n - n1, 0, r2));
  xCheck(equal_vec64(a, b, n));
  randompack_free(r1);
  randompack_free(r2);
  FREE(a);
  FREE(b);
}

static void test_unaligned_u16(char *engine) {
  const int n = BUFSIZE*5;
  uint8_t toss[5];
  uint16_t *a, *b;
  TEST_ALLOC(a, n);
  TEST_ALLOC(b, n);
  randompack_rng *r1 = create_seeded_rng(engine, 11);
  randompack_rng *r2 = create_seeded_rng(engine, 11);
  xCheck(r1 && r2);
  xCheck(randompack_uint8(toss, 3, 0, r1));
  xCheck(randompack_uint16(a, n, 0, r1));
  xCheck(randompack_uint8(toss, 3, 0, r2));
  xCheck(randompack_uint16(b, n, 0, r2));
  xCheck(equal_vec16(a, b, n));
  randompack_free(r1);
  randompack_free(r2);
  FREE(a);
  FREE(b);
}

static void test_unaligned_u32(char *engine) {
  const int n = BUFSIZE*3;
  uint8_t toss[5];
  uint32_t *a, *b;
  TEST_ALLOC(a, n);
  TEST_ALLOC(b, n);
  randompack_rng *r1 = create_seeded_rng(engine, 13);
  randompack_rng *r2 = create_seeded_rng(engine, 13);
  xCheck(r1 && r2);
  xCheck(randompack_uint8(toss, 1, 0, r1));
  xCheck(randompack_uint32(a, n, 0, r1));
  xCheck(randompack_uint8(toss, 1, 0, r2));
  xCheck(randompack_uint32(b, n, 0, r2));
  xCheck(equal_vec32(a, b, n));
  randompack_free(r1);
  randompack_free(r2);
  FREE(a);
  FREE(b);
}

void TestBuffer(void) {
  int n = 0;
  char **engines = get_engines(&n);
  for (int i = 0; i < n; i++) {
    char *e = engines[i];
    test_split_u64(e);
    test_unaligned_u16(e);
    test_unaligned_u32(e);
  }
  free_engines(engines, n);
}
