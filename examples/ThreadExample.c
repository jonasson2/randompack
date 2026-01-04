// -*- C -*-
// Simple pthread example.
// Launch M threads. Each thread derives an independent RNG stream from a
// common seed via a spawn key and computes max(U(0,1)) over N draws.

#include <stdint.h>
#include <stdio.h>
#include <pthread.h>

#include "randompack.h"

enum { M = 10, N = 100000 };
static int seed = 42;

typedef struct {
  int stream_id;
  double result;
} job;

// Thread entry point: independent RNG stream, simple scalar summary
static void *worker(void *arg) {
  job *j = arg;
  randompack_rng *rng = randompack_create(0);
  if (!rng) return 0;
  uint32_t spawn_key[1];
  spawn_key[0] = j->stream_id;
  if (!randompack_seed(seed, spawn_key, 1, rng)) {
    randompack_free(rng);
    return 0;
  }
  double m = 0;
  for (int i = 0; i < N; i++) {
    double u;
    randompack_u01(&u, 1, rng);
    if (u > m) m = u;
  }
  j->result = m;
  randompack_free(rng);
  return 0;
}

int main(void) {
  pthread_t th[M];
  job jobs[M];
  for (int i = 0; i < M; i++) {
    jobs[i].stream_id = i;
    jobs[i].result = 0;
    pthread_create(&th[i], 0, worker, &jobs[i]);
  }
  for (int i = 0; i < M; i++)
    pthread_join(th[i], 0);
  for (int i = 0; i < M; i++)
    printf("stream %d: max = %.6f\n", i, jobs[i].result);
  return 0;
}
