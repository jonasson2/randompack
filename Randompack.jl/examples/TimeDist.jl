# TimeDist.jl
# Compare base/Distributions vs Randompack (ns/value)

using Random
using Distributions
using Randompack
using Printf

function compute_reps(chunk::Int)
  r = Int(floor(1e6 / chunk))
  return max(1, r)
end

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
      println("Usage: julia TimeDist.jl [engine] [-c chunk]")
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

  reps = compute_reps(chunk)
  buf = Vector{Float64}(undef, chunk)
  sink = Ref{Float64}(0.0)

  rng = rng_create(engine)

  d_logn = LogNormal(0, 1)
  d_unif = Uniform(2, 5)
  d_norm = Normal(2, 3)
  d_exp2 = Exponential(2)
  d_gumbel = Gumbel(0, 1)
  d_pareto = Pareto(1, 2)
  d_gam = Gamma(2, 3)
  d_chi = Chisq(5)
  d_beta = Beta(2, 5)
  d_t = TDist(10)
  d_f = FDist(5, 10)
  d_w = Weibull(2, 1)

  # Warmup: ensure JIT compile and keep CPU "awake" for at least 0.1s
  rand!(buf); consume!(sink, buf)
  randn!(buf); consume!(sink, buf)
  randexp!(buf); consume!(sink, buf)
  rand!(d_gam, buf); consume!(sink, buf)
  random_unif!(rng, buf); consume!(sink, buf)
  warmup!(0.1)

  @printf("Engine: %s\n", engine)
  @printf("%-14s %10s %11s %8s\n", "Distribution", "Base", "Randompack",
          "Factor")

  function run(name::String, fill_base!::Function, fill_rp!::Function)
    base_ns = time_dist!(chunk, reps, bench_time, fill_base!, sink)
    rp_ns = time_dist!(chunk, reps, bench_time, fill_rp!, sink)
    factor = base_ns / rp_ns
    @printf("%-14s %10.2f %11.2f %8.2f\n", name, base_ns, rp_ns, factor)
  end

  run("unif(0,1)", () -> begin
    rand!(buf)
    consume!(sink, buf)
  end, () -> begin
    random_unif!(rng, buf)
    consume!(sink, buf)
  end)

  run("unif(2,5)", () -> begin
    rand!(d_unif, buf)
    consume!(sink, buf)
  end, () -> begin
    random_unif!(rng, buf; a=2, b=5)
    consume!(sink, buf)
  end)

  run("std.normal", () -> begin
    randn!(buf)
    consume!(sink, buf)
  end, () -> begin
    random_normal!(rng, buf)
    consume!(sink, buf)
  end)

  run("normal(2,3)", () -> begin
    rand!(d_norm, buf)
    consume!(sink, buf)
  end, () -> begin
    random_normal!(rng, buf; mu=2, sigma=3)
    consume!(sink, buf)
  end)

  run("std.exp", () -> begin
    randexp!(buf)
    consume!(sink, buf)
  end, () -> begin
    random_exp!(rng, buf; scale=1)
    consume!(sink, buf)
  end)

  run("exp(2)", () -> begin
    rand!(d_exp2, buf)
    consume!(sink, buf)
  end, () -> begin
    random_exp!(rng, buf; scale=2)
    consume!(sink, buf)
  end)

  run("lognormal(0,1)", () -> begin
    rand!(d_logn, buf)
    consume!(sink, buf)
  end, () -> begin
    random_lognormal!(rng, buf; mu=0, sigma=1)
    consume!(sink, buf)
  end)

  run("gumbel(0,1)", () -> begin
    rand!(d_gumbel, buf)
    consume!(sink, buf)
  end, () -> begin
    random_gumbel!(rng, buf; mu=0, beta=1)
    consume!(sink, buf)
  end)

  run("pareto(1,2)", () -> begin
    rand!(d_pareto, buf)
    consume!(sink, buf)
  end, () -> begin
    random_pareto!(rng, buf; xm=1, alpha=2)
    consume!(sink, buf)
  end)

  run("gamma(2,3)", () -> begin
    rand!(d_gam, buf)
    consume!(sink, buf)
  end, () -> begin
    random_gamma!(rng, buf; shape=2, scale=3)
    consume!(sink, buf)
  end)

  run("chi2(5)", () -> begin
    rand!(d_chi, buf)
    consume!(sink, buf)
  end, () -> begin
    random_chi2!(rng, buf; nu=5)
    consume!(sink, buf)
  end)

  run("beta(2,5)", () -> begin
    rand!(d_beta, buf)
    consume!(sink, buf)
  end, () -> begin
    random_beta!(rng, buf; a=2, b=5)
    consume!(sink, buf)
  end)

  run("t(10)", () -> begin
    rand!(d_t, buf)
    consume!(sink, buf)
  end, () -> begin
    random_t!(rng, buf; nu=10)
    consume!(sink, buf)
  end)

  run("F(5,10)", () -> begin
    rand!(d_f, buf)
    consume!(sink, buf)
  end, () -> begin
    random_f!(rng, buf; nu1=5, nu2=10)
    consume!(sink, buf)
  end)

  run("weibull(2,1)", () -> begin
    rand!(d_w, buf)
    consume!(sink, buf)
  end, () -> begin
    random_weibull!(rng, buf; shape=2, scale=1)
    consume!(sink, buf)
  end)

  if sink[] == 123456789
    println("sink")
  end
end

main()
