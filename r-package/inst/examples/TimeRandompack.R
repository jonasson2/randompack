# TimeRandompack.R
# Time randompack distributions in R (ns/value)

set.seed(7)

if (!requireNamespace("randompack", quietly = TRUE)) {
  stop("Package 'randompack' is required. Install with install.packages('randompack') or from your repo.")
}

# Optional engine from command line: Rscript TimeRandompack.R x256++simd
args <- commandArgs(trailingOnly = TRUE)
engine <- if (length(args) >= 1L) args[[1]] else ""
if (!nzchar(engine)) engine <- "x256++simd"

rng <- randompack::randompack_rng(engine=engine)

chunk = 4096
bench_time = 0.2            # seconds per distribution
reps = max(1, floor(1e6 / chunk))

cat(sprintf("Engine: %s\n", if (nzchar(engine)) engine else "<default>"))
cat(sprintf("%-18s %8s\n", "Distribution", "ns/value"))

# Warmup (JIT/bytecode, first-call effects, CPU boosting)
t0 <- proc.time()[["elapsed"]]
for (i in 1:30000) rng$unif(1000)
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
ns = time_dist(function() rng$unif(chunk), chunk, reps, bench_time)
cat(sprintf("%-18s %8.2f\n", "u01", ns))

# unif(2,5)
ns = time_dist(function() rng$unif(chunk, 2, 5), chunk, reps, bench_time)
cat(sprintf("%-18s %8.2f\n", "unif(2,5)", ns))

# norm
ns = time_dist(function() rng$normal(chunk), chunk, reps, bench_time)
cat(sprintf("%-18s %8.2f\n", "norm", ns))

# normal(2,3)
ns = time_dist(function() rng$normal(chunk, 2, 3), chunk, reps, bench_time)
cat(sprintf("%-18s %8.2f\n", "normal(2,3)", ns))

# lognormal(0,1)
ns = time_dist(function() rng$lognormal(chunk, 0, 1), chunk, reps, bench_time)
cat(sprintf("%-18s %8.2f\n", "lognormal(0,1)", ns))

# gumbel(0,1)
ns = time_dist(function() rng$gumbel(chunk, 0, 1), chunk, reps, bench_time)
cat(sprintf("%-18s %8.2f\n", "gumbel(0,1)", ns))

# pareto(1,2)
ns = time_dist(function() rng$pareto(chunk, 1, 2), chunk, reps, bench_time)
cat(sprintf("%-18s %8.2f\n", "pareto(1,2)", ns))

# exp(1)
ns = time_dist(function() rng$exp(chunk, 1), chunk, reps, bench_time)
cat(sprintf("%-18s %8.2f\n", "exp(1)", ns))

# exp(2)
ns = time_dist(function() rng$exp(chunk, 2), chunk, reps, bench_time)
cat(sprintf("%-18s %8.2f\n", "exp(2)", ns))

# gamma(2,3)
ns = time_dist(function() rng$gamma(chunk, 2, 3), chunk, reps, bench_time)
cat(sprintf("%-18s %8.2f\n", "gamma(2,3)", ns))

# chi2(5)
ns = time_dist(function() rng$chi2(chunk, 5), chunk, reps, bench_time)
cat(sprintf("%-18s %8.2f\n", "chi2(5)", ns))

# beta(2,5)
ns = time_dist(function() rng$beta(chunk, 2, 5), chunk, reps, bench_time)
cat(sprintf("%-18s %8.2f\n", "beta(2,5)", ns))

# t(10)
ns = time_dist(function() rng$t(chunk, 10), chunk, reps, bench_time)
cat(sprintf("%-18s %8.2f\n", "t(10)", ns))

# F(5,10)
ns = time_dist(function() rng$f(chunk, 5, 10), chunk, reps, bench_time)
cat(sprintf("%-18s %8.2f\n", "F(5,10)", ns))

# weibull(2,1)
ns = time_dist(function() rng$weibull(chunk, 2, 1), chunk, reps, bench_time)
cat(sprintf("%-18s %8.2f\n", "weibull(2,1)", ns))
