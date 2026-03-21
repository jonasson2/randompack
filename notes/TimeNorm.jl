# TimeNorm.jl
# Minimal scaled exponential buffer fill for inspection and disassembly.

using Random
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

function base_fill!(buf::Vector{Float64}, sink::Base.RefValue{Float64}, scale::Float64)
  randexp!(buf)
  @inbounds for i in eachindex(buf)
    buf[i] = scale*buf[i]
  end
  consume!(sink, buf)
  return nothing
end

function rp_fill_exp!(rng::Randompack.RNG, buf::Vector{Float64},
                      sink::Base.RefValue{Float64}, scale::Float64)
  random_exp!(rng, buf; scale=scale)
  consume!(sink, buf)
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

function time_fill!(chunk::Int, reps::Int, bench_time::Float64,
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
  bench_time = 0.3
  scale = 2.0
  reps = max(1, floor(Int, 1e6 / chunk))
  buf = Vector{Float64}(undef, chunk)
  sink = Ref{Float64}(0.0)
  rng = rng_create("x256++simd")
  @printf("Library: %s\n", Randompack._libpath[])
  @printf("%-22s %8s\n", "Method", "ns/value")

  # Warmup
  base_fill!(buf, sink, scale)
  rp_fill_exp!(rng, buf, sink, scale)
  warmup!(0.1)

  ns = time_fill!(chunk, reps, bench_time,
                  () -> base_fill!(buf, sink, scale), sink)
  @printf("%-22s %8.2f\n", "Julia randexp!*", ns)

  ns = time_fill!(chunk, reps, bench_time,
                  () -> rp_fill_exp!(rng, buf, sink, scale), sink)
  @printf("%-22s %8.2f\n", "rp exp* (x256++simd)", ns)

  if sink[] == 123456789
    println("sink")
  end
  return nothing
end

main()
