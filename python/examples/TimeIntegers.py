#!/usr/bin/env python3
# TimeIntegers.py: time integer draws and permutations (ns/value) in Python.

import argparse
import random
import time
from dataclasses import dataclass
from typing import List

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


def time_long_long_range(rng: np.random.Generator,
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
      rng.integers(m, n + 1, size=chunk, dtype=np.int64)
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
    IntRangeSpec(1, 3, "1-3"),
    IntRangeSpec(1, 20, "1-20"),
    IntRangeSpec(1, 1000, "1-1000"),
    IntRangeSpec(1, 100000, "1-1e5"),
    IntRangeSpec(1, 10000000, "1-1e7"),
    IntRangeSpec(1, 1000000000, "1-1e9"),
  ]


def warmup(seconds: float) -> float:
  rng = np.random.default_rng(123)
  t0 = time.perf_counter()
  while time.perf_counter() - t0 < seconds:
    rng.integers(1, 1000, size=1024, dtype=np.int32)
    rng.integers(1, 1000000, size=1024, dtype=np.int64)
    rng.integers(0, 10, size=1024, dtype=np.uint8)
    rng.choice(1000, size=100, replace=False)
    rng.permutation(1000)
  return time.perf_counter() - t0


def main() -> None:
  parser = argparse.ArgumentParser(
    description="Time integer draws and permutations (ns/value)")
  parser.add_argument("-t", type=float, default=0.2, dest="bench_time",
                      help="benchmark time per case (seconds)")
  parser.add_argument("-c", type=int, default=4096, dest="chunk",
                      help="chunk size (values per call)")
  parser.add_argument("-s", type=int, default=None, dest="seed",
                      help="fixed seed (default random seed per case)")
  args = parser.parse_args()

  int_ranges = make_int_ranges()
  u8_specs = [U8Spec(2, "bound 2"), U8Spec(10, "bound 10")]
  perm_specs = [PermSpec(100, "100"), PermSpec(100000, "100000")]
  sample_specs = [
    SampleSpec(1000, 10, "1000/10"),
    SampleSpec(1000, 499, "1000/499"),
    SampleSpec(1000, 501, "1000/501"),
    SampleSpec(1000, 990, "1000/990"),
  ]
  ll_ranges = [
    IntRangeSpec(1, 10, "1-10"),
    IntRangeSpec(1, 1000, "1-1e3"),
    IntRangeSpec(1, 1000000, "1-1e6"),
    IntRangeSpec(1, 10000000000, "1-1e10"),
    IntRangeSpec(1, 1000000000000000000, "1-1e18"),
  ]

  warm = warmup(0.1)

  print("time per value:   ns/value")
  print(f"bench_time:       {args.bench_time:.3f} s per case")
  print(f"warmup:           {warm:.3f} s")
  print(f"chunk:            {args.chunk}")
  print("\n%-14s %8s" % ("int range", "ns/value"))
  for spec in int_ranges:
    case_seed = args.seed if args.seed is not None else random.randint(0, 2**31 - 1)
    rng = np.random.default_rng(case_seed)
    ns = time_int_range(rng, args.chunk, args.bench_time, spec.m, spec.n)
    print("%-14s %8.2f" % (spec.label, ns))
  print("\n%-14s %8s" % ("long long", "ns/value"))
  for spec in ll_ranges:
    case_seed = args.seed if args.seed is not None else random.randint(0, 2**31 - 1)
    rng = np.random.default_rng(case_seed)
    ns = time_long_long_range(rng, args.chunk, args.bench_time, spec.m, spec.n)
    print("%-14s %8.2f" % (spec.label, ns))
  print("\n%-14s %8s" % ("uint8", "ns/value"))
  for spec in u8_specs:
    case_seed = args.seed if args.seed is not None else random.randint(0, 2**31 - 1)
    rng = np.random.default_rng(case_seed)
    ns = time_uint8_bound(rng, args.chunk, args.bench_time, spec.bound)
    print("%-14s %8.2f" % (spec.label, ns))
  print("\n%-14s %8s" % ("uint64", "ns/value"))
  case_seed = args.seed if args.seed is not None else random.randint(0, 2**31 - 1)
  rng = np.random.default_rng(case_seed)
  ns_u64 = time_uint64_bound(rng, args.chunk, args.bench_time, (1 << 64) // 3)
  print("%-14s %8.2f" % ("UINT64_MAX/3", ns_u64))
  print("\n%-14s %10s" % ("perm n", "ns/value"))
  for spec in perm_specs:
    case_seed = args.seed if args.seed is not None else random.randint(0, 2**31 - 1)
    rng = np.random.default_rng(case_seed)
    ns = time_perm(rng, args.bench_time, spec.n)
    print("%-14s %10.2f" % (spec.label, ns))
  print("\n%-14s %10s" % ("sample", "ns/value"))
  for spec in sample_specs:
    case_seed = args.seed if args.seed is not None else random.randint(0, 2**31 - 1)
    rng = np.random.default_rng(case_seed)
    ns = time_sample(rng, args.bench_time, spec.n, spec.k)
    print("%-14s %10.2f" % (spec.label, ns))


if __name__ == "__main__":
  main()
