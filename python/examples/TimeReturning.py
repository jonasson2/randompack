#!/usr/bin/env python3
# TimeReturning.py
# Time randompack distributions (returning arrays) in Python (ns/value)

import argparse
import os
import sys
import time

import numpy as np

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
if ROOT not in sys.path:
  sys.path.insert(0, ROOT)

import randompack as rp


def compute_reps(chunk: int) -> int:
  return max(1, 1_000_000 // chunk)


def time_dist(fn, chunk: int, reps: int, bench_time: float) -> float:
  calls = 0
  t0 = time.perf_counter()
  t = t0
  while t - t0 < bench_time:
    for _ in range(reps):
      x = fn()
      sink = x[chunk - 1]
    calls += reps
    t = time.perf_counter()
  if calls == 0:
    return float("nan")
  return 1e9 * (t - t0) / (calls * chunk)


def main() -> None:
  parser = argparse.ArgumentParser(
    description="Time randompack distributions (returning) in Python (ns/value)")
  parser.add_argument("engine", nargs="?", default="",
                      help="engine name (default x256++simd)")
  parser.add_argument("-t", type=float, default=0.2, dest="bench_time",
                      help="benchmark time per case (seconds)")
  parser.add_argument("-c", type=int, default=4096, dest="chunk",
                      help="chunk size (values per call)")
  args = parser.parse_args()

  engine = args.engine if args.engine else "x256++simd"
  rng = rp.Rng(engine)

  chunk = args.chunk
  bench_time = args.bench_time
  reps = compute_reps(chunk)

  print(f"Engine: {engine}")
  print(f"{'Distribution':<18} {'ns/value':>8}")

  # Warmup
  t0 = time.perf_counter()
  while time.perf_counter() - t0 < 0.1:
    rng.unif(chunk)
  t1 = time.perf_counter()
  print(f"Warmup time: {t1 - t0:.3f} s\n")

  ns = time_dist(lambda: rng.unif(chunk), chunk, reps, bench_time)
  print(f"{'u01':<18} {ns:8.2f}")

  ns = time_dist(lambda: rng.unif(chunk, a=2, b=5), chunk, reps, bench_time)
  print(f"{'unif(2,5)':<18} {ns:8.2f}")

  ns = time_dist(lambda: rng.normal(chunk), chunk, reps, bench_time)
  print(f"{'norm':<18} {ns:8.2f}")

  ns = time_dist(lambda: rng.normal(chunk, mu=2, sigma=3), chunk, reps,
                 bench_time)
  print(f"{'normal(2,3)':<18} {ns:8.2f}")

  ns = time_dist(lambda: rng.exp(chunk, scale=1), chunk, reps, bench_time)
  print(f"{'exp(1)':<18} {ns:8.2f}")

  ns = time_dist(lambda: rng.exp(chunk, scale=2), chunk, reps, bench_time)
  print(f"{'exp(2)':<18} {ns:8.2f}")

  ns = time_dist(lambda: rng.lognormal(chunk, mu=0, sigma=1), chunk, reps,
                 bench_time)
  print(f"{'lognormal(0,1)':<18} {ns:8.2f}")

  ns = time_dist(lambda: rng.gumbel(chunk, mu=0, beta=1), chunk, reps,
                 bench_time)
  print(f"{'gumbel(0,1)':<18} {ns:8.2f}")

  ns = time_dist(lambda: rng.pareto(chunk, xm=1, alpha=2), chunk, reps,
                 bench_time)
  print(f"{'pareto(1,2)':<18} {ns:8.2f}")

  ns = time_dist(lambda: rng.gamma(chunk, shape=2, scale=3), chunk, reps,
                 bench_time)
  print(f"{'gamma(2,3)':<18} {ns:8.2f}")

  ns = time_dist(lambda: rng.chi2(chunk, nu=5), chunk, reps, bench_time)
  print(f"{'chi2(5)':<18} {ns:8.2f}")

  ns = time_dist(lambda: rng.beta(chunk, a=2, b=5), chunk, reps, bench_time)
  print(f"{'beta(2,5)':<18} {ns:8.2f}")

  ns = time_dist(lambda: rng.t(chunk, nu=10), chunk, reps, bench_time)
  print(f"{'t(10)':<18} {ns:8.2f}")

  ns = time_dist(lambda: rng.f(chunk, nu1=5, nu2=10), chunk, reps,
                 bench_time)
  print(f"{'F(5,10)':<18} {ns:8.2f}")

  ns = time_dist(lambda: rng.weibull(chunk, shape=2, scale=1), chunk, reps,
                 bench_time)
  print(f"{'weibull(2,1)':<18} {ns:8.2f}")


if __name__ == "__main__":
  main()
