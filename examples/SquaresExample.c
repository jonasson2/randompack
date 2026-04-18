// Simple pthread example.
// Launch M threads. Each thread derives an independent RNG stream from a
// common seed by setting a different squares key and computes max(U(0,1))
// over N draws. Error checking is omitted for simplicity.

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

static void *worker(void *arg) {
  job *j = arg;
  randompack_rng *rng = randompack_create("squares");
  randompack_seed(seed, 0, 0, rng);
  randompack_squares_set_key(j->stream_id, rng);
  double m = 0;
  for (int i=0; i<N; i++) {
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
  for (int i=0; i<M; i++) {
    jobs[i].stream_id = i;
    jobs[i].result = 0;
    pthread_create(&th[i], 0, worker, &jobs[i]);
  }
  for (int i=0; i<M; i++)
    pthread_join(th[i], 0);
  for (int i=0; i<M; i++)
    printf("stream %d: max = %.6f\n", i, jobs[i].result);
  return 0;
}