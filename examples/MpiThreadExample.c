// -*- C -*-
// Hybrid MPI + pthreads example.
// Each MPI rank runs T threads. Each thread uses its own RNG stream derived
// from a common seed via spawn_key = (rank, tid).
// Each thread computes max(U(0,1)) over n draws. Each rank reports its best max;
// rank 0 gathers and prints results from all ranks.

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include <mpi.h>
#include "randompack.h"

enum { T = 4 };
static int n = 100000;
static int seed = 123;

// Per-rank storage for thread-local results (one slot per thread)
static double thread_max[T];

typedef struct {
  int rank;
  int tid;
} task;

// Thread entry point: create an RNG, derive a substream using (rank, tid),
// and compute the maximum of n uniform draws.
static void *worker(void *arg) {
  task *t = arg;
  randompack_rng *rng = randompack_create(0);
  if (!rng) return 0;
  uint32_t spawn_key[2];
  spawn_key[0] = t->rank;
  spawn_key[1] = t->tid;
  if (!randompack_seed(seed, spawn_key, 2, rng)) {
    randompack_free(rng);
    return 0;
  }
  double m = 0;
  for (int i = 0; i < n; i++) {
    double u;
    randompack_u01(&u, 1, rng);
    if (u > m) m = u;
  }
  thread_max[t->tid] = m;
  randompack_free(rng);
  return 0;
}

int main(int argc, char **argv) {
  MPI_Init(&argc, &argv);
  int rank = 0;
  int nranks = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nranks);
  pthread_t th[T];
  task tasks[T];

  // Launch T threads on this rank, each with a distinct spawn key
  for (int tid = 0; tid < T; tid++) {
    tasks[tid].rank = rank;
    tasks[tid].tid = tid;
  }
  for (int tid = 0; tid < T; tid++) {
    if (pthread_create(&th[tid], 0, worker, &tasks[tid]) != 0)
      MPI_Abort(MPI_COMM_WORLD, 2);
  }

  for (int tid = 0; tid < T; tid++)
    pthread_join(th[tid], 0);

  // Reduce thread-local results to one value per rank
  double local_max = 0;
  for (int tid = 0; tid < T; tid++)
    if (thread_max[tid] > local_max) local_max = thread_max[tid];

  // Gather per-rank maxima to rank 0
  double *all = 0;
  if (rank == 0) {
    all = malloc(nranks*sizeof(double));
    if (!all) MPI_Abort(MPI_COMM_WORLD, 4);
  }

  MPI_Gather(&local_max, 1, MPI_DOUBLE,
             all, 1, MPI_DOUBLE,
             0, MPI_COMM_WORLD);

  if (rank == 0) {
    for (int r = 0; r < nranks; r++)
      printf("rank %d: max = %.17g\n", r, all[r]);
    free(all);
  }

  MPI_Finalize();
  return 0;
}
