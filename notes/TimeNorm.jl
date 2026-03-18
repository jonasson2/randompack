# TimeNorm.jl
# Minimal standard normal buffer fill for inspection and disassembly.

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

function base_fill!(buf::Vector{Float64}, sink::Base.RefValue{Float64})
  randn!(buf)
  consume!(sink, buf)
  return nothing
end

function rp_fill!(rng::Randompack.RNG, buf::Vector{Float64},
                  sink::Base.RefValue{Float64})
  random_normal!(rng, buf)
  consume!(sink, buf)
  return nothing
end

function main()
  chunk = 4096
  buf = Vector{Float64}(undef, chunk)
  sink = Ref{Float64}(0.0)
  rng = rng_create("x256++simd")
  base_fill!(buf, sink)
  rp_fill!(rng, buf, sink)
  @printf("Library: %s\n", Randompack._libpath[])
  if sink[] == 123456789
    println("sink")
  end
  return nothing
end

main()
