#!/usr/bin/env python3
# TimeIntegers.py: compare NumPy vs Randompack for integer draws (ns/value).

import argparse
import random
import time

import numpy as np
try:
  import randompack as rp
except ImportError:
  print("Error: randompack module not found")
  print("Please install randompack or add it to PYTHONPATH")
  import sys
  sys.exit(1)


def compute_reps(chunk: int) -> int:
  return max(1, 1_000_000 // chunk)


def time_int_range(rng, chunk: int, bench_time: float, m: int, n: int,
                   use_randompack: bool) -> float:
  reps = compute_reps(chunk)
  buf = np.empty(chunk, dtype=np.int32)
  calls = 0
  t0 = time.perf_counter()
  t = t0
  while t - t0 < bench_time:
    for _ in range(reps):
      if use_randompack:
        rng.int(out=buf, a=m, b=n)
      else:
        rng.integers(m, n + 1, size=chunk, dtype=np.int32)
    calls += reps
    t = time.perf_counter()
  total_vals = calls * chunk
  if total_vals <= 0:
    return float("nan")
  return 1e9 * (t - t0) / total_vals


def time_long_long_range(rng, chunk: int, bench_time: float, m: int, n: int,
                          use_randompack: bool) -> float:
  reps = compute_reps(chunk)
  buf = np.empty(chunk, dtype=np.int64)
  calls = 0
  t0 = time.perf_counter()
  t = t0
  while t - t0 < bench_time:
    for _ in range(reps):
      if use_randompack:
        rng.int(out=buf, a=m, b=n)
      else:
        rng.integers(m, n + 1, size=chunk, dtype=np.int64)
    calls += reps
    t = time.perf_counter()
  total_vals = calls * chunk
  if total_vals <= 0:
    return float("nan")
  return 1e9 * (t - t0) / total_vals


def time_perm(rng, bench_time: float, n: int, use_randompack: bool) -> float:
  reps = max(1, 100_000 // n)
  buf = np.empty(n, dtype=np.int32)
  calls = 0
  t0 = time.perf_counter()
  t = t0
  while t - t0 < bench_time:
    for _ in range(reps):
      if use_randompack:
        rng.perm(n, out=buf)
      else:
        rng.permutation(n)
    calls += reps
    t = time.perf_counter()
  total_vals = calls * n
  if total_vals <= 0:
    return float("nan")
  return 1e9 * (t - t0) / total_vals


def time_sample(rng, bench_time: float, n: int, k: int,
                use_randompack: bool) -> float:
  reps = max(1, 100_000 // n)
  buf = np.empty(k, dtype=np.int32)
  calls = 0
  t0 = time.perf_counter()
  t = t0
  while t - t0 < bench_time:
    for _ in range(reps):
      if use_randompack:
        rng.sample(n, k, out=buf)
      else:
        rng.choice(n, size=k, replace=False)
    calls += reps
    t = time.perf_counter()
  total_vals = calls * k
  if total_vals <= 0:
    return float("nan")
  return 1e9 * (t - t0) / total_vals


def warmup(seconds: float) -> float:
  np_rng = np.random.default_rng(123)
  rp_rng = rp.Rng()
  t0 = time.perf_counter()
  buf32 = np.empty(1024, dtype=np.int32)
  buf64 = np.empty(1024, dtype=np.int64)
  perm_buf = np.empty(1000, dtype=np.int32)
  while time.perf_counter() - t0 < seconds:
    np_rng.integers(1, 1000, size=1024, dtype=np.int32)
    np_rng.integers(1, 1000000, size=1024, dtype=np.int64)
    np_rng.choice(1000, size=100, replace=False)
    np_rng.permutation(1000)
    rp_rng.int(out=buf32, a=1, b=1000)
    rp_rng.int(out=buf64, a=1, b=1000000)
    rp_rng.perm(1000, out=perm_buf)
  return time.perf_counter() - t0


def main() -> None:
  parser = argparse.ArgumentParser(
    description="Compare NumPy vs Randompack for integer draws (ns/value)")
  parser.add_argument("-t", type=float, default=0.2, dest="bench_time",
                      help="benchmark time per case (seconds)")
  parser.add_argument("-c", type=int, default=4096, dest="chunk",
                      help="chunk size (values per call)")
  parser.add_argument("-s", type=int, default=None, dest="seed",
                      help="fixed seed (default random seed per case)")
  parser.add_argument("-e", type=str, default="x256++simd", dest="engine",
                      help="Randompack engine (default x256++simd)")
  args = parser.parse_args()

  warm = warmup(0.1)

  print("time per value:   ns/value")
  print(f"bench_time:       {args.bench_time:.3f} s per case")
  print(f"warmup:           {warm:.3f} s")
  print(f"chunk:            {args.chunk}")
  print(f"engine:           {args.engine}")
  print()
  print("%-18s %10s %11s %8s" % ("Benchmark", "NumPy", "Randompack", "Factor"))

  # int 1-10
  case_seed = args.seed if args.seed is not None else random.randint(0, 2**31 - 1)
  np_rng = np.random.default_rng(case_seed)
  rp_rng = rp.Rng(args.engine)
  rp_rng.seed(case_seed)
  np_ns = time_int_range(np_rng, args.chunk, args.bench_time, 1, 10, False)
  np_rng = np.random.default_rng(case_seed)
  rp_rng.seed(case_seed)
  rp_ns = time_int_range(rp_rng, args.chunk, args.bench_time, 1, 10, True)
  factor = np_ns / rp_ns
  print("%-18s %10.2f %11.2f %8.2f" % ("int 1-10", np_ns, rp_ns, factor))

  # int 1-1e5
  case_seed = args.seed if args.seed is not None else random.randint(0, 2**31 - 1)
  np_rng = np.random.default_rng(case_seed)
  rp_rng = rp.Rng(args.engine)
  rp_rng.seed(case_seed)
  np_ns = time_int_range(np_rng, args.chunk, args.bench_time, 1, 100000, False)
  np_rng = np.random.default_rng(case_seed)
  rp_rng.seed(case_seed)
  rp_ns = time_int_range(rp_rng, args.chunk, args.bench_time, 1, 100000, True)
  factor = np_ns / rp_ns
  print("%-18s %10.2f %11.2f %8.2f" % ("int 1-1e5", np_ns, rp_ns, factor))

  # int 1-2e9
  case_seed = args.seed if args.seed is not None else random.randint(0, 2**31 - 1)
  np_rng = np.random.default_rng(case_seed)
  rp_rng = rp.Rng(args.engine)
  rp_rng.seed(case_seed)
  np_ns = time_int_range(np_rng, args.chunk, args.bench_time, 1, 2000000000, False)
  np_rng = np.random.default_rng(case_seed)
  rp_rng.seed(case_seed)
  rp_ns = time_int_range(rp_rng, args.chunk, args.bench_time, 1, 2000000000, True)
  factor = np_ns / rp_ns
  print("%-18s %10.2f %11.2f %8.2f" % ("int 1-2e9", np_ns, rp_ns, factor))

  # long long 1-2e9
  case_seed = args.seed if args.seed is not None else random.randint(0, 2**31 - 1)
  np_rng = np.random.default_rng(case_seed)
  rp_rng = rp.Rng(args.engine)
  rp_rng.seed(case_seed)
  np_ns = time_long_long_range(np_rng, args.chunk, args.bench_time, 1, 2000000000, False)
  np_rng = np.random.default_rng(case_seed)
  rp_rng.seed(case_seed)
  rp_ns = time_long_long_range(rp_rng, args.chunk, args.bench_time, 1, 2000000000, True)
  factor = np_ns / rp_ns
  print("%-18s %10.2f %11.2f %8.2f" % ("long long 1-2e9", np_ns, rp_ns, factor))

  # long long 1-6e18
  case_seed = args.seed if args.seed is not None else random.randint(0, 2**31 - 1)
  np_rng = np.random.default_rng(case_seed)
  rp_rng = rp.Rng(args.engine)
  rp_rng.seed(case_seed)
  np_ns = time_long_long_range(np_rng, args.chunk, args.bench_time, 1, 6000000000000000000, False)
  np_rng = np.random.default_rng(case_seed)
  rp_rng.seed(case_seed)
  rp_ns = time_long_long_range(rp_rng, args.chunk, args.bench_time, 1, 6000000000000000000, True)
  factor = np_ns / rp_ns
  print("%-18s %10.2f %11.2f %8.2f" % ("long long 1-6e18", np_ns, rp_ns, factor))

  # perm 100
  case_seed = args.seed if args.seed is not None else random.randint(0, 2**31 - 1)
  np_rng = np.random.default_rng(case_seed)
  rp_rng = rp.Rng(args.engine)
  rp_rng.seed(case_seed)
  np_ns = time_perm(np_rng, args.bench_time, 100, False)
  np_rng = np.random.default_rng(case_seed)
  rp_rng.seed(case_seed)
  rp_ns = time_perm(rp_rng, args.bench_time, 100, True)
  factor = np_ns / rp_ns
  print("%-18s %10.2f %11.2f %8.2f" % ("perm 100", np_ns, rp_ns, factor))

  # perm 100000
  case_seed = args.seed if args.seed is not None else random.randint(0, 2**31 - 1)
  np_rng = np.random.default_rng(case_seed)
  rp_rng = rp.Rng(args.engine)
  rp_rng.seed(case_seed)
  np_ns = time_perm(np_rng, args.bench_time, 100000, False)
  np_rng = np.random.default_rng(case_seed)
  rp_rng.seed(case_seed)
  rp_ns = time_perm(rp_rng, args.bench_time, 100000, True)
  factor = np_ns / rp_ns
  print("%-18s %10.2f %11.2f %8.2f" % ("perm 100000", np_ns, rp_ns, factor))

  # sample 20/1000
  case_seed = args.seed if args.seed is not None else random.randint(0, 2**31 - 1)
  np_rng = np.random.default_rng(case_seed)
  rp_rng = rp.Rng(args.engine)
  rp_rng.seed(case_seed)
  np_ns = time_sample(np_rng, args.bench_time, 1000, 20, False)
  np_rng = np.random.default_rng(case_seed)
  rp_rng.seed(case_seed)
  rp_ns = time_sample(rp_rng, args.bench_time, 1000, 20, True)
  factor = np_ns / rp_ns
  print("%-18s %10.2f %11.2f %8.2f" % ("sample 20/1000", np_ns, rp_ns, factor))

  # sample 500/1000
  case_seed = args.seed if args.seed is not None else random.randint(0, 2**31 - 1)
  np_rng = np.random.default_rng(case_seed)
  rp_rng = rp.Rng(args.engine)
  rp_rng.seed(case_seed)
  np_ns = time_sample(np_rng, args.bench_time, 1000, 500, False)
  np_rng = np.random.default_rng(case_seed)
  rp_rng.seed(case_seed)
  rp_ns = time_sample(rp_rng, args.bench_time, 1000, 500, True)
  factor = np_ns / rp_ns
  print("%-18s %10.2f %11.2f %8.2f" % ("sample 500/1000", np_ns, rp_ns, factor))

  # sample 980/1000
  case_seed = args.seed if args.seed is not None else random.randint(0, 2**31 - 1)
  np_rng = np.random.default_rng(case_seed)
  rp_rng = rp.Rng(args.engine)
  rp_rng.seed(case_seed)
  np_ns = time_sample(np_rng, args.bench_time, 1000, 980, False)
  np_rng = np.random.default_rng(case_seed)
  rp_rng.seed(case_seed)
  rp_ns = time_sample(rp_rng, args.bench_time, 1000, 980, True)
  factor = np_ns / rp_ns
  print("%-18s %10.2f %11.2f %8.2f" % ("sample 980/1000", np_ns, rp_ns, factor))


if __name__ == "__main__":
  main()
