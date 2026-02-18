# TimeDist.R
# Compare base R, dqrng, and randompack distributions (ns/value)

set.seed(7)

if (!requireNamespace("randompack", quietly = TRUE)) {
  stop("Package 'randompack' is required. Install it before running.")
}
if (!requireNamespace("dqrng", quietly = TRUE)) {
  stop("Package 'dqrng' is required. Install it before running.")
}

args <- commandArgs(trailingOnly = TRUE)
bitexact <- any(args == "-b")
args <- args[args != "-b"]
engine <- if (length(args) >= 1L) args[[1]] else ""
if (!nzchar(engine)) engine <- "x256++simd"

rng <- randompack::randompack_rng(engine=engine, bitexact=bitexact)

chunk = 4096
bench_time = 0.2
reps = max(1, floor(1e6 / chunk))

cat(sprintf("Platform:  %s\n", R.version$platform))
cat(sprintf("Engine:    %s\n", engine))
cat(sprintf("Time/case: %.3f s\n", bench_time))

time_dist = function(f, chunk, reps, bench_time) {
  calls = 0
  t0 = proc.time()[["elapsed"]]
  while ((proc.time()[["elapsed"]] - t0) < bench_time) {
    for (i in 1:reps) {
      x = f()
      sink = x[chunk]
    }
    calls = calls + reps
  }
  t1 = proc.time()[["elapsed"]]
  1e9 * (t1 - t0) / (calls * chunk)
}

warmup = function(seconds) {
  t0 <- proc.time()[["elapsed"]]
  while ((proc.time()[["elapsed"]] - t0) < seconds) {
    rng$unif(1000)
    runif(1000)
  }
  proc.time()[["elapsed"]] - t0
}

warm = warmup(0.1)
cat(sprintf("Warmup:    %.3f s\n\n", warm))

cat(sprintf("%-14s %10s %10s %11s %8s %8s\n",
            "DISTRIBUTION", "BASE-R", "DQRNG", "RANDOMPACK", "FACTOR-B",
            "FACTOR-D"))

old_kind <- RNGkind()
restore_rng <- function() {
  RNGkind(kind=old_kind[1], normal.kind=old_kind[2], sample.kind=old_kind[3])
  set.seed(7)
}

run_case = function(name, f_base, f_rp, f_dqrng=NULL, use_dqset=FALSE) {
  restore_rng()
  rp_ns = time_dist(f_rp, chunk, reps, bench_time)
  restore_rng()
  base_ns = time_dist(f_base, chunk, reps, bench_time)
  factor_b = base_ns / rp_ns
  if (is.null(f_dqrng)) {
    dqrng_str = sprintf("%10s", "")
    factor_d_str = sprintf("%8s", "")
  } else {
    if (use_dqset) dqrng::dqset.seed(123)
    dqrng_ns = time_dist(f_dqrng, chunk, reps, bench_time)
    dqrng_str = sprintf("%10.2f", dqrng_ns)
    factor_d_str = sprintf("%8.2f", dqrng_ns / rp_ns)
  }
  cat(sprintf("%-14s %10.2f %s %11.2f %8.2f %s\n",
              name, base_ns, dqrng_str, rp_ns, factor_b, factor_d_str))
}

run_case("u01",
         function() runif(chunk),
         function() rng$unif(chunk),
         function() dqrng::dqrunif(chunk),
         use_dqset=TRUE)

run_case("unif(2,5)",
         function() runif(chunk, min=2, max=5),
         function() rng$unif(chunk, 2, 5),
         function() dqrng::dqrunif(chunk, min=2, max=5),
         use_dqset=TRUE)

run_case("norm",
         function() rnorm(chunk),
         function() rng$normal(chunk),
         function() dqrng::dqrnorm(chunk),
         use_dqset=TRUE)

run_case("normal(2,3)",
         function() rnorm(chunk, mean=2, sd=3),
         function() rng$normal(chunk, 2, 3),
         function() dqrng::dqrnorm(chunk, mean=2, sd=3),
         use_dqset=TRUE)

run_case("exp(1)",
         function() rexp(chunk, rate=1),
         function() rng$exp(chunk, 1))

run_case("exp(2)",
         function() rexp(chunk, rate=1/2),
         function() rng$exp(chunk, 2))

run_case("lognormal(0,1)",
         function() rlnorm(chunk, meanlog=0, sdlog=1),
         function() rng$lognormal(chunk, 0, 1))

run_case("gamma(2,3)",
         function() rgamma(chunk, shape=2, scale=3),
         function() rng$gamma(chunk, 2, 3))

run_case("chi2(5)",
         function() rchisq(chunk, df=5),
         function() rng$chi2(chunk, 5))

run_case("beta(2,5)",
         function() rbeta(chunk, shape1=2, shape2=5),
         function() rng$beta(chunk, 2, 5))

run_case("t(10)",
         function() rt(chunk, df=10),
         function() rng$t(chunk, 10))

run_case("F(5,10)",
         function() rf(chunk, df1=5, df2=10),
         function() rng$f(chunk, 5, 10))

run_case("weibull(2,1)",
         function() rweibull(chunk, shape=2, scale=1),
         function() rng$weibull(chunk, 2, 1))
