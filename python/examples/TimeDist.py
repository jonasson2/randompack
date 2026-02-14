#!/usr/bin/env python3
# TimeDist.py: compare NumPy vs randompack distributions (ns/value)

import argparse
import os
import platform
import sys
import time
from dataclasses import dataclass
from typing import Callable, List

import numpy as np

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
if ROOT not in sys.path:
  sys.path.insert(0, ROOT)

import randompack as rp


@dataclass
class Dist:
  name: str
  np_fn: Callable[[np.random.Generator, int], np.ndarray]
  rp_fn: Callable[[rp.Rng, int], np.ndarray]


def compute_reps(chunk: int) -> int:
  return max(1, 1_000_000 // chunk)


def time_dist(fn: Callable[[], np.ndarray], chunk: int,
              bench_time: float) -> float:
  reps = compute_reps(chunk)
  calls = 0
  t0 = time.perf_counter()
  t = t0
  while t - t0 < bench_time:
    for _ in range(reps):
      x = fn()
      sink = x[chunk - 1]
    calls += reps
    t = time.perf_counter()
  total_vals = calls * chunk
  if total_vals <= 0:
    return float("nan")
  return 1e9 * (t - t0) / total_vals


def make_dists() -> List[Dist]:
  def np_u01(rng: np.random.Generator, n: int) -> np.ndarray:
    return rng.random(n)

  def np_unif_2_5(rng: np.random.Generator, n: int) -> np.ndarray:
    return rng.uniform(2.0, 5.0, n)

  def np_norm(rng: np.random.Generator, n: int) -> np.ndarray:
    return rng.standard_normal(n)

  def np_normal_2_3(rng: np.random.Generator, n: int) -> np.ndarray:
    return rng.normal(2.0, 3.0, n)

  def np_exp_1(rng: np.random.Generator, n: int) -> np.ndarray:
    return rng.exponential(1.0, n)

  def np_exp_2(rng: np.random.Generator, n: int) -> np.ndarray:
    return rng.exponential(2.0, n)

  def np_logn_0_1(rng: np.random.Generator, n: int) -> np.ndarray:
    return rng.lognormal(0.0, 1.0, n)

  def np_gumbel_0_1(rng: np.random.Generator, n: int) -> np.ndarray:
    return rng.gumbel(0.0, 1.0, n)

  def np_pareto_1_2(rng: np.random.Generator, n: int) -> np.ndarray:
    # Pareto Type I with xm=1, alpha=2: X = xm*(1+Y) where Y ~ Pareto(alpha).
    return rng.pareto(2.0, n) + 1.0

  def np_gamma_2_3(rng: np.random.Generator, n: int) -> np.ndarray:
    return rng.gamma(2.0, 3.0, n)

  def np_chi2_5(rng: np.random.Generator, n: int) -> np.ndarray:
    return rng.chisquare(5.0, n)

  def np_beta_2_5(rng: np.random.Generator, n: int) -> np.ndarray:
    return rng.beta(2.0, 5.0, n)

  def np_t_10(rng: np.random.Generator, n: int) -> np.ndarray:
    return rng.standard_t(10.0, n)

  def np_f_5_10(rng: np.random.Generator, n: int) -> np.ndarray:
    return rng.f(5.0, 10.0, n)

  def np_weibull_2_1(rng: np.random.Generator, n: int) -> np.ndarray:
    return rng.weibull(2.0, n)

  def rp_u01(rng: rp.Rng, n: int) -> np.ndarray:
    return rng.unif(size=n)

  def rp_unif_2_5(rng: rp.Rng, n: int) -> np.ndarray:
    return rng.unif(size=n, a=2, b=5)

  def rp_norm(rng: rp.Rng, n: int) -> np.ndarray:
    return rng.normal(size=n)

  def rp_normal_2_3(rng: rp.Rng, n: int) -> np.ndarray:
    return rng.normal(size=n, mu=2, sigma=3)

  def rp_exp_1(rng: rp.Rng, n: int) -> np.ndarray:
    return rng.exp(size=n, scale=1)

  def rp_exp_2(rng: rp.Rng, n: int) -> np.ndarray:
    return rng.exp(size=n, scale=2)

  def rp_logn_0_1(rng: rp.Rng, n: int) -> np.ndarray:
    return rng.lognormal(size=n, mu=0, sigma=1)

  def rp_gumbel_0_1(rng: rp.Rng, n: int) -> np.ndarray:
    return rng.gumbel(size=n, mu=0, beta=1)

  def rp_pareto_1_2(rng: rp.Rng, n: int) -> np.ndarray:
    return rng.pareto(size=n, xm=1, alpha=2)

  def rp_gamma_2_3(rng: rp.Rng, n: int) -> np.ndarray:
    return rng.gamma(size=n, shape=2, scale=3)

  def rp_chi2_5(rng: rp.Rng, n: int) -> np.ndarray:
    return rng.chi2(size=n, nu=5)

  def rp_beta_2_5(rng: rp.Rng, n: int) -> np.ndarray:
    return rng.beta(size=n, a=2, b=5)

  def rp_t_10(rng: rp.Rng, n: int) -> np.ndarray:
    return rng.t(size=n, nu=10)

  def rp_f_5_10(rng: rp.Rng, n: int) -> np.ndarray:
    return rng.f(size=n, nu1=5, nu2=10)

  def rp_weibull_2_1(rng: rp.Rng, n: int) -> np.ndarray:
    return rng.weibull(size=n, shape=2, scale=1)

  return [
    Dist("unif(0,1)", np_u01, rp_u01),
    Dist("unif(2,5)", np_unif_2_5, rp_unif_2_5),
    Dist("std.normal", np_norm, rp_norm),
    Dist("normal(2,3)", np_normal_2_3, rp_normal_2_3),
    Dist("std.exp", np_exp_1, rp_exp_1),
    Dist("exp(2)", np_exp_2, rp_exp_2),
    Dist("lognormal(0,1)", np_logn_0_1, rp_logn_0_1),
    Dist("gumbel(0,1)", np_gumbel_0_1, rp_gumbel_0_1),
    Dist("pareto(1,2)", np_pareto_1_2, rp_pareto_1_2),
    Dist("gamma(2,3)", np_gamma_2_3, rp_gamma_2_3),
    Dist("chi2(5)", np_chi2_5, rp_chi2_5),
    Dist("beta(2,5)", np_beta_2_5, rp_beta_2_5),
    Dist("t(10)", np_t_10, rp_t_10),
    Dist("F(5,10)", np_f_5_10, rp_f_5_10),
    Dist("weibull(2,1)", np_weibull_2_1, rp_weibull_2_1),
  ]


def main() -> None:
  parser = argparse.ArgumentParser(
    description="Compare NumPy and randompack distributions (ns/value)")
  parser.add_argument("engine", nargs="?", default="",
                      help="engine name (default x256++simd)")
  parser.add_argument("-t", type=float, default=0.2, dest="bench_time",
                      help="benchmark time per case (seconds)")
  parser.add_argument("-c", type=int, default=4096, dest="chunk",
                      help="chunk size (values per call)")
  args = parser.parse_args()

  engine = args.engine if args.engine else "x256++simd"
  chunk = args.chunk
  bench_time = args.bench_time

  np_rng = np.random.default_rng(7)
  rp_rng = rp.Rng(engine)

  t0 = time.perf_counter()
  while time.perf_counter() - t0 < 0.1:
    rp_rng.unif(size=chunk)
    np_rng.random(chunk)
  warm = time.perf_counter() - t0

  print(f"Platform:  {platform.platform()}")
  print(f"Engine:    {engine}")
  print(f"Warmup:    {warm:.3f} s")
  print(f"Time/case: {bench_time:.3f} s\n")

  print(f"{'DISTRIBUTION':<14} {'NUMPY':>10} {'RANDOMPACK':>11} {'FACTOR':>7}")
  for d in make_dists():
    np_ns = time_dist(lambda: d.np_fn(np_rng, chunk), chunk, bench_time)
    rp_ns = time_dist(lambda: d.rp_fn(rp_rng, chunk), chunk, bench_time)
    factor = np_ns / rp_ns if rp_ns > 0 else float("nan")
    print(f"{d.name:<14} {np_ns:10.2f} {rp_ns:11.2f} {factor:7.2f}")


if __name__ == "__main__":
  main()
