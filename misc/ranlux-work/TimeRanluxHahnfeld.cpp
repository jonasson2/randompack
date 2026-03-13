// TimeRanluxHahnfeld.cpp: throughput (GB/s) of Hahnfeld/Moneta ranlux++.

#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <new>

#ifdef __cplusplus
extern "C" {
#endif
#include "getopt.h"
#include "randompack.h"
#include "ranluxpp.h"
#ifdef __cplusplus
}
#endif
#include "RanluxppEngine.h"

#if defined(_WIN32)
  #include <windows.h>
#elif defined(CLOCK_MONOTONIC)
  #include <time.h>
#else
  #include <time.h>
#endif

static int max2(int m, int n) {
  return m > n ? m : n;
}

typedef bool (*fill_u64_fn)(uint64_t out[], int n, void *rng);

static uint64_t clock_nsec(void) {
#ifdef _WIN32
  LARGE_INTEGER freq, counter;
  QueryPerformanceFrequency(&freq);
  QueryPerformanceCounter(&counter);
  return (uint64_t)((1000000000.0*(double)counter.QuadPart)/(double)freq.QuadPart);
#elif defined(CLOCK_MONOTONIC)
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
    return (uint64_t)ts.tv_sec*1000000000ULL + (uint64_t)ts.tv_nsec;
  }
#endif
  return (uint64_t)((1000000000.0*(double)clock())/(double)CLOCKS_PER_SEC);
}

static void print_help(void) {
  printf("TimeRanluxHahnfeld - measure RNG engine throughput\n");
  printf("Usage: TimeRanluxHahnfeld [options]\n\n");
  printf("Options:\n");
  printf("  -h            Show this help message\n");
  printf("  -t seconds    Benchmark time per engine (default 0.1)\n");
  printf("  -c chunk      Chunk size (values per fill, default 4096)\n");
  printf("  -s seed       RNG seed (default 7)\n\n");
  printf("Notes:\n");
  printf("  Reports throughput normalized to 64 random bits.\n");
  printf("  Hahnfeld IntRndm() returns 48 random bits per call.\n");
}

static bool get_options(int argc, char **argv,
                        double *bench_time, int *chunk, uint64_t *seed, bool *help) {
  opterr = 0;
  optind = 1;
  int opt;
  *bench_time = 0.1;
  *chunk = 4096;
  *seed = 7;
  *help = false;
  while ((opt = getopt(argc, argv, "ht:c:s:")) != -1) {
    switch (opt) {
      case 'h':
        *help = true;
        return true;
      case 't':
        *bench_time = atof(optarg);
        if (*bench_time <= 0) {
          return false;
        }
        break;
      case 'c':
        *chunk = atoi(optarg);
        if (*chunk <= 0) {
          return false;
        }
        break;
      case 's':
        *seed = strtoull(optarg, 0, 10);
        break;
      default:
        return false;
    }
  }
  if (optind < argc) {
    return false;
  }
  return true;
}

static inline void consume64(const void *p) {
  static volatile uint64_t sink;
  uint64_t u;
  memcpy(&u, p, sizeof(u));
  sink ^= u;
}

static bool fill_u64_hahn(uint64_t out[], int n, void *rng) {
  RanluxppEngine *r = (RanluxppEngine *)rng;
  for (int i = 0; i < n; i++) {
    out[i] = r->IntRndm();
  }
  return true;
}

static bool fill_u64_rp(uint64_t out[], int n, void *rng) {
  randompack_rng *r = (randompack_rng *)rng;
  return randompack_uint64(out, (size_t)n, 0, r);
}

static bool fill_u64_portable(uint64_t out[], int n, void *rng) {
  ranluxpp_t *r = (ranluxpp_t *)rng;
  int i = 0;
  while (i < n) {
    ranluxpp_nextstate(r);
    int take = n - i < 9 ? n - i : 9;
    memcpy(out + i, r->x, (size_t)take*sizeof(*out));
    i += take;
  }
  return true;
}

static void print_result(const char *name, double ns, int bits) {
  double ns64 = ns*64/bits;
  double gb64 = 8/ns64;
  printf("%-18s", name);
  printf(" %10.2f", ns64);
  printf(" %8.2f", gb64);
  printf("\n");
}

static void warmup_cpu(int n) {
  enum { NBUF = 1000 };
  uint64_t buf[NBUF];
  RanluxppEngine rng(1);
  int reps = max2(1, n*1000000/NBUF);
  for (int i = 0; i < reps; i++) {
    fill_u64_hahn(buf, NBUF, &rng);
  }
  consume64(&buf[NBUF - 1]);
}

static double time_u64(int chunk, double bench_time, fill_u64_fn fill, void *rng) {
  enum { M = 1000000 };
  uint64_t *buf = new (std::nothrow) uint64_t[chunk];
  if (!buf) {
    return 0;
  }
  int reps = max2(1, M/chunk);
  int64_t calls = 0;
  uint64_t t0 = clock_nsec();
  uint64_t deadline = t0 + (uint64_t)(bench_time*1e9);
  uint64_t t = t0;
  while (t < deadline) {
    for (int i = 0; i < reps; i++) {
      if (!fill(buf, chunk, rng)) {
        delete[] buf;
        return 0;
      }
      consume64(&buf[chunk - 1]);
    }
    calls += reps;
    t = clock_nsec();
  }
  delete[] buf;
  return (calls > 0) ? (t - t0)/((double)calls*chunk) : 0;
}

int main(int argc, char **argv) {
  double bench_time;
  int chunk;
  uint64_t seed;
  bool help;
  if (!get_options(argc, argv, &bench_time, &chunk, &seed, &help) || help) {
    print_help();
    return help ? 0 : 1;
  }
  warmup_cpu(100);
  printf("throughput:       GB/s (decimal), normalized to 64 random bits\n");
  printf("bench_time:       %.3f s per engine\n", bench_time);
  printf("chunk:            %d\n", chunk);
  printf("seed:             %" PRIu64 "\n\n", seed);
  printf("%-18s %10s %8s\n", "Engine", "ns/64bits", "GB/s");
  RanluxppEngine rng(seed);
  double ns = time_u64(chunk, bench_time, fill_u64_hahn, &rng);
  print_result("ranlux-hahnmon", ns, 48);
  randompack_rng *rng_rp = randompack_create("ranlux++");
  if (!rng_rp) {
    fprintf(stderr, "randompack_create failed: ranlux++\n");
    return 1;
  }
  if (!randompack_seed((int)seed, 0, 0, rng_rp)) {
    fprintf(stderr, "randompack_seed failed: ranlux++\n");
    randompack_free(rng_rp);
    return 1;
  }
  ns = time_u64(chunk, bench_time, fill_u64_rp, rng_rp);
  print_result("ranlux-randompack", ns, 64);
  randompack_free(rng_rp);
  ranluxpp_t portable;
  ranluxpp_init(&portable, seed, 2048);
  ns = time_u64(chunk, bench_time, fill_u64_portable, &portable);
  print_result("ranlux-portable", ns, 64);
  return 0;
}
