// -*- C -*-
// Tests for randompack_set_state.

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "randompack.h"
#include "randompack_config.h"
#include "test_util.h"
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
  char *engines[] = {"x256++", "x256**", "x256++simd", "xoro++", "x128+",
    "ranlux++"};
  for (int i = 0; i < LEN(engines); i++) {
    uint64_t zero[9] = {0};
    int nstate = engine_nstate(engines[i]);
    randompack_rng *rng = make_rng(engines[i]);
    bool ok = randompack_set_state(zero, nstate, rng);
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
  uint64_t cwg[] = {1,0,2,0,3,0,4,0};
  uint64_t philox[] = {1,2,3,4,5,6};
  uint64_t sfc64simd[] = {7,11,13,17};
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
    {"sfc64simd", sfc64simd, LEN(sfc64simd)},
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
    CHECK_EQUALV(a, b, K);
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
  CHECK_EQUALV(u0, u1, 1);
  randompack_free(rng);
}

static void test_philox_set_key(void) {
  uint64_t key[2] = {5, 6};
  uint64_t state[6] = {1, 2, 3, 4, 0, 0};
  randompack_rng *rng = make_rng("philox");
  bool ok = randompack_set_state(state, LEN(state), rng);
  check_success(ok, rng);
  ok = randompack_philox_set_key(key, rng);
  check_success(ok, rng);
  uint64_t a[4], b[4];
  ok = randompack_uint64(a, LEN(a), 0, rng);
  check_success(ok, rng);
  ok = randompack_set_state(state, LEN(state), rng);
  check_success(ok, rng);
  ok = randompack_philox_set_key(key, rng);
  check_success(ok, rng);
  ok = randompack_uint64(b, LEN(b), 0, rng);
  check_success(ok, rng);
  CHECK_EQUALV(a, b, LEN(a));
  randompack_free(rng);
  rng = make_rng("squares");
  ok = randompack_philox_set_key(key, rng);
  check_failure(ok, rng);
  randompack_free(rng);
}

static void test_pcg_set_increment(void) {
  uint64_t c1 = 1234567ULL;
  uint64_t state[4] = {c1*c1*c1, 0, 1, 0};
  uint64_t inc[2] = {c1*c1 | 1ULL, c1};
  uint64_t inc_even[2] = {2, c1};
  randompack_rng *rng = make_rng("pcg64");
  bool ok = randompack_set_state(state, LEN(state), rng);
  check_success(ok, rng);
  ok = randompack_pcg64_set_inc(inc, rng);
  check_success(ok, rng);
  uint64_t a[4], b[4];
  ok = randompack_uint64(a, LEN(a), 0, rng);
  check_success(ok, rng);
  ok = randompack_set_state(state, LEN(state), rng);
  check_success(ok, rng);
  ok = randompack_pcg64_set_inc(inc, rng);
  check_success(ok, rng);
  ok = randompack_uint64(b, LEN(b), 0, rng);
  check_success(ok, rng);
  CHECK_EQUALV(a, b, LEN(a));
  ok = randompack_pcg64_set_inc(inc_even, rng);
  check_failure(ok, rng);
  randompack_free(rng);
  rng = make_rng("squares");
  ok = randompack_pcg64_set_inc(inc, rng);
  check_failure(ok, rng);
  randompack_free(rng);
}

static void test_cwg_set_increment(void) {
  uint64_t state[8] = {1, 0, 7, 0, 11, 0, 13, 0};
  uint64_t inc[2] = {3, 5};
  uint64_t inc_even[2] = {2, 5};
  randompack_rng *rng = make_rng("cwg128");
  bool ok = randompack_set_state(state, LEN(state), rng);
  check_success(ok, rng);
  ok = randompack_cwg128_set_inc(inc, rng);
  check_success(ok, rng);
  uint64_t a[4], b[4];
  ok = randompack_uint64(a, LEN(a), 0, rng);
  check_success(ok, rng);
  ok = randompack_set_state(state, LEN(state), rng);
  check_success(ok, rng);
  ok = randompack_cwg128_set_inc(inc, rng);
  check_success(ok, rng);
  ok = randompack_uint64(b, LEN(b), 0, rng);
  check_success(ok, rng);
  CHECK_EQUALV(a, b, LEN(a));
  ok = randompack_cwg128_set_inc(inc_even, rng);
  check_failure(ok, rng);
  randompack_free(rng);
  rng = make_rng("pcg64");
  ok = randompack_cwg128_set_inc(inc, rng);
  check_failure(ok, rng);
  randompack_free(rng);
}

static void test_chacha_set_nonce(void) {
  uint32_t nonce[3] = {7, 11, 13};
  uint64_t state0[6] = {1, 2, 3, 4, 0, 17ULL << 32};
  uint64_t state1[6] = {1, 2, 3, 4, 7ULL | (11ULL << 32), 13ULL | (17ULL << 32)};
  randompack_rng *rng = make_rng("chacha20");
  bool ok = randompack_set_state(state0, LEN(state0), rng);
  check_success(ok, rng);
  ok = randompack_set_chacha_nonce(nonce, rng);
  check_success(ok, rng);
  uint64_t a[4], b[4];
  ok = randompack_uint64(a, LEN(a), 0, rng);
  check_success(ok, rng);
  randompack_free(rng);
  rng = make_rng("chacha20");
  ok = randompack_set_state(state1, LEN(state1), rng);
  check_success(ok, rng);
  ok = randompack_uint64(b, LEN(b), 0, rng);
  check_success(ok, rng);
  CHECK_EQUALV(a, b, LEN(a));
  randompack_free(rng);
  rng = make_rng("pcg64");
  ok = randompack_set_chacha_nonce(nonce, rng);
  check_failure(ok, rng);
  randompack_free(rng);
}

static void test_squares_set_key(void) {
  uint64_t key = 11;
  uint64_t state[2] = {7, 0};
  randompack_rng *rng = make_rng("squares");
  bool ok = randompack_set_state(state, LEN(state), rng);
  check_success(ok, rng);
  ok = randompack_squares_set_key(key, rng);
  check_success(ok, rng);
  uint64_t a[4], b[4];
  ok = randompack_uint64(a, LEN(a), 0, rng);
  check_success(ok, rng);
  ok = randompack_set_state(state, LEN(state), rng);
  check_success(ok, rng);
  ok = randompack_squares_set_key(key, rng);
  check_success(ok, rng);
  ok = randompack_uint64(b, LEN(b), 0, rng);
  check_success(ok, rng);
  CHECK_EQUALV(a, b, LEN(a));
  randompack_free(rng);
  rng = make_rng("philox");
  ok = randompack_squares_set_key(key, rng);
  check_failure(ok, rng);
  randompack_free(rng);
}

static void test_sfc64_set_abc(void) {
  uint64_t abc[3] = {7, 11, 13};
  uint64_t state[4] = {1, 2, 3, 17};
  randompack_rng *rng = make_rng("sfc64");
  bool ok = randompack_set_state(state, LEN(state), rng);
  check_success(ok, rng);
  uint64_t a[4], b[4];
  ok = randompack_sfc64_set_abc(abc, rng);
  check_success(ok, rng);
  ok = randompack_uint64(a, LEN(a), 0, rng);
  check_success(ok, rng);
  randompack_free(rng);
  rng = make_rng("sfc64");
  state[0] = abc[0];
  state[1] = abc[1];
  state[2] = abc[2];
  ok = randompack_set_state(state, LEN(state), rng);
  check_success(ok, rng);
  ok = randompack_uint64(b, LEN(b), 0, rng);
  check_success(ok, rng);
  CHECK_EQUALV(a, b, LEN(a));
  randompack_free(rng);
  rng = make_rng("philox");
  ok = randompack_sfc64_set_abc(abc, rng);
  check_failure(ok, rng);
  randompack_free(rng);
}

void TestSetState(void) {
  test_invalid_args();
  test_xorfamily_nonzero();
  test_determinism();
  test_buf_reset();
  test_philox_set_key();
  test_squares_set_key();
  test_sfc64_set_abc();
  test_pcg_set_increment();
  test_cwg_set_increment();
  test_chacha_set_nonce();
}
