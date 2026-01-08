#include <stdbool.h>
#include <string.h>
#include "randompack.h"
#include "randompack_config.h"
static int check_engines(void) {
  int n = 0;
  int emax = 0;
  int dmax = 0;
  char *engines = 0;
  char *descriptions = 0;
  bool ok = randompack_engines(0, 0, &n, &emax, &dmax);
  if (!ok) return 1;
  if (n < 6 || emax < 6 || dmax < 20) return 1;
  if (!ALLOC(engines, n*emax)) return 1;
  if (!ALLOC(descriptions, n*dmax)) {
    FREE(engines);
    return 1;
  }
  ok = randompack_engines(engines, descriptions, &n, &emax, &dmax);
  if (!ok) {
    FREE(engines);
    FREE(descriptions);
    return 1;
  }
  for (int i = 0; i < n; i++) {
    char *name = engines + i*emax;
    if (!strcmp(name, "pcg64") || !strcmp(name, "cwg128")) {
      FREE(engines);
      FREE(descriptions);
      return 1;
    }
  }
  FREE(engines);
  FREE(descriptions);
  return 0;
}

static int check_create_failures(void) {
  char *names[] = { "pcg64", "cwg128" };
  for (int i = 0; i < LEN(names); i++) {
    randompack_rng *rng = randompack_create(names[i]);
    if (!rng) return 1;
    char *err = randompack_last_error(rng);
    if (!err || !err[0]) {
      randompack_free(rng);
      return 1;
    }
    uint64_t x = 0;
    bool ok = randompack_uint64(&x, 1, 0, rng);
    if (ok) {
      randompack_free(rng);
      return 1;
    }
    randompack_free(rng);
  }
  return 0;
}

static int check_create_success(void) {
  randompack_rng *rng = randompack_create("x256++");
  if (!rng) return 1;
  char *err = randompack_last_error(rng);
  if (err && err[0]) {
    randompack_free(rng);
    return 1;
  }
  if (!randompack_seed(123, 0, 0, rng)) {
    randompack_free(rng);
    return 1;
  }
  double x[1] = {0};
  if (!randompack_u01(x, 1, rng)) {
    randompack_free(rng);
    return 1;
  }
  if (x[0] == 0) {
    randompack_free(rng);
    return 1;
  }
  randompack_free(rng);
  return 0;
}

int main(void) {
  if (check_engines()) return 1;
  if (check_create_failures()) return 1;
  if (check_create_success()) return 1;
  return 0;
}
