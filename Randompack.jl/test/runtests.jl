using Test
using Randompack

@testset "Randompack" begin
  include("rng_and_state.jl")
  include("distributions.jl")
end
