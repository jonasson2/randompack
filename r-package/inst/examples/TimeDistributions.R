# TimeDistributions.R
# Time continuous distributions in R (ns/value)

# RNGkind(kind="L'Ecuyer-CMRG", normal.kind="Inversion")
set.seed(7)

chunk = 1024
bench_time = 0.2            # seconds per distribution
reps = max(1, floor(1e6 / chunk))

cat(sprintf("%-18s %8s\n", "Distribution", "ns/value"))

# Warmup (JIT/bytecode, first-call effects, CPU boosting)
t0 <- proc.time()[["elapsed"]]
for (i in 1:30000) runif(1000)
t1 <- proc.time()[["elapsed"]]
cat(sprintf("Warmup time: %.3f s\n\n", t1 - t0))

time_dist = function(f, chunk, reps, bench_time) {
  calls = 0
  t0 = proc.time()[["elapsed"]]
  while ((proc.time()[["elapsed"]] - t0) < bench_time) {
    for (i in 1:reps) {
      x = f()
      sink = x[chunk]       # force use
    }
    calls = calls + reps
  }
  t1 = proc.time()[["elapsed"]]
  1e9 * (t1 - t0) / (calls * chunk)
}

# u01
ns = time_dist(function() runif(chunk), chunk, reps, bench_time)
cat(sprintf("%-18s %8.2f\n", "u01", ns))

# unif(2,5)
ns = time_dist(function() 2 + 3*runif(chunk), chunk, reps, bench_time)
cat(sprintf("%-18s %8.2f\n", "unif(2,5)", ns))

# norm
ns = time_dist(function() rnorm(chunk), chunk, reps, bench_time)
cat(sprintf("%-18s %8.2f\n", "norm", ns))

# normal(2,3)
ns = time_dist(function() 2 + 3*rnorm(chunk), chunk, reps, bench_time)
cat(sprintf("%-18s %8.2f\n", "normal(2,3)", ns))

# lognormal(0,1)
ns = time_dist(function() rlnorm(chunk, meanlog=0, sdlog=1), chunk, reps, bench_time)
cat(sprintf("%-18s %8.2f\n", "lognormal(0,1)", ns))

# exp(1)
ns = time_dist(function() rexp(chunk, rate=1), chunk, reps, bench_time)
cat(sprintf("%-18s %8.2f\n", "exp(1)", ns))

# exp(2)  (scale=2 => rate=1/2)
ns = time_dist(function() rexp(chunk, rate=1/2), chunk, reps, bench_time)
cat(sprintf("%-18s %8.2f\n", "exp(2)", ns))

# gamma(2,3)
ns = time_dist(function() rgamma(chunk, shape=2, scale=3), chunk, reps, bench_time)
cat(sprintf("%-18s %8.2f\n", "gamma(2,3)", ns))

# chi2(5)
ns = time_dist(function() rchisq(chunk, df=5), chunk, reps, bench_time)
cat(sprintf("%-18s %8.2f\n", "chi2(5)", ns))

# beta(2,5)
ns = time_dist(function() rbeta(chunk, shape1=2, shape2=5), chunk, reps, bench_time)
cat(sprintf("%-18s %8.2f\n", "beta(2,5)", ns))

# t(10)
ns = time_dist(function() rt(chunk, df=10), chunk, reps, bench_time)
cat(sprintf("%-18s %8.2f\n", "t(10)", ns))

# F(5,10)
ns = time_dist(function() rf(chunk, df1=5, df2=10), chunk, reps, bench_time)
cat(sprintf("%-18s %8.2f\n", "F(5,10)", ns))

# weibull(2,1)  (shape=2, scale=1)
ns = time_dist(function() rweibull(chunk, shape=2, scale=1), chunk, reps, bench_time)
cat(sprintf("%-18s %8.2f\n", "weibull(2,1)", ns))
