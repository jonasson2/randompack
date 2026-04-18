// Simple pthread example.
// Launch M threads. A base RNG is created from a common seed. Each thread gets
// a duplicate of the previous thread's RNG, jumped ahead by 2^64 steps, and
// computes max(U(0,1)) over N draws. Error checking is omitted for simplicity.

#include <stdio.h>
#include <pthread.h>

#include "randompack.h"

enum { M = 10, N = 100000 };
static int seed = 42;

typedef struct {
  int stream_id;
  double result;
  randompack_rng *rng;
} job;

static void *worker(void *arg) {
  job *j = arg;
  double m = 0;
  for (int i=0; i<N; i++) {
    double u;
    randompack_u01(&u, 1, j->rng);
    if (u > m) m = u;
  }
  j->result = m;
  randompack_free(j->rng);
  return 0;
}

int main(void) {
  pthread_t th[M];
  job jobs[M];
  randompack_rng *base = randompack_create(0);
  randompack_seed(seed, 0, 0, base);
  jobs[0].stream_id = 0;
  jobs[0].result = 0;
  jobs[0].rng = randompack_duplicate(base);
  for (int i=1; i<M; i++) {
    jobs[i].stream_id = i;
    jobs[i].result = 0;
    jobs[i].rng = randompack_duplicate(jobs[i-1].rng);
    randompack_jump(64, jobs[i].rng);
  }
  randompack_free(base);
  for (int i=0; i<M; i++)
    pthread_create(&th[i], 0, worker, &jobs[i]);
  for (int i=0; i<M; i++)
    pthread_join(th[i], 0);
  for (int i=0; i<M; i++)
    printf("stream %d: max = %.6f\n", jobs[i].stream_id, jobs[i].result);
  return 0;
}