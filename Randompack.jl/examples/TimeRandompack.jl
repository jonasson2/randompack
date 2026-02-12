# TimeRandompack.jl
# Time randompack distributions in Julia (ns/value)

using Randompack
using Printf

@inline function consume!(sink::Base.RefValue{Float64}, buf::Vector{Float64})
  n = length(buf)
  i0 = 1
  i1 = 1 + (n - 1) ÷ 3
  i2 = 1 + 2 * ((n - 1) ÷ 3)
  i3 = n
  @inbounds sink[] = sink[] + buf[i0] + buf[i1] + buf[i2] + buf[i3]
  return nothing
end

function warmup!(seconds::Float64)
  t0 = time_ns()
  limit = Int(round(seconds * 1e9))
  x = 0.0
  while (time_ns() - t0) < limit
    @inbounds @simd for i in 1:1024
      x = x + i
    end
  end
  if x == -1.0
    println("warmup")
  end
  return nothing
end

function time_dist!(chunk::Int, reps::Int, bench_time::Float64,
                    fill!::Function, sink::Base.RefValue{Float64})
  calls = 0
  t0 = time_ns()
  t = t0
  limit = Int(round(bench_time * 1e9))
  while (t - t0) < limit
    for _ = 1:reps
      fill!()
      calls += 1
    end
    t = time_ns()
  end
  total = calls * chunk
  total == 0 && return 0.0
  return (t - t0) / total
end

function main()
  engine = ""
  chunk = 4096
  bench_time = 0.2
  i = 1
  while i <= length(ARGS)
    arg = ARGS[i]
    if arg == "-h" || arg == "--help"
      println("Usage: julia TimeRandompack.jl [engine] [-c chunk]")
      println("  engine     RNG engine name (default: x256++simd)")
      println("  -c chunk   number of draws per call (default: 4096)")
      return
    elseif arg == "-c" || arg == "--chunk"
      if i == length(ARGS)
        error("missing value for -c")
      end
      i += 1
      chunk = parse(Int, ARGS[i])
      if chunk <= 0
        error("chunk must be positive")
      end
    elseif startswith(arg, "-")
      error("unknown argument: " * arg)
    elseif engine == ""
      engine = arg
    else
      error("unexpected argument: " * arg)
    end
    i += 1
  end
  if isempty(engine)
    engine = "x256++simd"
  end

  reps = max(1, floor(Int, 1e6 / chunk))
  buf = Vector{Float64}(undef, chunk)
  sink = Ref{Float64}(0.0)

  rng = rng_create(engine)

  @printf("Engine: %s\n", engine)
  @printf("%-18s %8s\n", "Distribution", "ns/value")

  # Warmup
  random_unif!(rng, buf); consume!(sink, buf)
  warmup!(0.1)

  ns = time_dist!(chunk, reps, bench_time, () -> begin
    random_unif!(rng, buf)
    consume!(sink, buf)
  end, sink)
  @printf("%-18s %8.2f\n", "u01", ns)

  ns = time_dist!(chunk, reps, bench_time, () -> begin
    random_unif!(rng, buf; a=2, b=5)
    consume!(sink, buf)
  end, sink)
  @printf("%-18s %8.2f\n", "unif(2,5)", ns)

  ns = time_dist!(chunk, reps, bench_time, () -> begin
    random_normal!(rng, buf)
    consume!(sink, buf)
  end, sink)
  @printf("%-18s %8.2f\n", "norm", ns)

  ns = time_dist!(chunk, reps, bench_time, () -> begin
    random_normal!(rng, buf; mu=2, sigma=3)
    consume!(sink, buf)
  end, sink)
  @printf("%-18s %8.2f\n", "normal(2,3)", ns)

  ns = time_dist!(chunk, reps, bench_time, () -> begin
    random_exp!(rng, buf; scale=1)
    consume!(sink, buf)
  end, sink)
  @printf("%-18s %8.2f\n", "exp(1)", ns)

  ns = time_dist!(chunk, reps, bench_time, () -> begin
    random_exp!(rng, buf; scale=2)
    consume!(sink, buf)
  end, sink)
  @printf("%-18s %8.2f\n", "exp(2)", ns)

  ns = time_dist!(chunk, reps, bench_time, () -> begin
    random_lognormal!(rng, buf; mu=0, sigma=1)
    consume!(sink, buf)
  end, sink)
  @printf("%-18s %8.2f\n", "lognormal(0,1)", ns)

  ns = time_dist!(chunk, reps, bench_time, () -> begin
    random_gumbel!(rng, buf; mu=0, beta=1)
    consume!(sink, buf)
  end, sink)
  @printf("%-18s %8.2f\n", "gumbel(0,1)", ns)

  ns = time_dist!(chunk, reps, bench_time, () -> begin
    random_pareto!(rng, buf; xm=1, alpha=2)
    consume!(sink, buf)
  end, sink)
  @printf("%-18s %8.2f\n", "pareto(1,2)", ns)

  ns = time_dist!(chunk, reps, bench_time, () -> begin
    random_gamma!(rng, buf; shape=2, scale=3)
    consume!(sink, buf)
  end, sink)
  @printf("%-18s %8.2f\n", "gamma(2,3)", ns)

  ns = time_dist!(chunk, reps, bench_time, () -> begin
    random_chi2!(rng, buf; nu=5)
    consume!(sink, buf)
  end, sink)
  @printf("%-18s %8.2f\n", "chi2(5)", ns)

  ns = time_dist!(chunk, reps, bench_time, () -> begin
    random_beta!(rng, buf; a=2, b=5)
    consume!(sink, buf)
  end, sink)
  @printf("%-18s %8.2f\n", "beta(2,5)", ns)

  ns = time_dist!(chunk, reps, bench_time, () -> begin
    random_t!(rng, buf; nu=10)
    consume!(sink, buf)
  end, sink)
  @printf("%-18s %8.2f\n", "t(10)", ns)

  ns = time_dist!(chunk, reps, bench_time, () -> begin
    random_f!(rng, buf; nu1=5, nu2=10)
    consume!(sink, buf)
  end, sink)
  @printf("%-18s %8.2f\n", "F(5,10)", ns)

  ns = time_dist!(chunk, reps, bench_time, () -> begin
    random_weibull!(rng, buf; shape=2, scale=1)
    consume!(sink, buf)
  end, sink)
  @printf("%-18s %8.2f\n", "weibull(2,1)", ns)
  if sink[] == 123456789
    println("sink")
  end
end

main()
