using Random
using Distributions
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
  chunk = 4096
  bench_time = 0.2
  reps = compute_reps(chunk)

  buf = Vector{Float64}(undef, chunk)
  sink = Ref{Float64}(0.0)

  d_logn = LogNormal(0, 1)
  d_exp1 = Exponential(1)
  d_exp2 = Exponential(2)
  d_gam = Gamma(2, 3)
  d_chi = Chisq(5)
  d_beta = Beta(2, 5)
  d_t = TDist(10)
  d_f = FDist(5, 10)
  d_w = Weibull(2, 1)

  # Warmup: ensure JIT compile and keep CPU "awake" for at least 0.1s
  rand!(buf); consume!(sink, buf)
  randn!(buf); consume!(sink, buf)
  rand!(d_exp1, buf); consume!(sink, buf)
  rand!(d_w, buf); consume!(sink, buf)
  begin
    @inbounds for i in 1:chunk
      buf[i] = 2 + 3 * buf[i]
    end
    consume!(sink, buf)
  end
  warmup!(0.1)

  println("Distribution       ns/value")

  function run(name::String, fill!::Function)
    ns = time_dist!(chunk, reps, bench_time, fill!, sink)
    @printf("%-18s %8.2f\n", name, ns)
  end

  run("u01", () -> begin
    rand!(buf)
    consume!(sink, buf)
  end)

  run("unif(2,5)", () -> begin
    rand!(buf)
    @inbounds @simd for i in 1:chunk
      buf[i] = 2 + 3 * buf[i]
    end
    consume!(sink, buf)
  end)

  run("norm", () -> begin
    randn!(buf)
    consume!(sink, buf)
  end)

  run("normal(2,3)", () -> begin
    randn!(buf)
    @inbounds @simd for i in 1:chunk
      buf[i] = 2 + 3 * buf[i]
    end
    consume!(sink, buf)
  end)

  run("exp(1)", () -> begin
    rand!(d_exp1, buf)
    consume!(sink, buf)
  end)

  run("exp(2)", () -> begin
    rand!(d_exp2, buf)
    consume!(sink, buf)
  end)

  run("lognormal(0,1)", () -> begin
    rand!(d_logn, buf)
    consume!(sink, buf)
  end)

  run("gumbel(0,1)", () -> begin
    rand!(buf)
    @inbounds @simd for i in 1:chunk
      u = buf[i]
      buf[i] = -log(-log(u))
    end
    consume!(sink, buf)
  end)

  run("pareto(1,2)", () -> begin
    rand!(buf)
    @inbounds @simd for i in 1:chunk
      u = buf[i]
      buf[i] = (1 - u)^(-0.5)
    end
    consume!(sink, buf)
  end)

  run("gamma(2,3)", () -> begin
    rand!(d_gam, buf)
    consume!(sink, buf)
  end)

  run("chi2(5)", () -> begin
    rand!(d_chi, buf)
    consume!(sink, buf)
  end)

  run("beta(2,5)", () -> begin
    rand!(d_beta, buf)
    consume!(sink, buf)
  end)

  run("t(10)", () -> begin
    rand!(d_t, buf)
    consume!(sink, buf)
  end)

  run("F(5,10)", () -> begin
    rand!(d_f, buf)
    consume!(sink, buf)
  end)

  run("weibull(2,1)", () -> begin
    rand!(d_w, buf)
    consume!(sink, buf)
  end)

  if sink[] == 123456789
    println("sink")
  end
end

main()
