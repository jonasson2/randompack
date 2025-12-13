// -*- C -*-
#include <stdint.h>
#include <threads.h>
#include <assert.h>
#include <stdlib.h>

#include "randompack.h"

enum { M = 10 };
static int n;          // shared
static int seed = 123; // deterministic root seed

typedef struct {
  int i;
  double *x;
} job;

static int worker(void *arg) {
  job *j = (job *)arg;

  randompack_rng *rng = randompack_create(0); // default engine
  assert(rng);

  uint32_t key[1] = {(uint32_t)j->i};
  randompack_seed_spawn(seed, key, 1, rng);

  randompack_u01(j->x, n, rng);

  randompack_free(rng);
  return 0;
}

int main(void) {
  n = 100000;

  double *x[M];
  for (int i=0; i<M; i++) {
    x[i] = malloc((size_t)n*sizeof(double));
    assert(x[i]);
  }

  thrd_t th[M];
  job jobs[M];

  for (int i=0; i<M; i++) {
    jobs[i].i = i;
    jobs[i].x = x[i];
    int rc = thrd_create(&th[i], worker, &jobs[i]);
    assert(rc == thrd_success);
  }

  for (int i=0; i<M; i++) {
    int rc = 0;
    thrd_join(th[i], &rc);
    assert(rc == 0);
  }

  // ...use x[0..M-1]...

  for (int i=0; i<M; i++)
    free(x[i]);

  return 0;
}
