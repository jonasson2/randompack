# TimeDqrng.R
# Time dqrng-supported distributions in R (ns/value)

set.seed(7)

if (!requireNamespace("dqrng", quietly = TRUE)) {
  stop("Package 'dqrng' is required. Install with install.packages('dqrng').")
}

chunk = 4096
bench_time = 0.2            # seconds per distribution
reps = max(1, floor(1e6 / chunk))

cat(sprintf("%-18s %8s\n", "Distribution", "ns/value"))

# Warmup (JIT/bytecode, first-call effects, CPU boosting)
t0 <- proc.time()[["elapsed"]]
for (i in 1:30000) dqrng::dqrunif(1000)
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
ns = time_dist(function() dqrng::dqrunif(chunk), chunk, reps, bench_time)
cat(sprintf("%-18s %8.2f\n", "u01", ns))

# unif(2,5)
ns = time_dist(function() 2 + 3*dqrng::dqrunif(chunk), chunk, reps, bench_time)
cat(sprintf("%-18s %8.2f\n", "unif(2,5)", ns))

# norm
ns = time_dist(function() dqrng::dqrnorm(chunk), chunk, reps, bench_time)
cat(sprintf("%-18s %8.2f\n", "norm", ns))

# normal(2,3)
ns = time_dist(function() 2 + 3*dqrng::dqrnorm(chunk), chunk, reps, bench_time)
cat(sprintf("%-18s %8.2f\n", "normal(2,3)", ns))

# exp(1)
ns = time_dist(function() dqrng::dqrexp(chunk, rate=1), chunk, reps, bench_time)
cat(sprintf("%-18s %8.2f\n", "exp(1)", ns))

# exp(2)  (scale=2 => rate=1/2)
ns = time_dist(function() dqrng::dqrexp(chunk, rate=1/2), chunk, reps, bench_time)
cat(sprintf("%-18s %8.2f\n", "exp(2)", ns))
