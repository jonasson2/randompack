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
bench_time = 0.4            # seconds per distribution
reps = max(1, floor(1e6 / chunk))

cat(sprintf("Engine: %s\n", if (nzchar(engine)) engine else "<default>"))
cat(sprintf("%-18s %8s\n", "Distribution", "ns/value"))

warmup = function(seconds) {
  t0 <- proc.time()[["elapsed"]]
  while ((proc.time()[["elapsed"]] - t0) < seconds) {
    rng$unif(1000)
  }
  proc.time()[["elapsed"]] - t0
}

cat(sprintf("Platform:  %s\n", R.version$platform))
cat(sprintf("Engine:    %s\n", if (nzchar(engine)) engine else "<default>"))
cat(sprintf("Time/case: %.3f s\n", bench_time))
warm <- warmup(0.1)
cat(sprintf("Warmup:    %.3f s\n\n", warm))

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

run_case = function(name, f_rp) {
  rp_ns = time_dist(f_rp, chunk, reps, bench_time)
  cat(sprintf("%-18s %8.2f\n", name, rp_ns))
}

run_case("u01",
         function() rng$unif(chunk))

run_case("unif(2,5)",
         function() rng$unif(chunk, 2, 5))

run_case("norm",
         function() rng$normal(chunk))

run_case("normal(2,3)",
         function() rng$normal(chunk, 2, 3))

run_case("exp(1)",
         function() rng$exp(chunk, 1))

run_case("exp(2)",
         function() rng$exp(chunk, 2))

run_case("lognormal(0,1)",
         function() rng$lognormal(chunk, 0, 1))

run_case("gamma(2,3)",
         function() rng$gamma(chunk, 2, 3))

run_case("chi2(5)",
         function() rng$chi2(chunk, 5))

run_case("beta(2,5)",
         function() rng$beta(chunk, 2, 5))

run_case("t(10)",
         function() rng$t(chunk, 10))

run_case("F(5,10)",
         function() rng$f(chunk, 5, 10))

run_case("weibull(2,1)",
         function() rng$weibull(chunk, 2, 1))
