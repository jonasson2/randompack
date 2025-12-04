// -*- C -*-
#include <stdint.h>
#include <stdbool.h>

#include "randompack.h"
#include "printX.h"
#include "xCheck.h"

#define RUN_TEST(x)        \
  do {                     \
    xCheckAddMsg(#x);      \
    test_##x();            \
  } while (0)

typedef struct {
  const char *full;
  const char *abbrev;
} engine_entry;

static uint64_t first_uint64(const char *engine) {
  uint64_t x = 0;
  randompack_rng *rng = randompack_create(engine, 123);
  xCheck(rng);
  bool ok = randompack_uint64(&x, 1, 0, rng);
  xCheck(ok);
  print64(engine, x);
  randompack_free(rng);
  return x;
}

static void test_engine_aliases(void) {
  engine_entry engines[] = {
    { "xorshift128+", "x128+" },
    { "xoshiro256**", "x256**" },
    { "xoshiro256++", "x256++" },
    { "chacha20",     "chacha20" },
  };
  const int n = (int)(sizeof engines / sizeof engines[0]);
  uint64_t vals[n];

  for (int i = 0; i < n; i++) {
    vals[i] = first_uint64(engines[i].full);
    uint64_t shortv = first_uint64(engines[i].abbrev);
    xCheck(vals[i] == shortv);
  }

  for (int i = 0; i < n; i++) {
    for (int j = i + 1; j < n; j++) {
      xCheck(vals[i] != vals[j]);
    }
  }
}

static void test_bad_engine_name(void) {
  randompack_rng *rng = randompack_create("garbage", 123); // garbage name
  xCheck(rng);
  const char *err = randompack_last_error(rng);
  xCheck(err && err[0]); // non-null, non-blank
  bool ok = randompack_uint64(0, 1, 0, rng); // null name
  xCheck(!ok);
  const char *err2 = randompack_last_error(rng);
  xCheck(err2 && err2[0]);
  printS("randompack_create with engine", "garbage");
  printS("last error", err);
  printMsg("randompack_create with null engine");
  printS("last error", err2);
  randompack_free(rng);
}

static void test_null_engine_name(void) {
  randompack_rng *rng = randompack_create(0, 123);
  xCheck(rng);
  const char *err = randompack_last_error(rng);
  xCheck(!err || !err[0]);
  uint64_t x = 0;
  bool ok = randompack_uint64(&x, 1, 0, rng);
  xCheck(ok);
  randompack_free(rng);
}

void TestRandomCreate(void) {
  RUN_TEST(engine_aliases);
  RUN_TEST(bad_engine_name);
  RUN_TEST(null_engine_name);
}
