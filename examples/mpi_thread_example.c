// -*- C -*-
// Hybrid MPI + C11 threads example:
// Each MPI rank runs T threads. Each thread gets its own stream via spawn_key=(rank, tid)
// and estimates max(U(0,1)) over n draws. Each rank returns its best max; rank 0 prints all.
//
// Compile (example): mpicc -std=c11 -O2 hybrid.c -lrandompack
// Run: mpirun -np 4 ./a.out
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <threads.h>

#include <mpi.h>
#include "randompack.h"

enum { T = 4 };        // threads per rank
static int n = 100000; // draws per thread
static int seed = 123;

static double thread_max[T];

typedef struct {
  int rank;
  int tid;
} task;

static int worker(void *arg) {
  task *t = arg;

  randompack_rng *rng = randompack_create(0);
  if (!rng) return 1;

  uint32_t spawn_key[2] = {t->rank, t->tid};
  randompack_seed(seed, spawn_key, 2, rng);

  double m = 0.0;
  for (int i=0; i<n; i++) {
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

  int rank = 0, nranks = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nranks);

  thrd_t th[T];
  task tasks[T];

  for (int tid=0; tid<T; tid++) {
    tasks[tid].rank = rank;
    tasks[tid].tid = tid;
    int rc = thrd_create(&th[tid], worker, &tasks[tid]);
    if (rc != thrd_success) MPI_Abort(MPI_COMM_WORLD, 2);
  }

  for (int tid=0; tid<T; tid++) {
    int rc = 0;
    thrd_join(th[tid], &rc);
    if (rc != 0) MPI_Abort(MPI_COMM_WORLD, 3);
  }

  double local_max = 0.0;
  for (int tid=0; tid<T; tid++)
    if (thread_max[tid] > local_max) local_max = thread_max[tid];

  double *all = 0;
  if (rank == 0) {
    all = malloc(nranks*sizeof(double));
    if (!all) MPI_Abort(MPI_COMM_WORLD, 4);
  }

  MPI_Gather(&local_max, 1, MPI_DOUBLE, all, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

  if (rank == 0) {
    for (int r=0; r<nranks; r++)
      printf("rank %d: max = %.17g\n", r, all[r]);
    free(all);
  }

  MPI_Finalize();
  return 0;
}
