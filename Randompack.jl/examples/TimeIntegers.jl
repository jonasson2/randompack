# TimeIntegers.jl
# Compare Julia Random vs Randompack for integer draws (ns/value)

using Random
using Printf
using Randompack

function compute_reps(chunk::Int)
  return max(1, 1_000_000 ÷ chunk)
end

function time_int_range!(rng, chunk::Int, bench_time::Float64, m::Int, n::Int,
                         use_randompack::Bool)
  reps = compute_reps(chunk)
  buf = Vector{Int32}(undef, chunk)
  calls = 0
  t0 = time_ns()
  t = t0
  limit = Int(round(bench_time * 1e9))
  while (t - t0) < limit
    for _ = 1:reps
      if use_randompack
        random_int!(rng, buf, m:n)
      else
        rand!(rng, buf, m:n)
      end
    end
    calls += reps
    t = time_ns()
  end
  total = calls * chunk
  total == 0 && return NaN
  return (t - t0) / total
end

function time_long_long_range!(rng, chunk::Int, bench_time::Float64, m::Int64, n::Int64,
                                use_randompack::Bool)
  reps = compute_reps(chunk)
  buf = Vector{Int64}(undef, chunk)
  calls = 0
  t0 = time_ns()
  t = t0
  limit = Int(round(bench_time * 1e9))
  while (t - t0) < limit
    for _ = 1:reps
      if use_randompack
        random_int!(rng, buf, m:n)
      else
        rand!(rng, buf, m:n)
      end
    end
    calls += reps
    t = time_ns()
  end
  total = calls * chunk
  total == 0 && return NaN
  return (t - t0) / total
end

function time_perm!(rng, bench_time::Float64, n::Int, use_randompack::Bool)
  reps = max(1, 100_000 ÷ n)
  buf = Vector{Int32}(undef, n)
  calls = 0
  t0 = time_ns()
  t = t0
  limit = Int(round(bench_time * 1e9))
  while (t - t0) < limit
    for _ = 1:reps
      if use_randompack
        random_perm!(rng, buf)
      else
        for i = 1:n
          buf[i] = i
        end
        shuffle!(rng, buf)
      end
    end
    calls += reps
    t = time_ns()
  end
  total = calls * n
  total == 0 && return NaN
  return (t - t0) / total
end

function time_sample!(rng, bench_time::Float64, n::Int, k::Int,
                      use_randompack::Bool)
  reps = max(1, 100_000 ÷ n)
  buf = Vector{Int32}(undef, k)
  calls = 0
  t0 = time_ns()
  t = t0
  limit = Int(round(bench_time * 1e9))
  while (t - t0) < limit
    for _ = 1:reps
      if use_randompack
        random_sample!(rng, buf, n)
      else
        randperm(rng, n)[1:k]
      end
    end
    calls += reps
    t = time_ns()
  end
  total = calls * k
  total == 0 && return NaN
  return (t - t0) / total
end

function warmup!(seconds::Float64)
  jl_rng = Xoshiro()
  rp_rng = rng_create()
  t0 = time_ns()
  limit = Int(round(seconds * 1e9))
  buf32 = Vector{Int32}(undef, 1024)
  buf64 = Vector{Int64}(undef, 1024)
  perm_buf = Vector{Int32}(undef, 1000)
  while (time_ns() - t0) < limit
    rand!(jl_rng, buf32, 1:1000)
    rand!(jl_rng, buf64, Int64(1):Int64(1000000))
    random_int!(rp_rng, buf32, 1:1000)
    random_int!(rp_rng, buf64, Int64(1):Int64(1000000))
    random_perm!(rp_rng, perm_buf)
  end
  return nothing
end

function main()
  bench_time = 0.2
  chunk = 4096
  seed = nothing
  engine = "x256++simd"
  i = 1
  while i <= length(ARGS)
    arg = ARGS[i]
    if arg == "-h" || arg == "--help"
      println("Usage: julia TimeIntegers.jl [-t sec] [-c chunk] [-s seed] [-e engine]")
      println("  -t sec     time per case in seconds (default: 0.2)")
      println("  -c chunk   number of draws per call (default: 4096)")
      println("  -s seed    fixed seed (default random seed per case)")
      println("  -e engine  Randompack engine (default: x256++simd)")
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
    elseif arg == "-e"
      if i == length(ARGS)
        error("missing value for -e")
      end
      i += 1
      engine = ARGS[i]
    else
      error("Unknown option: $arg")
    end
    i += 1
  end

  warm = warmup!(0.1)

  println("time per value:   ns/value")
  @printf("bench_time:       %.3f s per case\n", bench_time)
  @printf("chunk:            %d\n", chunk)
  @printf("engine:           %s\n", engine)
  println()
  @printf("%-18s %10s %11s %8s\n", "Benchmark", "Julia", "Randompack", "Factor")

  # int 1-10
  case_seed = seed === nothing ? rand(RandomDevice(), 0:2^31-1) : seed
  jl_rng = Xoshiro(case_seed)
  rp_rng = rng_create(engine)
  rng_seed!(rp_rng, case_seed)
  jl_ns = time_int_range!(jl_rng, chunk, bench_time, 1, 10, false)
  jl_rng = Xoshiro(case_seed)
  rng_seed!(rp_rng, case_seed)
  rp_ns = time_int_range!(rp_rng, chunk, bench_time, 1, 10, true)
  factor = jl_ns / rp_ns
  @printf("%-18s %10.2f %11.2f %8.2f\n", "int 1-10", jl_ns, rp_ns, factor)

  # int 1-1e5
  case_seed = seed === nothing ? rand(RandomDevice(), 0:2^31-1) : seed
  jl_rng = Xoshiro(case_seed)
  rp_rng = rng_create(engine)
  rng_seed!(rp_rng, case_seed)
  jl_ns = time_int_range!(jl_rng, chunk, bench_time, 1, 100000, false)
  jl_rng = Xoshiro(case_seed)
  rng_seed!(rp_rng, case_seed)
  rp_ns = time_int_range!(rp_rng, chunk, bench_time, 1, 100000, true)
  factor = jl_ns / rp_ns
  @printf("%-18s %10.2f %11.2f %8.2f\n", "int 1-1e5", jl_ns, rp_ns, factor)

  # int 1-2e9
  case_seed = seed === nothing ? rand(RandomDevice(), 0:2^31-1) : seed
  jl_rng = Xoshiro(case_seed)
  rp_rng = rng_create(engine)
  rng_seed!(rp_rng, case_seed)
  jl_ns = time_int_range!(jl_rng, chunk, bench_time, 1, 2000000000, false)
  jl_rng = Xoshiro(case_seed)
  rng_seed!(rp_rng, case_seed)
  rp_ns = time_int_range!(rp_rng, chunk, bench_time, 1, 2000000000, true)
  factor = jl_ns / rp_ns
  @printf("%-18s %10.2f %11.2f %8.2f\n", "int 1-2e9", jl_ns, rp_ns, factor)

  # long long 1-2e9
  case_seed = seed === nothing ? rand(RandomDevice(), 0:2^31-1) : seed
  jl_rng = Xoshiro(case_seed)
  rp_rng = rng_create(engine)
  rng_seed!(rp_rng, case_seed)
  jl_ns = time_long_long_range!(jl_rng, chunk, bench_time, Int64(1), Int64(2000000000), false)
  jl_rng = Xoshiro(case_seed)
  rng_seed!(rp_rng, case_seed)
  rp_ns = time_long_long_range!(rp_rng, chunk, bench_time, Int64(1), Int64(2000000000), true)
  factor = jl_ns / rp_ns
  @printf("%-18s %10.2f %11.2f %8.2f\n", "long long 1-2e9", jl_ns, rp_ns, factor)

  # long long 1-6e18
  case_seed = seed === nothing ? rand(RandomDevice(), 0:2^31-1) : seed
  jl_rng = Xoshiro(case_seed)
  rp_rng = rng_create(engine)
  rng_seed!(rp_rng, case_seed)
  jl_ns = time_long_long_range!(jl_rng, chunk, bench_time, Int64(1), Int64(6000000000000000000), false)
  jl_rng = Xoshiro(case_seed)
  rng_seed!(rp_rng, case_seed)
  rp_ns = time_long_long_range!(rp_rng, chunk, bench_time, Int64(1), Int64(6000000000000000000), true)
  factor = jl_ns / rp_ns
  @printf("%-18s %10.2f %11.2f %8.2f\n", "long long 1-6e18", jl_ns, rp_ns, factor)

  # perm 100
  case_seed = seed === nothing ? rand(RandomDevice(), 0:2^31-1) : seed
  jl_rng = Xoshiro(case_seed)
  rp_rng = rng_create(engine)
  rng_seed!(rp_rng, case_seed)
  jl_ns = time_perm!(jl_rng, bench_time, 100, false)
  jl_rng = Xoshiro(case_seed)
  rng_seed!(rp_rng, case_seed)
  rp_ns = time_perm!(rp_rng, bench_time, 100, true)
  factor = jl_ns / rp_ns
  @printf("%-18s %10.2f %11.2f %8.2f\n", "perm 100", jl_ns, rp_ns, factor)

  # perm 100000
  case_seed = seed === nothing ? rand(RandomDevice(), 0:2^31-1) : seed
  jl_rng = Xoshiro(case_seed)
  rp_rng = rng_create(engine)
  rng_seed!(rp_rng, case_seed)
  jl_ns = time_perm!(jl_rng, bench_time, 100000, false)
  jl_rng = Xoshiro(case_seed)
  rng_seed!(rp_rng, case_seed)
  rp_ns = time_perm!(rp_rng, bench_time, 100000, true)
  factor = jl_ns / rp_ns
  @printf("%-18s %10.2f %11.2f %8.2f\n", "perm 100000", jl_ns, rp_ns, factor)

  # sample 20/1000
  case_seed = seed === nothing ? rand(RandomDevice(), 0:2^31-1) : seed
  jl_rng = Xoshiro(case_seed)
  rp_rng = rng_create(engine)
  rng_seed!(rp_rng, case_seed)
  jl_ns = time_sample!(jl_rng, bench_time, 1000, 20, false)
  jl_rng = Xoshiro(case_seed)
  rng_seed!(rp_rng, case_seed)
  rp_ns = time_sample!(rp_rng, bench_time, 1000, 20, true)
  factor = jl_ns / rp_ns
  @printf("%-18s %10.2f %11.2f %8.2f\n", "sample 20/1000", jl_ns, rp_ns, factor)

  # sample 500/1000
  case_seed = seed === nothing ? rand(RandomDevice(), 0:2^31-1) : seed
  jl_rng = Xoshiro(case_seed)
  rp_rng = rng_create(engine)
  rng_seed!(rp_rng, case_seed)
  jl_ns = time_sample!(jl_rng, bench_time, 1000, 500, false)
  jl_rng = Xoshiro(case_seed)
  rng_seed!(rp_rng, case_seed)
  rp_ns = time_sample!(rp_rng, bench_time, 1000, 500, true)
  factor = jl_ns / rp_ns
  @printf("%-18s %10.2f %11.2f %8.2f\n", "sample 500/1000", jl_ns, rp_ns, factor)

  # sample 980/1000
  case_seed = seed === nothing ? rand(RandomDevice(), 0:2^31-1) : seed
  jl_rng = Xoshiro(case_seed)
  rp_rng = rng_create(engine)
  rng_seed!(rp_rng, case_seed)
  jl_ns = time_sample!(jl_rng, bench_time, 1000, 980, false)
  jl_rng = Xoshiro(case_seed)
  rng_seed!(rp_rng, case_seed)
  rp_ns = time_sample!(rp_rng, bench_time, 1000, 980, true)
  factor = jl_ns / rp_ns
  @printf("%-18s %10.2f %11.2f %8.2f\n", "sample 980/1000", jl_ns, rp_ns, factor)
end

main()
