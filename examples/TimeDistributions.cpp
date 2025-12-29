// TimeDistributions.cpp: time continuous distributions in C++ (ns/value).
// Build: g++ -O3 -std=c++17 TimeDistributions.cpp -o TimeDistributions

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <chrono>
#include <random>
#include <string>

static inline double now_sec() {
  using clock = std::chrono::steady_clock;
  return std::chrono::duration<double>(clock::now().time_since_epoch()).count();
}

static inline void consume_double(double x) {
  static volatile double sink = 0;
  sink += x;
}

static int compute_reps(int chunk) {
  int reps = 1000000 / chunk;
  return reps > 0 ? reps : 1;
}

template <class F>
static double time_dist(int chunk, double bench_time, int reps, F &&fill) {
  int calls = 0;
  double t0 = now_sec();
  double t = t0;
  while (t - t0 < bench_time) {
    for (int i = 0; i < reps; i++) {
      fill();
    }
    calls += reps;
    t = now_sec();
  }
  int total = calls * chunk;
  if (total <= 0)
    return 0;
  return 1e9 * (t - t0) / total;
}

static void print_help() {
  std::printf("TimeDistributions — time continuous distributions (ns/value)\n");
  std::printf("Usage: TimeDistributions [options]\n\n");
  std::printf("Options:\n");
  std::printf("  -h            Show this help message\n");
  std::printf("  -t seconds    Benchmark time per distribution (default 0.2)\n");
  std::printf("  -c chunk      Chunk size (default 1024)\n");
  std::printf("  -s seed       RNG seed (default 7)\n");
}

int main(int argc, char **argv) {
  int chunk = 1024;
  double bench_time = 0.2;
  uint64_t seed = 7;

  for (int i = 1; i < argc; i++) {
    if (!std::strcmp(argv[i], "-h")) {
      print_help();
      return 0;
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
      seed = (uint64_t)std::strtoull(argv[++i], 0, 10);
      continue;
    }
    std::fprintf(stderr, "Unknown/invalid option: %s\n", argv[i]);
    return 1;
  }

  if (chunk <= 0 || bench_time <= 0) {
    std::fprintf(stderr, "Invalid -c or -t\n");
    return 1;
  }

  int reps = compute_reps(chunk);
  std::mt19937_64 rng(seed);

  // Uniform(0,1): use generate_canonical to get a double in [0,1).
  auto u01 = [&]() {
    return std::generate_canonical<double, 64>(rng);
  };

  // Warmup (JIT doesn't exist, but warm caches / branch predictors / libm paths).
  for (int i = 0; i < 10000; i++) {
    double u = u01();
    double z = std::normal_distribution<double>(0, 1)(rng);
    consume_double(u + z);
  }

  std::printf("Distribution       ns/value\n");

  // Buffers reused across all timings (avoid alloc noise).
  double *x = new double[chunk];

  // Distributions we can use directly from <random>.
  std::uniform_real_distribution<double> unif_2_5(2, 5);
  std::normal_distribution<double> norm_0_1(0, 1);
  std::lognormal_distribution<double> logn_0_1(0, 1);
  std::exponential_distribution<double> exp_1(1);     // lambda = 1/scale
  std::exponential_distribution<double> exp_2(0.5);   // scale=2 => lambda=0.5
  std::gamma_distribution<double> gamma_2_3(2, 3);    // shape=2, scale=3
  std::gamma_distribution<double> chi2_5(2.5, 2);     // chi2(5) = Gamma(5/2, 2)
  std::weibull_distribution<double> weibull_2_1(2, 1);// shape=2, scale=1

  auto run = [&](const char *name, auto &&fill) {
    double ns = time_dist(chunk, bench_time, reps, fill);
    std::printf("%-18s %8.2f\n", name, ns);
  };

  run("u01", [&]() {
    for (int i = 0; i < chunk; i++)
      x[i] = u01();
    consume_double(x[chunk - 1]);
  });

  run("unif(2,5)", [&]() {
    for (int i = 0; i < chunk; i++)
      x[i] = unif_2_5(rng);
    consume_double(x[chunk - 1]);
  });

  run("norm", [&]() {
    for (int i = 0; i < chunk; i++)
      x[i] = norm_0_1(rng);
    consume_double(x[chunk - 1]);
  });

  run("normal(2,3)", [&]() {
    // mean=2, sd=3
    std::normal_distribution<double> n23(2, 3);
    for (int i = 0; i < chunk; i++)
      x[i] = n23(rng);
    consume_double(x[chunk - 1]);
  });

  run("lognormal(0,1)", [&]() {
    for (int i = 0; i < chunk; i++)
      x[i] = logn_0_1(rng);
    consume_double(x[chunk - 1]);
  });

  run("gumbel(0,1)", [&]() {
    // Gumbel(mu=0,beta=1): X = -log(-log(U))
    for (int i = 0; i < chunk; i++) {
      double u = u01();
      // u is in [0,1); avoid log(0) if u==0
      while (u <= 0)
        u = u01();
      x[i] = -std::log(-std::log(u));
    }
    consume_double(x[chunk - 1]);
  });

  run("pareto(1,2)", [&]() {
    // Pareto Type I, xm=1, alpha=2: X=(1-U)^(-1/alpha)
    for (int i = 0; i < chunk; i++) {
      double u = u01();
      // avoid 1-U == 0 (if u==1), but u is < 1; still avoid u==0 for stability
      while (u <= 0)
        u = u01();
      x[i] = std::pow(1 - u, -0.5);
    }
    consume_double(x[chunk - 1]);
  });

  run("exp(1)", [&]() {
    for (int i = 0; i < chunk; i++)
      x[i] = exp_1(rng);
    consume_double(x[chunk - 1]);
  });

  run("exp(2)", [&]() {
    for (int i = 0; i < chunk; i++)
      x[i] = exp_2(rng);
    consume_double(x[chunk - 1]);
  });

  run("gamma(2,3)", [&]() {
    for (int i = 0; i < chunk; i++)
      x[i] = gamma_2_3(rng);
    consume_double(x[chunk - 1]);
  });

  run("chi2(5)", [&]() {
    for (int i = 0; i < chunk; i++)
      x[i] = chi2_5(rng);
    consume_double(x[chunk - 1]);
  });

  run("beta(2,5)", [&]() {
    // Beta(a,b) via gamma ratio: G1~Gamma(a,1), G2~Gamma(b,1), X=G1/(G1+G2)
    std::gamma_distribution<double> g1(2, 1);
    std::gamma_distribution<double> g2(5, 1);
    for (int i = 0; i < chunk; i++) {
      double a = g1(rng);
      double b = g2(rng);
      x[i] = a / (a + b);
    }
    consume_double(x[chunk - 1]);
  });

  run("t(10)", [&]() {
    // t_nu = Z / sqrt(V/nu), Z~N(0,1), V~Chi2(nu)
    std::normal_distribution<double> z01(0, 1);
    std::gamma_distribution<double> chi2_10(5, 2); // chi2(10)=Gamma(10/2,2)
    for (int i = 0; i < chunk; i++) {
      double z = z01(rng);
      double v = chi2_10(rng);
      x[i] = z / std::sqrt(v / 10);
    }
    consume_double(x[chunk - 1]);
  });

  run("F(5,10)", [&]() {
    // F = (V1/nu1)/(V2/nu2), V1~Chi2(nu1), V2~Chi2(nu2)
    std::gamma_distribution<double> chi2_5_(2.5, 2);
    std::gamma_distribution<double> chi2_10_(5, 2);
    for (int i = 0; i < chunk; i++) {
      double v1 = chi2_5_(rng);
      double v2 = chi2_10_(rng);
      x[i] = (v1 / 5) / (v2 / 10);
    }
    consume_double(x[chunk - 1]);
  });

  run("weibull(2,1)", [&]() {
    for (int i = 0; i < chunk; i++)
      x[i] = weibull_2_1(rng);
    consume_double(x[chunk - 1]);
  });

  delete[] x;
  return 0;
}
