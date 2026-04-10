// TimeDistCpp.cpp
// Compare C++ standard library <random> vs randompack distributions (ns/value).
//
// Example build:
//   c++ -O3 -std=c++17 -I../src TimeDistCpp.cpp
//   -L../release/src -lrandompack -Wl,-rpath,../release/src -o TimeDistCpp

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <chrono>
#include <random>

extern "C" {
#include "randompack.h"
}

static double now_sec()
{
  using clock = std::chrono::steady_clock;
  return std::chrono::duration<double>(clock::now().time_since_epoch()).count();
}

static void consume(double x)
{
  static volatile double sink = 0;
  sink += x;
}

static int compute_reps(int chunk)
{
  int reps = 1000000/chunk;
  return reps > 0 ? reps : 1;
}

static double warmup_cpu(double seconds)
{
  double t0 = now_sec();
  double t = t0;
  volatile double x = 0;
  while (t - t0 < seconds) {
    for (int i = 0; i < 1024; i++) {
      x += i;
    }
    t = now_sec();
  }
  if (x < 0) {
    std::printf("warmup\n");
  }
  return t - t0;
}

template <class F>
static double time_dist(int chunk, double bench_time, int reps, F &&fill)
{
  int calls = 0;
  double t0 = now_sec();
  double t = t0;
  while (t - t0 < bench_time) {
    for (int i = 0; i < reps; i++) {
      fill();
      calls++;
    }
    t = now_sec();
  }
  int total = calls*chunk;
  if (total <= 0) {
    return 0;
  }
  return 1e9*(t - t0)/total;
}

static void print_help()
{
  std::printf("TimeDistCpp — compare C++ standard library <random> vs ");
  std::printf("randompack (ns/value)\n");
  std::printf("Usage: TimeDistCpp [options]\n\n");
  std::printf("Options:\n");
  std::printf("  -h            Show this help message\n");
  std::printf("  -e engine     RNG engine (default x256++simd)\n");
  std::printf("  -t seconds    Benchmark time per distribution (default 0.2)\n");
  std::printf("  -c chunk      Chunk size (default 4096)\n");
  std::printf("  -s seed       Fixed RNG seed (default random seed per case)\n");
  std::printf("  -b            Use bitexact log/exp implementations\n");
}

int main(int argc, char **argv)
{
  char *engine = (char *)"x256++simd";
  int chunk = 4096;
  double bench_time = 0.2;
  int seed = 0;
  bool have_seed = false;
  bool bitexact = false;
  for (int i = 1; i < argc; i++) {
    if (!std::strcmp(argv[i], "-h")) {
      print_help();
      return 0;
    }
    if (!std::strcmp(argv[i], "-e") && i + 1 < argc) {
      engine = argv[++i];
      continue;
    }
    if (!std::strcmp(argv[i], "-t") && i + 1 < argc) {
      bench_time = std::atof(argv[++i]);
      continue;
    }
    if (!std::strcmp(argv[i], "-c") && i + 1 < argc) {
      chunk = std::atoi(argv[++i]);
      continue;
    }
    if (!std::strcmp(argv[i], "-s") && i + 1 < argc) {
      seed = std::atoi(argv[++i]);
      have_seed = true;
      continue;
    }
    if (!std::strcmp(argv[i], "-b")) {
      bitexact = true;
      continue;
    }
    std::fprintf(stderr, "Unknown/invalid option: %s\n", argv[i]);
    return 1;
  }
  if (chunk <= 0 || bench_time <= 0) {
    std::fprintf(stderr, "Invalid -c or -t\n");
    return 1;
  }

  randompack_rng *rng = randompack_create(engine);
  if (!rng) {
    std::fprintf(stderr, "randompack_create failed: %s\n", engine);
    return 1;
  }
  if (bitexact && !randompack_bitexact(rng, true)) {
    std::fprintf(stderr, "randompack_bitexact failed\n");
    randompack_free(rng);
    return 1;
  }
  int reps = compute_reps(chunk);
  std::random_device rd;
  std::mt19937_64 cpp_rng;
  auto u01 = [&]() {
    return std::generate_canonical<double, 64>(cpp_rng);
  };

  double warm = warmup_cpu(0.1);
  double *x = new double[chunk];

  std::uniform_real_distribution<double> unif_2_5(2, 5);
  std::normal_distribution<double> norm_0_1(0, 1);
  std::normal_distribution<double> normal_2_3(2, 3);
  std::lognormal_distribution<double> logn_0_1(0, 1);
  std::exponential_distribution<double> exp_1(1);
  std::exponential_distribution<double> exp_2(0.5);
  std::normal_distribution<double> skew_z0(0, 1);
  std::normal_distribution<double> skew_z1(0, 1);
  std::gamma_distribution<double> gamma_2_3(2, 3);
  std::gamma_distribution<double> gamma_0_5_2(0.5, 2);
  std::gamma_distribution<double> chi2_5(2.5, 2);
  std::weibull_distribution<double> weibull_2_3(2, 3);

  std::printf("C++ RNG library:   standard library <random>\n");
  std::printf("C++ engine:        mt19937_64 (Mersenne Twister)\n");
  std::printf("Randompack engine: x256++simd (xoshiro256++, SIMD accelerated)\n");
  std::printf("Chunk size:        %d\n", chunk);
  std::printf("Engine key:        %s\n", engine);
  std::printf("Warmup:            %.3f s\n", warm);
  std::printf("\n");
  std::printf("%-18s %10s %11s %8s\n", "Distribution", "C++",
              "Randompack", "Factor");

  auto run = [&](const char *name, auto &&fill_cpp, auto &&fill_rp) {
    int case_seed = have_seed ? seed : (int)(rd() & 0x7fffffff);
    cpp_rng.seed((uint64_t)case_seed);
    if (!randompack_seed(case_seed, 0, 0, rng)) {
      std::fprintf(stderr, "randompack_seed failed\n");
      randompack_free(rng);
      std::exit(1);
    }
    double cpp_ns = time_dist(chunk, bench_time, reps, fill_cpp);
    cpp_rng.seed((uint64_t)case_seed);
    if (!randompack_seed(case_seed, 0, 0, rng)) {
      std::fprintf(stderr, "randompack_seed failed\n");
      randompack_free(rng);
      std::exit(1);
    }
    double rp_ns = time_dist(chunk, bench_time, reps, fill_rp);
    double factor = cpp_ns/rp_ns;
    std::printf("%-18s %10.2f %11.2f %8.2f\n", name, cpp_ns, rp_ns, factor);
  };

  run("u01", [&]() {
    for (int i = 0; i < chunk; i++) {
      x[i] = u01();
    }
    consume(x[chunk - 1]);
  }, [&]() {
    randompack_u01(x, (size_t)chunk, rng);
    consume(x[chunk - 1]);
  });

  run("unif(2,5)", [&]() {
    for (int i = 0; i < chunk; i++) {
      x[i] = unif_2_5(cpp_rng);
    }
    consume(x[chunk - 1]);
  }, [&]() {
    randompack_unif(x, (size_t)chunk, 2, 5, rng);
    consume(x[chunk - 1]);
  });

  run("norm", [&]() {
    for (int i = 0; i < chunk; i++) {
      x[i] = norm_0_1(cpp_rng);
    }
    consume(x[chunk - 1]);
  }, [&]() {
    randompack_norm(x, (size_t)chunk, rng);
    consume(x[chunk - 1]);
  });

  run("normal(2,3)", [&]() {
    for (int i = 0; i < chunk; i++) {
      x[i] = normal_2_3(cpp_rng);
    }
    consume(x[chunk - 1]);
  }, [&]() {
    randompack_normal(x, (size_t)chunk, 2, 3, rng);
    consume(x[chunk - 1]);
  });

  run("exp(1)", [&]() {
    for (int i = 0; i < chunk; i++) {
      x[i] = exp_1(cpp_rng);
    }
    consume(x[chunk - 1]);
  }, [&]() {
    randompack_exp(x, (size_t)chunk, 1, rng);
    consume(x[chunk - 1]);
  });

  run("exp(2)", [&]() {
    for (int i = 0; i < chunk; i++) {
      x[i] = exp_2(cpp_rng);
    }
    consume(x[chunk - 1]);
  }, [&]() {
    randompack_exp(x, (size_t)chunk, 2, rng);
    consume(x[chunk - 1]);
  });

  run("lognormal(0,1)", [&]() {
    for (int i = 0; i < chunk; i++) {
      x[i] = logn_0_1(cpp_rng);
    }
    consume(x[chunk - 1]);
  }, [&]() {
    randompack_lognormal(x, (size_t)chunk, 0, 1, rng);
    consume(x[chunk - 1]);
  });

  run("skew-normal(0,1,5)", [&]() {
    double delta = 5/std::sqrt(1 + 5*5);
    double omega = std::sqrt(1 - delta*delta);
    for (int i = 0; i < chunk; i++) {
      double z0 = skew_z0(cpp_rng);
      double z1 = skew_z1(cpp_rng);
      x[i] = delta*std::abs(z0) + omega*z1;
    }
    consume(x[chunk - 1]);
  }, [&]() {
    randompack_skew_normal(x, (size_t)chunk, 0, 1, 5, rng);
    consume(x[chunk - 1]);
  });

  run("gumbel(0,1)", [&]() {
    for (int i = 0; i < chunk; i++) {
      double u = u01();
      while (u <= 0) {
        u = u01();
      }
      x[i] = -std::log(-std::log(u));
    }
    consume(x[chunk - 1]);
  }, [&]() {
    randompack_gumbel(x, (size_t)chunk, 0, 1, rng);
    consume(x[chunk - 1]);
  });

  run("pareto(1,2)", [&]() {
    for (int i = 0; i < chunk; i++) {
      double u = u01();
      while (u <= 0) {
        u = u01();
      }
      x[i] = std::pow(1 - u, -0.5);
    }
    consume(x[chunk - 1]);
  }, [&]() {
    randompack_pareto(x, (size_t)chunk, 1, 2, rng);
    consume(x[chunk - 1]);
  });

  run("gamma(2,3)", [&]() {
    for (int i = 0; i < chunk; i++) {
      x[i] = gamma_2_3(cpp_rng);
    }
    consume(x[chunk - 1]);
  }, [&]() {
    randompack_gamma(x, (size_t)chunk, 2, 3, rng);
    consume(x[chunk - 1]);
  });

  run("gamma(0.5,2)", [&]() {
    for (int i = 0; i < chunk; i++) {
      x[i] = gamma_0_5_2(cpp_rng);
    }
    consume(x[chunk - 1]);
  }, [&]() {
    randompack_gamma(x, (size_t)chunk, 0.5, 2, rng);
    consume(x[chunk - 1]);
  });

  run("beta(2,5)", [&]() {
    std::gamma_distribution<double> g1(2, 1);
    std::gamma_distribution<double> g2(5, 1);
    for (int i = 0; i < chunk; i++) {
      double a = g1(cpp_rng);
      double b = g2(cpp_rng);
      x[i] = a/(a + b);
    }
    consume(x[chunk - 1]);
  }, [&]() {
    randompack_beta(x, (size_t)chunk, 2, 5, rng);
    consume(x[chunk - 1]);
  });

  run("chi2(5)", [&]() {
    for (int i = 0; i < chunk; i++) {
      x[i] = chi2_5(cpp_rng);
    }
    consume(x[chunk - 1]);
  }, [&]() {
    randompack_chi2(x, (size_t)chunk, 5, rng);
    consume(x[chunk - 1]);
  });

  run("t(10)", [&]() {
    std::normal_distribution<double> z01(0, 1);
    std::gamma_distribution<double> chi2_10(5, 2);
    for (int i = 0; i < chunk; i++) {
      double z = z01(cpp_rng);
      double v = chi2_10(cpp_rng);
      x[i] = z/std::sqrt(v/10);
    }
    consume(x[chunk - 1]);
  }, [&]() {
    randompack_t(x, (size_t)chunk, 10, rng);
    consume(x[chunk - 1]);
  });

  run("F(5,10)", [&]() {
    std::gamma_distribution<double> chi2_5_(2.5, 2);
    std::gamma_distribution<double> chi2_10_(5, 2);
    for (int i = 0; i < chunk; i++) {
      double v1 = chi2_5_(cpp_rng);
      double v2 = chi2_10_(cpp_rng);
      x[i] = (v1/5)/(v2/10);
    }
    consume(x[chunk - 1]);
  }, [&]() {
    randompack_f(x, (size_t)chunk, 5, 10, rng);
    consume(x[chunk - 1]);
  });

  run("weibull(2,3)", [&]() {
    for (int i = 0; i < chunk; i++) {
      x[i] = weibull_2_3(cpp_rng);
    }
    consume(x[chunk - 1]);
  }, [&]() {
    randompack_weibull(x, (size_t)chunk, 2, 3, rng);
    consume(x[chunk - 1]);
  });

  delete[] x;
  randompack_free(rng);
  return 0;
}
