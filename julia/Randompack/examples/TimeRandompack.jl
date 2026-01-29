# TimeRandompack.jl
# Time randompack distributions in Julia (ns/value)

using Randompack
using Printf

engine = length(ARGS) >= 1 ? ARGS[1] : ""
if isempty(engine)
  engine = "x256++simd"
end

rng = rng_create(engine)

chunk = 4096
bench_time = 0.2
reps = max(1, floor(Int, 1e6 / chunk))
buf = Vector{Float64}(undef, chunk)

@printf("Engine: %s\n", engine)
@printf("%-18s %8s\n", "Distribution", "ns/value")

# Warmup
let t0 = time()
  while (time() - t0) < 0.1
    random_unif!(rng, buf)
  end
  t1 = time()
  @printf("Warmup time: %.3f s\n\n", t1 - t0)
end

function time_dist(f, chunk, reps, bench_time)
  calls = 0
  t0 = time()
  while (time() - t0) < bench_time
    for _ in 1:reps
      x = f()
      sink = x[chunk]
    end
    calls += reps
  end
  t1 = time()
  return 1e9 * (t1 - t0) / (calls * chunk)
end

ns = time_dist(() -> (random_unif!(rng, buf); buf), chunk, reps, bench_time)
@printf("%-18s %8.2f\n", "u01", ns)

ns = time_dist(() -> (random_unif!(rng, buf; a=2, b=5); buf), chunk, reps, bench_time)
@printf("%-18s %8.2f\n", "unif(2,5)", ns)

ns = time_dist(() -> (random_normal!(rng, buf); buf), chunk, reps, bench_time)
@printf("%-18s %8.2f\n", "norm", ns)

ns = time_dist(() -> (random_normal!(rng, buf; mu=2, sigma=3); buf), chunk, reps, bench_time)
@printf("%-18s %8.2f\n", "normal(2,3)", ns)

ns = time_dist(() -> (random_exp!(rng, buf; scale=1); buf), chunk, reps, bench_time)
@printf("%-18s %8.2f\n", "exp(1)", ns)

ns = time_dist(() -> (random_exp!(rng, buf; scale=2); buf), chunk, reps, bench_time)
@printf("%-18s %8.2f\n", "exp(2)", ns)

ns = time_dist(() -> (random_lognormal!(rng, buf; mu=0, sigma=1); buf), chunk, reps, bench_time)
@printf("%-18s %8.2f\n", "lognormal(0,1)", ns)

ns = time_dist(() -> (random_gumbel!(rng, buf; mu=0, beta=1); buf), chunk, reps, bench_time)
@printf("%-18s %8.2f\n", "gumbel(0,1)", ns)

ns = time_dist(() -> (random_pareto!(rng, buf; xm=1, alpha=2); buf), chunk, reps, bench_time)
@printf("%-18s %8.2f\n", "pareto(1,2)", ns)

ns = time_dist(() -> (random_gamma!(rng, buf; shape=2, scale=3); buf), chunk, reps, bench_time)
@printf("%-18s %8.2f\n", "gamma(2,3)", ns)

ns = time_dist(() -> (random_chi2!(rng, buf; nu=5); buf), chunk, reps, bench_time)
@printf("%-18s %8.2f\n", "chi2(5)", ns)

ns = time_dist(() -> (random_beta!(rng, buf; a=2, b=5); buf), chunk, reps, bench_time)
@printf("%-18s %8.2f\n", "beta(2,5)", ns)

ns = time_dist(() -> (random_t!(rng, buf; nu=10); buf), chunk, reps, bench_time)
@printf("%-18s %8.2f\n", "t(10)", ns)

ns = time_dist(() -> (random_f!(rng, buf; nu1=5, nu2=10); buf), chunk, reps, bench_time)
@printf("%-18s %8.2f\n", "F(5,10)", ns)

ns = time_dist(() -> (random_weibull!(rng, buf; shape=2, scale=1); buf), chunk, reps, bench_time)
@printf("%-18s %8.2f\n", "weibull(2,1)", ns)
