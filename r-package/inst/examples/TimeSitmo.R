# TimeDistributions_sitmo.R

library(sitmo)

seed <- 7L
chunk <- 1024L
bench_time <- 0.2
reps <- max(1L, as.integer(floor(1e6 / chunk)))

cat(sprintf("%-18s %8s\n", "Distribution", "ns/value"))

time_dist <- function(f, chunk, reps, bench_time) {
  calls <- 0L
  t0 <- proc.time()[["elapsed"]]
  while ((proc.time()[["elapsed"]] - t0) < bench_time) {
    for (i in 1:reps) {
      x <- f()
      sink <- x[chunk]  # force use
    }
    calls <- calls + reps
  }
  t1 <- proc.time()[["elapsed"]]
  1e9 * (t1 - t0) / (calls * chunk)
}

u01 <- function() runif_sitmo(chunk, seed=seed)

# Warmup (JIT/bytecode, first-call effects, CPU boosting)
t0 <- proc.time()[["elapsed"]]
for (i in 1:30000) runif(1000)
t1 <- proc.time()[["elapsed"]]
cat(sprintf("Warmup time: %.3f s\n\n", t1 - t0))

# u01
ns <- time_dist(function() u01(), chunk, reps, bench_time)
cat(sprintf("%-18s %8.2f\n", "u01", ns))
