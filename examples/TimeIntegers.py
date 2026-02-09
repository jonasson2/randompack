#!/usr/bin/env python3
# TimeIntegers.py: time integer draws and permutations (ns/value) in Python.

import argparse
import time
from dataclasses import dataclass
from typing import Callable, List

import numpy as np


@dataclass
class IntRangeSpec:
  m: int
  n: int
  label: str


@dataclass
class U8Spec:
  bound: int
  label: str


@dataclass
class PermSpec:
  n: int
  label: str


@dataclass
class SampleSpec:
  n: int
  k: int
  label: str


def compute_reps(chunk: int) -> int:
  return max(1, 1_000_000 // chunk)


def time_int_range(rng: np.random.Generator,
                   chunk: int,
                   bench_time: float,
                   m: int,
                   n: int) -> float:
  reps = compute_reps(chunk)
  calls = 0
  t0 = time.perf_counter()
  t = t0
  while t - t0 < bench_time:
    for _ in range(reps):
      rng.integers(m, n + 1, size=chunk, dtype=np.int32)
    calls += reps
    t = time.perf_counter()
  total_vals = calls * chunk
  if total_vals <= 0:
    return float("nan")
  return 1e9 * (t - t0) / total_vals


def time_uint8_bound(rng: np.random.Generator,
                     chunk: int,
                     bench_time: float,
                     bound: int) -> float:
  reps = compute_reps(chunk)
  calls = 0
  t0 = time.perf_counter()
  t = t0
  while t - t0 < bench_time:
    for _ in range(reps):
      rng.integers(0, bound, size=chunk, dtype=np.uint8)
    calls += reps
    t = time.perf_counter()
  total_vals = calls * chunk
  if total_vals <= 0:
    return float("nan")
  return 1e9 * (t - t0) / total_vals


def time_uint64_bound(rng: np.random.Generator,
                      chunk: int,
                      bench_time: float,
                      bound: int) -> float:
  reps = compute_reps(chunk)
  calls = 0
  t0 = time.perf_counter()
  t = t0
  while t - t0 < bench_time:
    for _ in range(reps):
      rng.integers(0, bound, size=chunk, dtype=np.uint64)
    calls += reps
    t = time.perf_counter()
  total_vals = calls * chunk
  if total_vals <= 0:
    return float("nan")
  return 1e9 * (t - t0) / total_vals


def time_perm(rng: np.random.Generator,
              bench_time: float,
              n: int) -> float:
  reps = max(1, 100_000 // n)
  calls = 0
  t0 = time.perf_counter()
  t = t0
  while t - t0 < bench_time:
    for _ in range(reps):
      rng.permutation(n)
    calls += reps
    t = time.perf_counter()
  total_vals = calls * n
  if total_vals <= 0:
    return float("nan")
  return 1e9 * (t - t0) / total_vals


def time_sample(rng: np.random.Generator,
                bench_time: float,
                n: int,
                k: int) -> float:
  reps = max(1, 100_000 // n)
  calls = 0
  t0 = time.perf_counter()
  t = t0
  while t - t0 < bench_time:
    for _ in range(reps):
      rng.choice(n, size=k, replace=False)
    calls += reps
    t = time.perf_counter()
  total_vals = calls * k
  if total_vals <= 0:
    return float("nan")
  return 1e9 * (t - t0) / total_vals


def make_int_ranges() -> List[IntRangeSpec]:
  return [
    IntRangeSpec(0, 2, "[0,2]"),
    IntRangeSpec(1, 10, "[1,10]"),
    IntRangeSpec(1, 100, "[1,100]"),
    IntRangeSpec(1, 1000, "[1,1000]"),
    IntRangeSpec(1, 10000, "[1,10000]"),
    IntRangeSpec(1, 100000, "[1,100000]"),
    IntRangeSpec(1, 1000000, "[1,1000000]"),
  ]


def main() -> None:
  parser = argparse.ArgumentParser(
    description="Time integer draws and permutations (ns/value)")
  parser.add_argument("-t", type=float, default=0.1, dest="bench_time",
                      help="benchmark time per case (seconds)")
  parser.add_argument("-c", type=int, default=4096, dest="chunk",
                      help="chunk size (values per call)")
  parser.add_argument("-s", type=int, default=7, dest="seed",
                      help="random seed")
  args = parser.parse_args()

  rng = np.random.default_rng(args.seed)

  int_ranges = make_int_ranges()
  u8_specs = [U8Spec(2, "bound 2"), U8Spec(10, "bound 10")]
  perm_specs = [PermSpec(100, "100"), PermSpec(100000, "100000")]
  sample_specs = [
    SampleSpec(1000, 10, "1000/10"),
    SampleSpec(1000, 499, "1000/499"),
    SampleSpec(1000, 501, "1000/501"),
    SampleSpec(1000, 990, "1000/990"),
  ]

  # Warm up.
  for spec in int_ranges:
    rng.integers(spec.m, spec.n + 1, size=16, dtype=np.int32)
  for spec in u8_specs:
    rng.integers(0, spec.bound, size=16, dtype=np.uint8)
  rng.integers(0, (1 << 64) // 3, size=16, dtype=np.uint64)
  for spec in perm_specs:
    rng.permutation(spec.n)
  for spec in sample_specs:
    rng.choice(spec.n, size=spec.k, replace=False)

  print("time per value:   ns/value")
  print(f"bench_time:       {args.bench_time:.3f} s per case")
  print(f"chunk:            {args.chunk}")
  print("\n%-14s %8s" % ("int range", "ns/value"))
  for spec in int_ranges:
    ns = time_int_range(rng, args.chunk, args.bench_time, spec.m, spec.n)
    print("%-14s %8.2f" % (spec.label, ns))
  print("\n%-14s %8s" % ("uint8", "ns/value"))
  for spec in u8_specs:
    ns = time_uint8_bound(rng, args.chunk, args.bench_time, spec.bound)
    print("%-14s %8.2f" % (spec.label, ns))
  print("\n%-14s %8s" % ("uint64", "ns/value"))
  ns_u64 = time_uint64_bound(rng, args.chunk, args.bench_time, (1 << 64) // 3)
  print("%-14s %8.2f" % ("UINT64_MAX/3", ns_u64))
  print("\n%-14s %10s" % ("perm n", "ns/value"))
  for spec in perm_specs:
    ns = time_perm(rng, args.bench_time, spec.n)
    print("%-14s %10.2f" % (spec.label, ns))
  print("\n%-14s %10s" % ("sample", "ns/value"))
  for spec in sample_specs:
    ns = time_sample(rng, args.bench_time, spec.n, spec.k)
    print("%-14s %10.2f" % (spec.label, ns))


if __name__ == "__main__":
  main()
