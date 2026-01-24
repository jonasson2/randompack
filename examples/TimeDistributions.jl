using Random
using Distributions
using Printf

function compute_reps(chunk::Int)
  r = Int(floor(1e6 / chunk))
  return max(1, r)
end

function time_dist!(chunk::Int, reps::Int, bench_time::Float64, fill!::Function)
  calls = 0
  t0 = time_ns()
  t = t0
  limit = Int(round(bench_time*1e9))
  while (t - t0) < limit
    for _ = 1:reps
      fill!()
    end
    calls += reps
    t = time_ns()
  end
  total = calls*chunk
  total == 0 && return 0.0
  return (t - t0) / total
end

function main()
  chunk = 4096
  bench_time = 0.2
  reps = compute_reps(chunk)

  rng = MersenneTwister(7)
  buf = Vector{Float64}(undef, chunk)
  sink = Ref{Float64}(0.0)

  # Pre-create distributions (no allocations inside timing)
  d_logn = LogNormal(0, 1)
  d_exp1 = Exponential(1)
  d_exp2 = Exponential(2)
  d_gam = Gamma(2, 3)
  d_chi = Chisq(5)
  d_beta = Beta(2, 5)
  d_t = TDist(10)
  d_f = FDist(5, 10)
  d_w = Weibull(2, 1)

  # Warmup + JIT compile all paths once
  rand!(rng, buf)
  randn!(rng, buf)
  rand!(rng, d_exp1, buf)
  rand!(rng, d_w, buf)
  begin
    @inbounds for i in 1:chunk
      buf[i] = 2 + 3*buf[i]
    end
    sink[] = buf[end]
  end

  println("Distribution       ns/value")

  function run(name::String, fill!::Function)
    ns = time_dist!(chunk, reps, bench_time, fill!)
    @printf("%-18s %8.2f\n", name, ns)
  end

  run("u01", () -> begin
    rand!(rng, buf)
    sink[] = buf[end]
  end)

  run("unif(2,5)", () -> begin
    rand!(rng, buf)
    @inbounds @simd for i in 1:chunk
      buf[i] = 2 + 3*buf[i]
    end
    sink[] = buf[end]
  end)

  run("norm", () -> begin
    randn!(rng, buf)
    sink[] = buf[end]
  end)

  run("normal(2,3)", () -> begin
    randn!(rng, buf)
    @inbounds @simd for i in 1:chunk
      buf[i] = 2 + 3*buf[i]
    end
    sink[] = buf[end]
  end)

  run("lognormal(0,1)", () -> begin
    rand!(rng, d_logn, buf)
    sink[] = buf[end]
  end)

  run("gumbel(0,1)", () -> begin
    rand!(rng, buf)
    @inbounds @simd for i in 1:chunk
      u = buf[i]
      buf[i] = -log(-log(u))
    end
    sink[] = buf[end]
  end)

  run("pareto(1,2)", () -> begin
    rand!(rng, buf)
    @inbounds @simd for i in 1:chunk
      u = buf[i]
      buf[i] = (1 - u)^(-0.5)
    end
    sink[] = buf[end]
  end)

  run("exp(1)", () -> begin
    rand!(rng, d_exp1, buf)
    sink[] = buf[end]
  end)

  run("exp(2)", () -> begin
    rand!(rng, d_exp2, buf)
    sink[] = buf[end]
  end)

  run("gamma(2,3)", () -> begin
    rand!(rng, d_gam, buf)
    sink[] = buf[end]
  end)

  run("chi2(5)", () -> begin
    rand!(rng, d_chi, buf)
    sink[] = buf[end]
  end)

  run("beta(2,5)", () -> begin
    rand!(rng, d_beta, buf)
    sink[] = buf[end]
  end)

  run("t(10)", () -> begin
    rand!(rng, d_t, buf)
    sink[] = buf[end]
  end)

  run("F(5,10)", () -> begin
    rand!(rng, d_f, buf)
    sink[] = buf[end]
  end)

  run("weibull(2,1)", () -> begin
    rand!(rng, d_w, buf)
    sink[] = buf[end]
  end)

  if sink[] == 123456789
    println("sink")
  end
end

main()
