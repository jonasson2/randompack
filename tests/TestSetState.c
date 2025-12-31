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
  if (!strcmp(engine, "xorshift128+")) return 2;
  if (!strcmp(engine, "squares64")) return 2;
  if (!strcmp(engine, "xoshiro256++")) return 4;
  if (!strcmp(engine, "xoshiro256**")) return 4;
  if (!strcmp(engine, "pcg64") || !strcmp(engine, "pcg64_dxsm")) return 4;
  if (!strcmp(engine, "cwg128") || !strcmp(engine, "cwg128_64")) return 5;
  if (!strcmp(engine, "philox")) return 6;
  if (!strcmp(engine, "chacha20")) return 6;
  return 0;
}

static randompack_rng *make_rng(char *engine) {
  randompack_rng *rng = randompack_create(engine);
  ASSERT(rng);
  check_rng_clean(rng);
  return rng;
}

static void test_invalid_args(void) {
  char *engines[] = {
    "xorshift128+",
    "squares64",
    "xoshiro256++",
    "xoshiro256**",
    "pcg64",
    "cwg128",
    "philox",
    "chacha20",
  };
  uint64_t state[8] = {1,2,3,4,5,6,7,8};
  for (int i = 0; i < LEN(engines); i++) {
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
}

static void test_xoshiro_nonzero(void) {
  uint64_t zero[] = {0,0,0,0};
  char *engines[] = {"xoshiro256++", "xoshiro256**"};
  for (int i = 0; i < LEN(engines); i++) {
    randompack_rng *rng = make_rng(engines[i]);
    bool ok = randompack_set_state(zero, 4, rng);
    check_failure(ok, rng);
    randompack_free(rng);
  }
}

static void test_pcg_inc_odd(void) {
  uint64_t even_inc[] = {1,2,4,5};
  uint64_t odd_inc[] = {1,2,5,6};
  randompack_rng *rng = make_rng("pcg64");

  bool ok = randompack_set_state(even_inc, 4, rng);
  check_failure(ok, rng);

  ok = randompack_set_state(odd_inc, 4, rng);
  check_success(ok, rng);

  randompack_free(rng);
}

static void test_cwg_s_odd(void) {
  uint64_t even_s[] = {1,2,3,4,6};
  uint64_t odd_s[] = {1,2,3,4,5};
  randompack_rng *rng = make_rng("cwg128");
  bool ok = randompack_set_state(even_s, LEN(even_s), rng);
  check_failure(ok, rng);
  ok = randompack_set_state(odd_s, LEN(odd_s), rng);
  check_success(ok, rng);
  randompack_free(rng);
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
    {"xorshift128+", x128, LEN(x128)},
    {"squares64", squares, LEN(squares)},
    {"xoshiro256++", x256pp, LEN(x256pp)},
    {"xoshiro256**", x256ss, LEN(x256ss)},
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
  randompack_rng *rng = make_rng("xoshiro256++");

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
  randompack_counter ctr = {{1, 2, 3, 4}};
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
  rng = make_rng("squares64");
  ok = randompack_philox_set_state(ctr, key, rng);
  check_failure(ok, rng);
  randompack_free(rng);
}

static void test_squares64_set_state(void) {
  uint64_t ctr = 7;
  uint64_t key = 11;
  randompack_rng *rng = make_rng("squares64");
  bool ok = randompack_squares64_set_state(ctr, key, rng);
  check_success(ok, rng);
  uint64_t a[4], b[4];
  ok = randompack_uint64(a, LEN(a), 0, rng);
  check_success(ok, rng);
  ok = randompack_squares64_set_state(ctr, key, rng);
  check_success(ok, rng);
  ok = randompack_uint64(b, LEN(b), 0, rng);
  check_success(ok, rng);
  xCheck(equal_vec64(a, b, LEN(a)));
  randompack_free(rng);
  rng = make_rng("philox");
  ok = randompack_squares64_set_state(ctr, key, rng);
  check_failure(ok, rng);
  randompack_free(rng);
}

void TestSetState(void) {
  test_invalid_args();
  test_xoshiro_nonzero();
  test_pcg_inc_odd();
  test_cwg_s_odd();
  test_determinism();
  test_buf_reset();
  test_philox_set_state();
  test_squares64_set_state();
}
