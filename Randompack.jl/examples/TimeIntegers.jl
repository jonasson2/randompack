# TimeIntegers.jl
# Time integer draws and permutations (ns/value) in Julia

using Random
using Printf

struct IntRangeSpec
  m::Int
  n::Int
  label::String
end

struct LongLongSpec
  m::Int64
  n::Int64
  label::String
end

struct U8Spec
  bound::Int
  label::String
end

struct PermSpec
  n::Int
  label::String
end

function compute_reps(chunk::Int)
  return max(1, 1_000_000 ÷ chunk)
end

function time_int_range!(rng, chunk::Int, bench_time::Float64, m::Int, n::Int)
  reps = compute_reps(chunk)
  buf = Vector{Int32}(undef, chunk)
  calls = 0
  t0 = time_ns()
  t = t0
  limit = Int(round(bench_time * 1e9))
  while (t - t0) < limit
    for _ = 1:reps
      rand!(rng, buf, m:n)
    end
    calls += reps
    t = time_ns()
  end
  total = calls * chunk
  total == 0 && return NaN
  return (t - t0) / total
end

function time_long_long_range!(rng, chunk::Int, bench_time::Float64, m::Int64, n::Int64)
  reps = compute_reps(chunk)
  buf = Vector{Int64}(undef, chunk)
  calls = 0
  t0 = time_ns()
  t = t0
  limit = Int(round(bench_time * 1e9))
  while (t - t0) < limit
    for _ = 1:reps
      rand!(rng, buf, m:n)
    end
    calls += reps
    t = time_ns()
  end
  total = calls * chunk
  total == 0 && return NaN
  return (t - t0) / total
end

function time_uint8_bound!(rng, chunk::Int, bench_time::Float64, bound::Int)
  reps = compute_reps(chunk)
  buf = Vector{UInt8}(undef, chunk)
  calls = 0
  t0 = time_ns()
  t = t0
  limit = Int(round(bench_time * 1e9))
  while (t - t0) < limit
    for _ = 1:reps
      rand!(rng, buf, UInt8(0):UInt8(bound-1))
    end
    calls += reps
    t = time_ns()
  end
  total = calls * chunk
  total == 0 && return NaN
  return (t - t0) / total
end

function time_uint64_bound!(rng, chunk::Int, bench_time::Float64, bound::UInt64)
  reps = compute_reps(chunk)
  buf = Vector{UInt64}(undef, chunk)
  calls = 0
  t0 = time_ns()
  t = t0
  limit = Int(round(bench_time * 1e9))
  while (t - t0) < limit
    for _ = 1:reps
      rand!(rng, buf, UInt64(0):bound-1)
    end
    calls += reps
    t = time_ns()
  end
  total = calls * chunk
  total == 0 && return NaN
  return (t - t0) / total
end

function time_perm!(rng, bench_time::Float64, n::Int)
  reps = max(1, 100_000 ÷ n)
  buf = collect(1:n)
  calls = 0
  t0 = time_ns()
  t = t0
  limit = Int(round(bench_time * 1e9))
  while (t - t0) < limit
    for _ = 1:reps
      shuffle!(rng, buf)
    end
    calls += reps
    t = time_ns()
  end
  total = calls * n
  total == 0 && return NaN
  return (t - t0) / total
end


function make_int_ranges()
  return [
    IntRangeSpec(1, 3, "1-3"),
    IntRangeSpec(1, 20, "1-20"),
    IntRangeSpec(1, 1000, "1-1000"),
    IntRangeSpec(1, 100000, "1-1e5"),
    IntRangeSpec(1, 10000000, "1-1e7"),
    IntRangeSpec(1, 1000000000, "1-1e9"),
  ]
end

function warmup!(seconds::Float64)
  rng = Xoshiro()
  t0 = time_ns()
  limit = Int(round(seconds * 1e9))
  buf32 = Vector{Int32}(undef, 1024)
  buf64 = Vector{Int64}(undef, 1024)
  buf8 = Vector{UInt8}(undef, 1024)
  perm_buf = collect(1:1000)
  while (time_ns() - t0) < limit
    rand!(rng, buf32, 1:1000)
    rand!(rng, buf64, Int64(1):Int64(1000000))
    rand!(rng, buf8, UInt8(0):UInt8(9))
    shuffle!(rng, perm_buf)
  end
  return nothing
end

function main()
  bench_time = 0.2
  chunk = 4096
  seed = nothing
  i = 1
  while i <= length(ARGS)
    arg = ARGS[i]
    if arg == "-h" || arg == "--help"
      println("Usage: julia TimeIntegers.jl [-t sec] [-c chunk] [-s seed]")
      println("  -t sec     time per case in seconds (default: 0.2)")
      println("  -c chunk   number of draws per call (default: 4096)")
      println("  -s seed    fixed seed (default random seed per case)")
      return
    elseif arg == "-t"
      if i == length(ARGS)
        error("missing value for -t")
      end
      i += 1
      bench_time = parse(Float64, ARGS[i])
      if bench_time <= 0
        error("time per case must be positive")
      end
    elseif arg == "-c"
      if i == length(ARGS)
        error("missing value for -c")
      end
      i += 1
      chunk = parse(Int, ARGS[i])
      if chunk <= 0
        error("chunk must be positive")
      end
    elseif arg == "-s"
      if i == length(ARGS)
        error("missing value for -s")
      end
      i += 1
      seed = parse(Int, ARGS[i])
    else
      error("Unknown option: $arg")
    end
    i += 1
  end

  int_ranges = make_int_ranges()
  u8_specs = [U8Spec(2, "bound 2"), U8Spec(10, "bound 10")]
  perm_specs = [PermSpec(100, "100"), PermSpec(100000, "100000")]
  ll_ranges = [
    LongLongSpec(Int64(1), Int64(10), "1-10"),
    LongLongSpec(Int64(1), Int64(1000), "1-1e3"),
    LongLongSpec(Int64(1), Int64(1000000), "1-1e6"),
    LongLongSpec(Int64(1), Int64(10000000000), "1-1e10"),
    LongLongSpec(Int64(1), Int64(1000000000000000000), "1-1e18"),
  ]

  warmup!(0.1)

  println("time per value:   ns/value")
  @printf("bench_time:       %.3f s per case\n", bench_time)
  @printf("chunk:            %d\n", chunk)
  @printf("\n%-14s %8s\n", "int range", "ns/value")
  for spec in int_ranges
    case_seed = seed === nothing ? rand(RandomDevice(), 0:2^31-1) : seed
    rng = Xoshiro(case_seed)
    ns = time_int_range!(rng, chunk, bench_time, spec.m, spec.n)
    @printf("%-14s %8.2f\n", spec.label, ns)
  end
  @printf("\n%-14s %8s\n", "long long", "ns/value")
  for spec in ll_ranges
    case_seed = seed === nothing ? rand(RandomDevice(), 0:2^31-1) : seed
    rng = Xoshiro(case_seed)
    ns = time_long_long_range!(rng, chunk, bench_time, spec.m, spec.n)
    @printf("%-14s %8.2f\n", spec.label, ns)
  end
  @printf("\n%-14s %8s\n", "uint8", "ns/value")
  for spec in u8_specs
    case_seed = seed === nothing ? rand(RandomDevice(), 0:2^31-1) : seed
    rng = Xoshiro(case_seed)
    ns = time_uint8_bound!(rng, chunk, bench_time, spec.bound)
    @printf("%-14s %8.2f\n", spec.label, ns)
  end
  @printf("\n%-14s %8s\n", "uint64", "ns/value")
  case_seed = seed === nothing ? rand(RandomDevice(), 0:2^31-1) : seed
  rng = Xoshiro(case_seed)
  ns_u64 = time_uint64_bound!(rng, chunk, bench_time, typemax(UInt64) ÷ 3)
  @printf("%-14s %8.2f\n", "UINT64_MAX/3", ns_u64)
  @printf("\n%-14s %10s\n", "perm n", "ns/value")
  for spec in perm_specs
    case_seed = seed === nothing ? rand(RandomDevice(), 0:2^31-1) : seed
    rng = Xoshiro(case_seed)
    ns = time_perm!(rng, bench_time, spec.n)
    @printf("%-14s %10.2f\n", spec.label, ns)
  end
end

main()
