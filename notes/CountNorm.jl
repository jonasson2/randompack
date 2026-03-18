# CountNorm.jl
# Count slow-path events for a fixed number of standard normal draws.

using Randompack
using Printf

function get_counters()
  cas = Ref{Int32}(0)
  cbt = Ref{Int32}(0)
  cc = Ref{Int32}(0)
  cbc = Ref{Int32}(0)
  ctail = Ref{Int32}(0)
  ccall((:randompack_get_counters, Randompack._libpath[]), Cvoid,
        (Ref{Int32}, Ref{Int32}, Ref{Int32}, Ref{Int32}, Ref{Int32}),
        cas, cbt, cc, cbc, ctail)
  return (cas[], cbt[], cc[], cbc[], ctail[])
end

function main()
  n = 10000
  buf = Vector{Float64}(undef, n)
  rng = rng_create("x256++simd")
  c0 = get_counters()
  random_normal!(rng, buf)
  c1 = get_counters()
  cas = c1[1] - c0[1]
  cbt = c1[2] - c0[2]
  cc = c1[3] - c0[3]
  cbc = c1[4] - c0[4]
  ctail = c1[5] - c0[5]
  @printf("Library: %s\n", Randompack._libpath[])
  @printf("Draws: %d\n", n)
  @printf("AboveSecant: %d\n", cas)
  @printf("BelowTangent: %d\n", cbt)
  @printf("AboveCurve: %d\n", cc)
  @printf("BelowCurve: %d\n", cbc)
  @printf("Tail: %d\n", ctail)
  return nothing
end

main()
