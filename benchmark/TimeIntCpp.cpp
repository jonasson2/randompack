// TimeIntCpp.cpp: Compare C++ std::random vs Randompack for integer draws (ns/value)

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <random>
#include <vector>

extern "C" {
#include "randompack.h"
}

int compute_reps(int chunk) {
  return std::max(1, 1000000/chunk);
}

template<typename T>
double time_int_range_std(std::mt19937_64 &rng, int chunk, double bench_time,
  T m, T n) {
  int reps = compute_reps(chunk);
  std::uniform_int_distribution<T> dist(m, n);
  std::vector<T> buf(chunk);
  int calls = 0;
  auto t0 = std::chrono::high_resolution_clock::now();
  auto t = t0;
  auto limit = std::chrono::duration<double>(bench_time);
  while (std::chrono::duration<double>(t - t0) < limit) {
    for (int i = 0; i < reps; i++) {
      for (int j = 0; j < chunk; j++) {
        buf[j] = dist(rng);
      }
    }
    calls += reps;
    t = std::chrono::high_resolution_clock::now();
  }
  long long total = static_cast<long long>(calls)*chunk;
  if (total <= 0) {
    return NAN;
  }
  auto elapsed = std::chrono::duration<double>(t - t0).count();
  return 1e9*elapsed/total;
}

double time_int_range_rp(randompack_rng *rng, int chunk, double bench_time,
  int m, int n) {
  int reps = compute_reps(chunk);
  std::vector<int> buf(chunk);
  int calls = 0;
  auto t0 = std::chrono::high_resolution_clock::now();
  auto t = t0;
  auto limit = std::chrono::duration<double>(bench_time);
  while (std::chrono::duration<double>(t - t0) < limit) {
    for (int i = 0; i < reps; i++) {
      randompack_int(buf.data(), chunk, m, n, rng);
    }
    calls += reps;
    t = std::chrono::high_resolution_clock::now();
  }
  long long total = static_cast<long long>(calls)*chunk;
  if (total <= 0) {
    return NAN;
  }
  auto elapsed = std::chrono::duration<double>(t - t0).count();
  return 1e9*elapsed/total;
}

double time_long_long_range_std(std::mt19937_64 &rng, int chunk, double bench_time,
  long long m, long long n) {
  int reps = compute_reps(chunk);
  std::uniform_int_distribution<long long> dist(m, n);
  std::vector<long long> buf(chunk);
  int calls = 0;
  auto t0 = std::chrono::high_resolution_clock::now();
  auto t = t0;
  auto limit = std::chrono::duration<double>(bench_time);
  while (std::chrono::duration<double>(t - t0) < limit) {
    for (int i = 0; i < reps; i++) {
      for (int j = 0; j < chunk; j++) {
        buf[j] = dist(rng);
      }
    }
    calls += reps;
    t = std::chrono::high_resolution_clock::now();
  }
  long long total = static_cast<long long>(calls)*chunk;
  if (total <= 0) {
    return NAN;
  }
  auto elapsed = std::chrono::duration<double>(t - t0).count();
  return 1e9*elapsed/total;
}

double time_long_long_range_rp(randompack_rng *rng, int chunk, double bench_time,
  long long m, long long n) {
  int reps = compute_reps(chunk);
  std::vector<long long> buf(chunk);
  int calls = 0;
  auto t0 = std::chrono::high_resolution_clock::now();
  auto t = t0;
  auto limit = std::chrono::duration<double>(bench_time);
  while (std::chrono::duration<double>(t - t0) < limit) {
    for (int i = 0; i < reps; i++) {
      randompack_long_long(buf.data(), chunk, m, n, rng);
    }
    calls += reps;
    t = std::chrono::high_resolution_clock::now();
  }
  long long total = static_cast<long long>(calls)*chunk;
  if (total <= 0) {
    return NAN;
  }
  auto elapsed = std::chrono::duration<double>(t - t0).count();
  return 1e9*elapsed/total;
}

double time_perm_std(std::mt19937_64 &rng, double bench_time, int n) {
  int reps = std::max(1, 100000/n);
  std::vector<int> buf(n);
  for (int i = 0; i < n; i++) {
    buf[i] = i;
  }
  int calls = 0;
  auto t0 = std::chrono::high_resolution_clock::now();
  auto t = t0;
  auto limit = std::chrono::duration<double>(bench_time);
  while (std::chrono::duration<double>(t - t0) < limit) {
    for (int i = 0; i < reps; i++) {
      std::shuffle(buf.begin(), buf.end(), rng);
    }
    calls += reps;
    t = std::chrono::high_resolution_clock::now();
  }
  long long total = static_cast<long long>(calls)*n;
  if (total <= 0) {
    return NAN;
  }
  auto elapsed = std::chrono::duration<double>(t - t0).count();
  return 1e9*elapsed/total;
}

double time_perm_rp(randompack_rng *rng, double bench_time, int n) {
  int reps = std::max(1, 100000/n);
  std::vector<int> buf(n);
  int calls = 0;
  auto t0 = std::chrono::high_resolution_clock::now();
  auto t = t0;
  auto limit = std::chrono::duration<double>(bench_time);
  while (std::chrono::duration<double>(t - t0) < limit) {
    for (int i = 0; i < reps; i++) {
      randompack_perm(buf.data(), n, rng);
    }
    calls += reps;
    t = std::chrono::high_resolution_clock::now();
  }
  long long total = static_cast<long long>(calls)*n;
  if (total <= 0) {
    return NAN;
  }
  auto elapsed = std::chrono::duration<double>(t - t0).count();
  return 1e9*elapsed/total;
}

double time_sample_std(std::mt19937_64 &rng, double bench_time, int n, int k) {
  int reps = std::max(1, 100000/n);
  std::vector<int> population(n);
  std::vector<int> work(n);
  std::vector<int> buf(k);
  for (int i = 0; i < n; i++) {
    population[i] = i;
  }
  int calls = 0;
  auto t0 = std::chrono::high_resolution_clock::now();
  auto t = t0;
  auto limit = std::chrono::duration<double>(bench_time);
  while (std::chrono::duration<double>(t - t0) < limit) {
    for (int i = 0; i < reps; i++) {
      work = population;
      std::shuffle(work.begin(), work.end(), rng);
      std::copy_n(work.begin(), k, buf.begin());
    }
    calls += reps;
    t = std::chrono::high_resolution_clock::now();
  }
  long long total = static_cast<long long>(calls)*k;
  if (total <= 0) {
    return NAN;
  }
  auto elapsed = std::chrono::duration<double>(t - t0).count();
  return 1e9*elapsed/total;
}

double time_sample_rp(randompack_rng *rng, double bench_time, int n, int k) {
  int reps = std::max(1, 100000/n);
  std::vector<int> buf(k);
  int calls = 0;
  auto t0 = std::chrono::high_resolution_clock::now();
  auto t = t0;
  auto limit = std::chrono::duration<double>(bench_time);
  while (std::chrono::duration<double>(t - t0) < limit) {
    for (int i = 0; i < reps; i++) {
      randompack_sample(buf.data(), n, k, rng);
    }
    calls += reps;
    t = std::chrono::high_resolution_clock::now();
  }
  long long total = static_cast<long long>(calls)*k;
  if (total <= 0) {
    return NAN;
  }
  auto elapsed = std::chrono::duration<double>(t - t0).count();
  return 1e9*elapsed/total;
}

void warmup(double seconds) {
  std::mt19937_64 std_rng(123);
  randompack_rng *rp_rng = randompack_create("x256++simd");
  randompack_seed(123, 0, 0, rp_rng);

  std::uniform_int_distribution<int32_t> dist1(1, 1000);
  std::uniform_int_distribution<int64_t> dist2(1, 1000000);
  std::vector<int32_t> buf32(1024);
  std::vector<int64_t> buf64(1024);
  std::vector<int> perm_buf(1000);
  for (int i = 0; i < 1000; i++) {
    perm_buf[i] = i;
  }

  auto t0 = std::chrono::high_resolution_clock::now();
  auto limit = std::chrono::duration<double>(seconds);
  while (std::chrono::duration<double>(
    std::chrono::high_resolution_clock::now() - t0) < limit) {
    for (int i = 0; i < 1024; i++) {
      buf32[i] = dist1(std_rng);
      buf64[i] = dist2(std_rng);
    }
    std::shuffle(perm_buf.begin(), perm_buf.end(), std_rng);
    randompack_int(buf32.data(), 1024, 1, 1000, rp_rng);
    randompack_long_long(buf64.data(), 1024, 1LL, 1000000LL, rp_rng);
    randompack_perm(perm_buf.data(), 1000, rp_rng);
  }
  randompack_free(rp_rng);
}

int main(int argc, char **argv) {
  double bench_time = 0.2;
  int chunk = 4096;
  uint64_t seed = 0;
  bool use_seed = false;
  const char *engine = "x256++simd";

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      printf("Usage: TimeIntCpp [-t sec] [-c chunk] [-s seed] [-e engine]\n");
      printf("  -t sec     time per case in seconds (default: 0.2)\n");
      printf("  -c chunk   number of draws per call (default: 4096)\n");
      printf("  -s seed    fixed seed (default random seed per case)\n");
      printf("  -e engine  Randompack engine (default: x256++simd)\n");
      return 0;
    }
    else if (strcmp(argv[i], "-t") == 0) {
      if (i + 1 >= argc) {
        fprintf(stderr, "missing value for -t\n");
        return 1;
      }
      bench_time = atof(argv[++i]);
      if (bench_time <= 0) {
        fprintf(stderr, "time per case must be positive\n");
        return 1;
      }
    }
    else if (strcmp(argv[i], "-c") == 0) {
      if (i + 1 >= argc) {
        fprintf(stderr, "missing value for -c\n");
        return 1;
      }
      chunk = atoi(argv[++i]);
      if (chunk <= 0) {
        fprintf(stderr, "chunk must be positive\n");
        return 1;
      }
    }
    else if (strcmp(argv[i], "-s") == 0) {
      if (i + 1 >= argc) {
        fprintf(stderr, "missing value for -s\n");
        return 1;
      }
      seed = strtoull(argv[++i], 0, 10);
      use_seed = true;
    }
    else if (strcmp(argv[i], "-e") == 0) {
      if (i + 1 >= argc) {
        fprintf(stderr, "missing value for -e\n");
        return 1;
      }
      engine = argv[++i];
    }
    else {
      fprintf(stderr, "Unknown option: %s\n", argv[i]);
      return 1;
    }
  }

  warmup(0.1);

  printf("time per value:   ns/value\n");
  printf("bench_time:       %.3f s per case\n", bench_time);
  printf("chunk:            %d\n", chunk);
  printf("engine:           %s\n", engine);
  printf("\n");
  printf("%-18s %10s %11s %8s\n", "Benchmark", "std", "Randompack", "Factor");

  uint64_t case_seed = use_seed ? seed : std::random_device{}();
  std::mt19937_64 std_rng(case_seed);
  randompack_rng *rp_rng = randompack_create(engine);
  randompack_seed(case_seed, 0, 0, rp_rng);
  double std_ns = time_int_range_std<int32_t>(std_rng, chunk, bench_time, 1, 10);
  std_rng.seed(case_seed);
  randompack_seed(case_seed, 0, 0, rp_rng);
  double rp_ns = time_int_range_rp(rp_rng, chunk, bench_time, 1, 10);
  double factor = std_ns/rp_ns;
  printf("%-18s %10.2f %11.2f %8.2f\n", "int 1-10", std_ns, rp_ns, factor);

  case_seed = use_seed ? seed : std::random_device{}();
  std_rng.seed(case_seed);
  randompack_seed(case_seed, 0, 0, rp_rng);
  std_ns = time_int_range_std<int32_t>(std_rng, chunk, bench_time, 1, 100000);
  std_rng.seed(case_seed);
  randompack_seed(case_seed, 0, 0, rp_rng);
  rp_ns = time_int_range_rp(rp_rng, chunk, bench_time, 1, 100000);
  factor = std_ns/rp_ns;
  printf("%-18s %10.2f %11.2f %8.2f\n", "int 1-1e5", std_ns, rp_ns, factor);

  case_seed = use_seed ? seed : std::random_device{}();
  std_rng.seed(case_seed);
  randompack_seed(case_seed, 0, 0, rp_rng);
  std_ns = time_int_range_std<int32_t>(std_rng, chunk, bench_time, 1, 2000000000);
  std_rng.seed(case_seed);
  randompack_seed(case_seed, 0, 0, rp_rng);
  rp_ns = time_int_range_rp(rp_rng, chunk, bench_time, 1, 2000000000);
  factor = std_ns/rp_ns;
  printf("%-18s %10.2f %11.2f %8.2f\n", "int 1-2e9", std_ns, rp_ns, factor);

  case_seed = use_seed ? seed : std::random_device{}();
  std_rng.seed(case_seed);
  randompack_seed(case_seed, 0, 0, rp_rng);
  std_ns = time_long_long_range_std(std_rng, chunk, bench_time, 1LL, 2000000000LL);
  std_rng.seed(case_seed);
  randompack_seed(case_seed, 0, 0, rp_rng);
  rp_ns = time_long_long_range_rp(rp_rng, chunk, bench_time, 1LL, 2000000000LL);
  factor = std_ns/rp_ns;
  printf("%-18s %10.2f %11.2f %8.2f\n", "long long 1-2e9", std_ns, rp_ns, factor);

  case_seed = use_seed ? seed : std::random_device{}();
  std_rng.seed(case_seed);
  randompack_seed(case_seed, 0, 0, rp_rng);
  std_ns = time_long_long_range_std(std_rng, chunk, bench_time, 1LL, 6000000000000000000LL);
  std_rng.seed(case_seed);
  randompack_seed(case_seed, 0, 0, rp_rng);
  rp_ns = time_long_long_range_rp(rp_rng, chunk, bench_time, 1LL, 6000000000000000000LL);
  factor = std_ns/rp_ns;
  printf("%-18s %10.2f %11.2f %8.2f\n", "long long 1-6e18", std_ns, rp_ns, factor);

  case_seed = use_seed ? seed : std::random_device{}();
  std_rng.seed(case_seed);
  randompack_seed(case_seed, 0, 0, rp_rng);
  std_ns = time_perm_std(std_rng, bench_time, 100);
  std_rng.seed(case_seed);
  randompack_seed(case_seed, 0, 0, rp_rng);
  rp_ns = time_perm_rp(rp_rng, bench_time, 100);
  factor = std_ns/rp_ns;
  printf("%-18s %10.2f %11.2f %8.2f\n", "perm 100", std_ns, rp_ns, factor);

  case_seed = use_seed ? seed : std::random_device{}();
  std_rng.seed(case_seed);
  randompack_seed(case_seed, 0, 0, rp_rng);
  std_ns = time_perm_std(std_rng, bench_time, 100000);
  std_rng.seed(case_seed);
  randompack_seed(case_seed, 0, 0, rp_rng);
  rp_ns = time_perm_rp(rp_rng, bench_time, 100000);
  factor = std_ns/rp_ns;
  printf("%-18s %10.2f %11.2f %8.2f\n", "perm 100000", std_ns, rp_ns, factor);

  case_seed = use_seed ? seed : std::random_device{}();
  std_rng.seed(case_seed);
  randompack_seed(case_seed, 0, 0, rp_rng);
  std_ns = time_sample_std(std_rng, bench_time, 1000, 20);
  std_rng.seed(case_seed);
  randompack_seed(case_seed, 0, 0, rp_rng);
  rp_ns = time_sample_rp(rp_rng, bench_time, 1000, 20);
  factor = std_ns/rp_ns;
  printf("%-18s %10.2f %11.2f %8.2f\n", "sample 20/1000", std_ns, rp_ns, factor);

  case_seed = use_seed ? seed : std::random_device{}();
  std_rng.seed(case_seed);
  randompack_seed(case_seed, 0, 0, rp_rng);
  std_ns = time_sample_std(std_rng, bench_time, 1000, 500);
  std_rng.seed(case_seed);
  randompack_seed(case_seed, 0, 0, rp_rng);
  rp_ns = time_sample_rp(rp_rng, bench_time, 1000, 500);
  factor = std_ns/rp_ns;
  printf("%-18s %10.2f %11.2f %8.2f\n", "sample 500/1000", std_ns, rp_ns, factor);

  case_seed = use_seed ? seed : std::random_device{}();
  std_rng.seed(case_seed);
  randompack_seed(case_seed, 0, 0, rp_rng);
  std_ns = time_sample_std(std_rng, bench_time, 1000, 980);
  std_rng.seed(case_seed);
  randompack_seed(case_seed, 0, 0, rp_rng);
  rp_ns = time_sample_rp(rp_rng, bench_time, 1000, 980);
  factor = std_ns/rp_ns;
  printf("%-18s %10.2f %11.2f %8.2f\n", "sample 980/1000", std_ns, rp_ns, factor);

  randompack_free(rp_rng);
  return 0;
}
