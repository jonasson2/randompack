"""
Randompack.jl - Bit-identical random number generation across platforms, with
multiple engines

Source and documentation: https://github.com/jonasson2/randompack

Creation and seeding
- rng_create
- rng_seed!

Distributions (all exist also as mutators: random_unif! ...]
- random_unif
- random_normal
- random_skew_normal
- random_lognormal
- random_exp
- random_gamma
- random_chi2
- random_beta
- random_t
- random_f
- random_gumbel
- random_pareto
- random_weibull
- random_mvn
- random_int
- random_perm
- random_sample

Advanced / state-control
- Randompack.engines
- Randompack.duplicate
- Randompack.randomize!
- Randompack.full_mantissa!
- Randompack.jump!
- Randompack.set_state!
- Randompack.philox_set_state!
- Randompack.squares_set_state!
- Randompack.pcg64_set_inc!
- Randompack.serialize
- Randompack.deserialize!
"""
module Randompack

if Sys.WORD_SIZE != 64
  error("Randompack requires a 64-bit Julia (Sys.WORD_SIZE == 64)")
end

export rng_create, rng_seed!, random_unif, random_unif!, random_normal,
       random_normal!, random_skew_normal, random_skew_normal!,
       random_lognormal, random_lognormal!,
       random_exp, random_exp!, random_gamma, random_gamma!, random_chi2,
       random_chi2!, random_beta, random_beta!, random_t, random_t!, random_f,
       random_f!, random_gumbel, random_gumbel!, random_pareto, random_pareto!,
       random_weibull, random_weibull!, random_mvn, random_mvn!, random_int,
       random_int!, random_perm, random_perm!, random_sample, random_sample!

const RNGPtr = Ptr{Cvoid}

mutable struct RNG
  ptr::RNGPtr
end

include("find_library.jl")
include("helpers.jl")
include("interface_base.jl")
include("interface_continuous.jl")
include("interface_discrete.jl")
include("interface_advanced.jl")

@static if VERSION >= v"1.11"
  Base.eval(@__MODULE__, Expr(:public,
    :engines, :duplicate, :randomize!, :full_mantissa!, :set_state!,
    :jump!, :philox_set_state!, :squares_set_state!, :pcg64_set_inc!,
    :serialize, :deserialize!
  ))
end

end # module
