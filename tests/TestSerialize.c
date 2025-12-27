// -*- C -*-
// Tests for randompack_serialize / randompack_deserialize.
//   (yes I'll admit it, it's created with ChatGPT)
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "randompack.h"
#include "randompack_config.h"
#include "TestUtil.h"
#include "xCheck.h"

enum { STATE_MIN_NEED_TEST = 64 + 8*BUFSIZE }; // The tests version of STATE_MIN_NEED

// --- helpers -------------------------------------------------------------
static engine_table_entry *find_engine_meta(const char *name) {
  for (int i=0; i<LEN(engine_table); i++)
    if (!strcmp(engine_table[i].name, name))
      return &engine_table[i];
  return 0;
}

static bool pcg_supported(void) {
  randompack_rng *rng = randompack_create("pcg64");
  if (!rng) return false;
  char *err = randompack_last_error(rng);
  bool ok = (!err || !err[0]);
  randompack_free(rng);
  return ok;
}

static void make_state(uint64_t *s, int n, rng_engine e) {
  for (int i=0; i<n; i++)
    s[i] = i + 1;
  // PCG increment must be odd
  if (e == PCG64)
    s[2] |= 1ULL;
}

static uint8_t *serialize_rng(randompack_rng *rng, int *len) {
  bool ok = randompack_serialize(0, len, rng);
  check_success(ok, rng);
  uint8_t *buf = 0;
  TEST_ALLOC(buf, *len);
  ok = randompack_serialize(buf, len, rng);
  check_success(ok, rng);
  return buf;
}

static bool equal_vecd_bits(double *a, double *b, int n) {
  uint64_t ua[32], ub[32];
  xCheck(n <= LEN(ua));
  memcpy(ua, a, n*sizeof(double));
  memcpy(ub, b, n*sizeof(double));
  return equal_vec64(ua, ub, n);
}

static void draw_mix(randompack_rng *rng,
                     uint32_t *u32, int nu32,
                     double *z, int nz,
                     uint64_t *u64, int nu64) {
  bool ok = randompack_uint32(u32, nu32, 0, rng);
  check_success(ok, rng);
  ok = randompack_norm(z, nz, rng);
  check_success(ok, rng);
  ok = randompack_uint64(u64, nu64, 0, rng);
  check_success(ok, rng);
}
// --- tests ---------------------------------------------------------------
static void test_sys_rejected(void) {
  int need = 0;
  randompack_rng *rng = randompack_create("system");
  ASSERT(rng);
  bool ok = randompack_serialize(0, &need, rng);
  check_failure(ok, rng);
  uint8_t buf[STATE_MIN_NEED_TEST];
  CLEAR(buf);
  uint32_t hdr[2] = {1u, SYS};
  memcpy(buf, hdr, sizeof hdr);
  ok = randompack_deserialize(buf, STATE_MIN_NEED_TEST, rng);
  check_failure(ok, rng);
  randompack_free(rng);
}

static void test_engine_mismatch_rejected(void) {
  randompack_rng *r1 = randompack_create("chacha20");
  randompack_rng *r2 = randompack_create("xoshiro256++");
  ASSERT(r1 && r2);
  engine_table_entry *m = find_engine_meta("chacha20");
  uint64_t state[8];
  make_state(state, m->state_words, r1->engine);
  bool ok = randompack_set_state(state, m->state_words, r1);
  check_success(ok, r1);
  int len = 0;
  uint8_t *buf = serialize_rng(r1, &len);
  ok = randompack_deserialize(buf, len, r2);
  check_failure(ok, r2);
  FREE(buf);
  randompack_free(r1);
  randompack_free(r2);
}

static void test_corrupt_header_rejected(void) {
  randompack_rng *rng = randompack_create("xoshiro256++");
  ASSERT(rng);
  engine_table_entry *m = find_engine_meta("xoshiro256++");
  uint64_t state[8];
  make_state(state, m->state_words, rng->engine);
  bool ok = randompack_set_state(state, m->state_words, rng);
  check_success(ok, rng);
  int len = 0;
  uint8_t *buf = serialize_rng(rng, &len);
  uint32_t *hdr = (uint32_t *)buf;
  hdr[0] = 2;            // bad version
  ok = randompack_deserialize(buf, len, rng);
  check_failure(ok, rng);
  hdr[0] = 1;
  hdr[1] = 99;           // bad engine
  ok = randompack_deserialize(buf, len, rng);
  check_failure(ok, rng);
  FREE(buf);
  randompack_free(rng);
}

static void test_serialize_roundtrip_and_truncation(void) {
  bool pcg_ok = pcg_supported();
  for (int i=0; i<LEN(engines); i++) {
    if (!pcg_ok && !strcmp(engines[i], "pcg64")) continue;
    engine_table_entry *m = find_engine_meta(engines[i]);
    xCheck(m != 0);
    randompack_rng *r1 = randompack_create(engines[i]);
    randompack_rng *r2 = randompack_create(engines[i]);
    ASSERT(r1 && r2);
    uint64_t state[8];
    make_state(state, m->state_words, r1->engine);
    bool ok = randompack_set_state(state, m->state_words, r1);
    check_success(ok, r1);
    int need = 0;
    ok = randompack_serialize(0, &need, r1);
    check_success(ok, r1);
    xCheck(need == STATE_MIN_NEED_TEST);
    uint8_t *buf = serialize_rng(r1, &need);
    uint32_t u32a[23], u32b[23];
    double   za[11],  zb[11];
    uint64_t u64a[17], u64b[17];
    draw_mix(r1, u32a, 23, za, 11, u64a, 17);
    ok = randompack_deserialize(buf, need, r2);
    check_success(ok, r2);
    draw_mix(r2, u32b, 23, zb, 11, u64b, 17);
    xCheck(equal_vec32(u32a, u32b, 23));
    xCheck(equal_vecd_bits(za, zb, 11));
    xCheck(equal_vec64(u64a, u64b, 17));
    FREE(buf);
    randompack_free(r1);
    randompack_free(r2);
  }
}

static void test_buffer_serialized(void) {
  uint32_t a[4], b[4];
  randompack_rng *r1 = create_seeded_rng("xoshiro256++", 5);
  randompack_rng *r2 = randompack_create("xoshiro256++");
  randompack_rng *r3 = create_seeded_rng("xoshiro256++", 5);
  xCheck(r1 && r2 && r3);
  bool ok = randompack_uint32(a, 2, 0, r1);
  check_success(ok, r1);
  int len = 0;
  uint8_t *buf = serialize_rng(r1, &len);
  ok = randompack_deserialize(buf, len, r2);
  check_success(ok, r2);
  ok = randompack_uint32(a + 2, 2, 0, r2);
  check_success(ok, r2);
  ok = randompack_uint32(b, 4, 0, r3);
  check_success(ok, r3);
  xCheck(equal_vec32(a, b, 4));
  FREE(buf);
  randompack_free(r1);
  randompack_free(r2);
  randompack_free(r3);
}
// --- entry point ---------------------------------------------------------
void TestSerialize(void) {
  test_sys_rejected();
  test_engine_mismatch_rejected();
  test_corrupt_header_rejected();
  test_serialize_roundtrip_and_truncation();
  test_buffer_serialized();
}
