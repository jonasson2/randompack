// -*- C -*-
// Tests for randompack_set_state.

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "randompack.h"
#include "randompack_config.h"
#include "TestUtil.h"
#include "xCheck.h"

static int engine_nstate(char *engine) {
  for (int i = 0; i < LEN(engine_table); i++) {
    if (!strcmp(engine_table[i].name, engine)) return engine_table[i].state_words;
  }
  return 0;
}

static randompack_rng *make_rng(char *engine) {
  randompack_rng *rng = randompack_create(engine);
  ASSERT(rng);
  check_rng_clean(rng);
  return rng;
}

static void test_invalid_args(void) {
  uint64_t state[8] = {1,2,3,4,5,6,7,8};
  int n = 0;
  char **engines = get_engines(&n);
  for (int i = 0; i < n; i++) {
    int nstate = engine_nstate(engines[i]);
    randompack_rng *rng = make_rng(engines[i]);
    bool ok = randompack_set_state(0, nstate, rng);
    check_failure(ok, rng);
    ok = randompack_set_state(state, -1, rng);
    check_failure(ok, rng);
    ok = randompack_set_state(state, nstate - 1, rng);
    check_failure(ok, rng);
    ok = randompack_set_state(state, nstate + 1, rng);
    check_failure(ok, rng);
    randompack_free(rng);
    xCheck(!randompack_set_state(state, nstate, 0));
  }
  free_engines(engines, n);
}

static void test_xorfamily_nonzero(void) {
  uint64_t zero[] = {0,0,0,0};
  char *engines[] = {"x256++", "x256**", "x256++simd", "xoro++", "x128+"};
  for (int i = 0; i < LEN(engines); i++) {
    randompack_rng *rng = make_rng(engines[i]);
    bool ok = randompack_set_state(zero, 4, rng);
    check_failure(ok, rng);
    randompack_free(rng);
  }
}

static void draw_uints(randompack_rng *rng, uint64_t *x, int n) {
  bool ok = randompack_uint64(x, n, 0, rng);
  check_success(ok, rng);
}

static void test_determinism(void) {
  enum { K = 20 };
  uint64_t x128[] = {0x0123456789abcdefULL, 0xfedcba9876543210ULL};
  uint64_t squares[] = {1,2};
  uint64_t x256pp[] = {1,2,3,4};
  uint64_t x256ss[] = {5,6,7,8};
  uint64_t pcg[] = {9,10,11,13};
  uint64_t cwg[] = {1,2,3,4,5};
  uint64_t philox[] = {1,2,3,4,5,6};
  uint64_t chacha[] = {
    0x0123456789abcdefULL,
    0xfedcba9876543210ULL,
    0x1111111111111111ULL,
    0x2222222222222222ULL,
    0x3333333333333333ULL,
    0x4444444444444444ULL
  };
  struct {
    char *engine;
    uint64_t *state;
    int nstate;
  } cases[] = {
    {"x128+", x128, LEN(x128)},
    {"squares", squares, LEN(squares)},
    {"x256++", x256pp, LEN(x256pp)},
    {"x256**", x256ss, LEN(x256ss)},
    {"pcg64", pcg, LEN(pcg)},
    {"cwg128", cwg, LEN(cwg)},
    {"philox", philox, LEN(philox)},
    {"chacha20", chacha, LEN(chacha)},
  };

  for (int i = 0; i < LEN(cases); i++) {
    uint64_t a[K], b[K];
    randompack_rng *rng = make_rng(cases[i].engine);
    bool ok = randompack_set_state(cases[i].state, cases[i].nstate, rng);
    check_success(ok, rng);
    draw_uints(rng, a, K);
    ok = randompack_set_state(cases[i].state, cases[i].nstate, rng);
    check_success(ok, rng);
    draw_uints(rng, b, K);
    xCheck(equal_vec64(a, b, K));
    randompack_free(rng);
  }
}

static void test_buf_reset(void) {
  uint64_t state[] = {1,2,3,4};
  randompack_rng *rng = make_rng("x256++");
  bool ok = randompack_set_state(state, LEN(state), rng);
  check_success(ok, rng);
  uint32_t u0[1];
  ok = randompack_uint32(u0, 1, 0, rng);
  check_success(ok, rng);
  ok = randompack_set_state(state, LEN(state), rng);
  check_success(ok, rng);
  uint32_t u1[1];
  ok = randompack_uint32(u1, 1, 0, rng);
  check_success(ok, rng);
  xCheck(equal_vec32(u0, u1, 1));
  randompack_free(rng);
}

static void test_philox_set_state(void) {
  randompack_philox_ctr ctr = {{1, 2, 3, 4}};
  randompack_philox_key key = {{5, 6}};
  randompack_rng *rng = make_rng("philox");
  bool ok = randompack_philox_set_state(ctr, key, rng);
  check_success(ok, rng);
  uint64_t a[4], b[4];
  ok = randompack_uint64(a, LEN(a), 0, rng);
  check_success(ok, rng);
  ok = randompack_philox_set_state(ctr, key, rng);
  check_success(ok, rng);
  ok = randompack_uint64(b, LEN(b), 0, rng);
  check_success(ok, rng);
  xCheck(equal_vec64(a, b, LEN(a)));
  randompack_free(rng);
  rng = make_rng("squares");
  ok = randompack_philox_set_state(ctr, key, rng);
  check_failure(ok, rng);
  randompack_free(rng);
}

static void test_pcg_set_state(void) {
  uint128_t c1 = 12345678901ULL, pcgstate = c1*c1*c1, inc = c1*c1;
  randompack_rng *rng = make_rng("pcg64");
  bool ok = randompack_pcg64_set_state(pcgstate, inc, rng);
  check_success(ok, rng);
  uint64_t a[4], b[4];
  ok = randompack_uint64(a, LEN(a), 0, rng);
  check_success(ok, rng);
  ok = randompack_pcg64_set_state(pcgstate, inc, rng);
  check_success(ok, rng);
  ok = randompack_uint64(b, LEN(b), 0, rng);
  check_success(ok, rng);
  xCheck(equal_vec64(a, b, LEN(a)));
  randompack_free(rng);
  rng = make_rng("squares");
  ok = randompack_pcg64_set_state(pcgstate, inc, rng);
  check_failure(ok, rng);
  ok = randompack_pcg64_set_state(pcgstate, 2, rng);
  check_failure(ok, rng);
  randompack_free(rng);
}

static void test_squares_set_state(void) {
  uint64_t ctr = 7;
  uint64_t key = 11;
  randompack_rng *rng = make_rng("squares");
  bool ok = randompack_squares_set_state(ctr, key, rng);
  check_success(ok, rng);
  uint64_t a[4], b[4];
  ok = randompack_uint64(a, LEN(a), 0, rng);
  check_success(ok, rng);
  ok = randompack_squares_set_state(ctr, key, rng);
  check_success(ok, rng);
  ok = randompack_uint64(b, LEN(b), 0, rng);
  check_success(ok, rng);
  xCheck(equal_vec64(a, b, LEN(a)));
  randompack_free(rng);
  rng = make_rng("philox");
  ok = randompack_squares_set_state(ctr, key, rng);
  check_failure(ok, rng);
  randompack_free(rng);
}

void TestSetState(void) {
  test_invalid_args();
  test_xorfamily_nonzero();
  test_determinism();
  test_buf_reset();
  test_philox_set_state();
  test_squares_set_state();
  test_pcg_set_state();
}
