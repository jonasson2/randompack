#!/usr/bin/env python3
# TimeDistributions.py: time continuous distributions (ns/value) in Python.

import math
import time
from dataclasses import dataclass
from typing import Callable, List, Optional, Tuple

import numpy as np


@dataclass
class Dist:
  name: str
  fn: Callable[[np.random.Generator, int], None]


def compute_reps(chunk: int) -> int:
  # Roughly match the C logic: ~1e6 values between timer checks.
  return max(1, 1_000_000 // chunk)


def time_dist(fn: Callable[[np.random.Generator, int], None],
              rng: np.random.Generator,
              chunk: int,
              bench_time: float) -> float:
  reps = compute_reps(chunk)
  calls = 0
  t0 = time.perf_counter()
  t = t0
  while t - t0 < bench_time:
    for _ in range(reps):
      fn(rng, chunk)
    calls += reps
    t = time.perf_counter()
  total_vals = calls * chunk
  if total_vals <= 0:
    return float("nan")
  return 1e9 * (t - t0) / total_vals


def make_dists() -> List[Dist]:
  def u01(rng: np.random.Generator, n: int) -> None:
    rng.random(n)

  def unif_2_5(rng: np.random.Generator, n: int) -> None:
    rng.uniform(2.0, 5.0, n)

  def norm(rng: np.random.Generator, n: int) -> None:
    rng.standard_normal(n)

  def normal_2_3(rng: np.random.Generator, n: int) -> None:
    rng.normal(2.0, 3.0, n)

  def lognormal_0_1(rng: np.random.Generator, n: int) -> None:
    rng.lognormal(0.0, 1.0, n)

  def gumbel_0_1(rng: np.random.Generator, n: int) -> None:
    rng.gumbel(0.0, 1.0, n)

  def pareto_1_2(rng: np.random.Generator, n: int) -> None:
    # Pareto Type I with xm=1, alpha=2: X = xm*(1+Y) where Y ~ Pareto(alpha) in NumPy.
    rng.pareto(2.0, n) + 1.0

  def exp_1(rng: np.random.Generator, n: int) -> None:
    rng.exponential(1.0, n)

  def exp_2(rng: np.random.Generator, n: int) -> None:
    rng.exponential(2.0, n)

  def gamma_2_3(rng: np.random.Generator, n: int) -> None:
    rng.gamma(2.0, 3.0, n)

  def chi2_5(rng: np.random.Generator, n: int) -> None:
    rng.chisquare(5.0, n)

  def beta_2_5(rng: np.random.Generator, n: int) -> None:
    rng.beta(2.0, 5.0, n)

  def t_10(rng: np.random.Generator, n: int) -> None:
    rng.standard_t(10.0, n)

  def f_5_10(rng: np.random.Generator, n: int) -> None:
    rng.f(5.0, 10.0, n)

  def weibull_2_3(rng: np.random.Generator, n: int) -> None:
    # Weibull with shape k=2, scale=3.
    rng.weibull(2.0, n) * 3.0

  return [
    Dist("u01", u01),
    Dist("u01", u01),
    Dist("unif(2,5)", unif_2_5),
    Dist("norm", norm),
    Dist("normal(2,3)", normal_2_3),
    Dist("lognormal(0,1)", lognormal_0_1),
    Dist("gumbel(0,1)", gumbel_0_1),
    Dist("pareto(1,2)", pareto_1_2),
    Dist("exp(1)", exp_1),
    Dist("exp(2)", exp_2),
    Dist("gamma(2,3)", gamma_2_3),
    Dist("chi2(5)", chi2_5),
    Dist("beta(2,5)", beta_2_5),
    Dist("t(10)", t_10),
    Dist("F(5,10)", f_5_10),
    Dist("weibull(2,3)", weibull_2_3),
  ]


def main() -> None:
  chunk = 4096
  bench_time = 0.2
  seed = 7

  rng = np.random.default_rng(seed)

  # Warm up: small draws for each distribution to avoid one-time setup effects.
  for d in make_dists():
    d.fn(rng, 16)

  print("Distribution       ns/value")
  for d in make_dists():
    ns = time_dist(d.fn, rng, chunk, bench_time)
    print(f"{d.name:<16} {ns:8.2f}")


if __name__ == "__main__":
  main()
