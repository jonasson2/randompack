# -----------------------------------------------------------------------------
# Creation and seeding
# -----------------------------------------------------------------------------

"""
    rng_create()
    rng_create(engine)

Create a new RNG using the selected engine. The default engine is `x256++simd`. The
`engine` argument is a string naming the RNG engine to use. Call `Randompack.engines()` to
list available engine names.

Use `Randompack.engines()` to list available names.

# Examples
```julia
rng = rng_create()
rng = rng_create("pcg64")
```

# See also
- Randompack.engines
- rng_seed!
"""
function rng_create(engine::AbstractString)
  p = ccall((:randompack_create, _libpath[]), RNGPtr, (Cstring,), engine)
  if p == C_NULL
    if _is_platform_dependent_engine(engine)
      throw(ErrorException("RNG engine '$engine' is not supported on this platform/build"))
    end
    throw(ErrorException("unknown RNG engine '$engine' (check spelling)"))
  end
  err = _last_error(p)
  if err !== nothing
    _free(p)
    if _is_platform_dependent_engine(engine)
      throw(ErrorException("RNG engine '$engine' is not supported on this platform/build"))
    end
    throw(ErrorException("unknown RNG engine '$engine' (check spelling)"))
  end
  rng = RNG(p)
  finalizer(rng) do r
    _free(r.ptr)
    r.ptr = C_NULL
  end
  return rng
end

rng_create() = rng_create("x256++simd")

"""
    rng_seed!(rng, seed)
    rng_seed!(rng, seed; spawn_key)

Seed an RNG with the given integer `seed`. The seed value must fit in a 32-bit signed
integer. The optional `spawn_key` argument is an integer vector used to derive independent
streams. If provided, its values are range-checked and converted to `UInt32` before
calling the C library. Omitting `spawn_key` is equivalent to passing an empty spawn_key.

# Examples
```julia
rng_seed!(rng, 123)
rng_seed!(rng, 123; spawn_key=[1, 2])
```

# See also
- Randompack.serialize
- Randompack.deserialize!
- Randompack.randomize!
"""
function rng_seed!(rng::RNG, seed::Integer; spawn_key=nothing)
  rng.ptr == C_NULL && throw(ErrorException("RNG pointer is NULL"))
  if seed < typemin(Cint) || seed > typemax(Cint)
    throw(ArgumentError("seed out of range for C int (must fit in Int32)"))
  end
  s = Cint(seed)
  if spawn_key === nothing
    ok = ccall((:randompack_seed, _libpath[]), Bool,
               (Cint, Ptr{UInt32}, Cint, RNGPtr),
               s, Ptr{UInt32}(C_NULL), Cint(0), rng.ptr)
    _check_ok(ok, rng.ptr, "randompack_seed failed")
    return
  end
  sk = if spawn_key isa Vector{UInt32}
    spawn_key
  elseif spawn_key isa AbstractVector{<:Integer}
    _u32_vec_from_ints(spawn_key)
  else
    throw(ArgumentError("spawn_key must be an integer vector"))
  end
  n = length(sk)
  if n == 0
    ok = ccall((:randompack_seed, _libpath[]), Bool,
               (Cint, Ptr{UInt32}, Cint, RNGPtr),
               s, Ptr{UInt32}(C_NULL), Cint(0), rng.ptr)
  else
    if n > typemax(Cint)
      throw(ArgumentError("spawn_key too long"))
    end
    ok = GC.@preserve sk begin
      ccall((:randompack_seed, _libpath[]), Bool,
            (Cint, Ptr{UInt32}, Cint, RNGPtr),
            s, pointer(sk), Cint(n), rng.ptr)
    end
  end
  _check_ok(ok, rng.ptr, "randompack_seed failed")
  return
end

