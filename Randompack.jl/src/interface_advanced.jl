# -----------------------------------------------------------------------------
# RNG management
# -----------------------------------------------------------------------------

struct EnginesTable
    engine::Vector{String}
    description::Vector{String}
end

Base.haskey(::EnginesTable, key::Symbol) =
  key == :engine || key == :description

"""
    Randompack.engines()

List supported RNG engine names.

The result summarizes each engine with its name and a brief description,
including a short literature reference and bit state size in brackets.

When displayed in the REPL, it is shown as a simple table, but
programmatically, the returned object provides the fields `engine` and
`description`, which are vectors of equal length.

# Examples
```julia
Randompack.engines()      # print the table
e = Randompack.engines()  # capture the result
e.engine[2]               # an individual engine
```
"""
function engines()
  n = Ref{Cint}(0)
  eng_max = Ref{Cint}(0)
  desc_max = Ref{Cint}(0)
  ok = ccall(_sym(:randompack_engines), Bool,
             (Ptr{Cchar}, Ptr{Cchar}, Ptr{Cint}, Ptr{Cint}, Ptr{Cint}),
             C_NULL, C_NULL, n, eng_max, desc_max)
  if !ok
    throw(ErrorException("randompack_engines failed"))
  end
  if n[] < 0 || eng_max[] <= 0 || desc_max[] <= 0
    throw(ErrorException("randompack_engines returned invalid sizes"))
  end
  neng = n[]
  engbuf = Vector{UInt8}(undef, neng * eng_max[])
  descbuf = Vector{UInt8}(undef, neng * desc_max[])
  ok = GC.@preserve engbuf descbuf begin
    ccall(_sym(:randompack_engines), Bool,
          (Ptr{Cchar}, Ptr{Cchar}, Ptr{Cint}, Ptr{Cint}, Ptr{Cint}),
          pointer(engbuf), pointer(descbuf), n, eng_max, desc_max)
  end
  if !ok
    throw(ErrorException("randompack_engines failed"))
  end
  names = Vector{String}(undef, neng)
  descs = Vector{String}(undef, neng)
  for i in 0:(neng-1)
    names[i+1] = unsafe_string(Ptr{UInt8}(pointer(engbuf)) + i*eng_max[])
    descs[i+1] = unsafe_string(Ptr{UInt8}(pointer(descbuf)) + i*desc_max[])
  end
  return EnginesTable(names, descs)
end

function Base.show(io::IO, ::MIME"text/plain", t::EnginesTable)
    w = maximum(length, t.engine)
    println(io, rpad("engine", w), "  description")
    for i in eachindex(t.engine)
        println(io,
                rpad(t.engine[i], w), "  ",
                t.description[i])
    end
end


"""
    Randompack.duplicate(rng::RNG)

Return a new RNG with identical engine and state. The two RNGs then evolve
independently.

# Examples
```julia
rng2 = Randompack.duplicate(rng1)
```

# See also
- rng_create
- rng_seed!
- Randompack.randomize!
"""
function duplicate(rng::RNG)
  rng.ptr == C_NULL && throw(ErrorException("RNG pointer is NULL"))
  p = ccall(_sym(:randompack_duplicate), RNGPtr, (RNGPtr,), rng.ptr)
  if p == C_NULL
    msg = _last_error(rng.ptr)
    throw(ErrorException(msg === nothing ? "randompack_duplicate failed" : msg))
  end
  out = RNG(p)
  finalizer(out) do r
    _free(r.ptr)
    r.ptr = C_NULL
  end
  return out
end

"""
    Randompack.randomize!(rng::RNG)

Randomize RNG state from system entropy. The engine type is preserved.

# Examples
```julia
Randompack.randomize!(rng)
```

# See also
- rng_create
- rng_seed!
- Randompack.duplicate

"""
function randomize!(rng::RNG)
  rng.ptr == C_NULL && throw(ErrorException("RNG pointer is NULL"))
  ok = ccall(_sym(:randompack_randomize), Bool, (RNGPtr,), rng.ptr)
  _check_ok(ok, rng.ptr, "randompack_randomize failed")
  return nothing
end

"""
    Randompack.jump!(rng::RNG, p::Integer)

Jump an xor-family engine ahead by 2^p steps. The `x128+` and `xoro128++`
engines support `p = 32, 64, 96`, while `x256++`, `x256**`, and `x256++simd`
also support `p = 128` and `p = 192`.
"""
function jump!(rng::RNG, p::Integer)
  rng.ptr == C_NULL && throw(ErrorException("RNG pointer is NULL"))
  ok = ccall(_sym(:randompack_jump), Bool, (Cint, RNGPtr), p, rng.ptr)
  _check_ok(ok, rng.ptr, "randompack_jump failed")
  return nothing
end

"""
    Randompack.advance!(rng::RNG, delta)

Advance a `pcg64` engine by an arbitrary 128-bit delta.

Values are range-checked and converted to `UInt64` before calling the C
library. `delta` may be a scalar integer in `[0, 2^128 - 1]`, or a
length-2 vector `[low, high]` of 64-bit words.

# Examples
```julia
Randompack.advance!(rng, 1024)
Randompack.advance!(rng, big(2)^96)
Randompack.advance!(rng, [1024, 0])
```

"""
function advance!(rng::RNG, delta::Integer)
  rng.ptr == C_NULL && throw(ErrorException("RNG pointer is NULL"))
  d = _u128_scalar_checked(delta)
  c_delta = UInt64[UInt64(d % (UInt128(1) << 64)), UInt64(d >> 64)]
  ok = ccall(_sym(:randompack_advance), Bool, (Ptr{UInt64}, RNGPtr),
             c_delta, rng.ptr)
  _check_ok(ok, rng.ptr, "randompack_advance failed")
  return nothing
end

function advance!(rng::RNG, delta::AbstractVector{<:Integer})
  rng.ptr == C_NULL && throw(ErrorException("RNG pointer is NULL"))
  length(delta) == 2 || throw(ArgumentError("delta must have length 2"))
  c_delta = _u64_vec_from_ints(delta)
  ok = ccall(_sym(:randompack_advance), Bool,
             (Ptr{UInt64}, RNGPtr), c_delta, rng.ptr)
  _check_ok(ok, rng.ptr, "randompack_advance failed")
  return nothing
end

# -----------------------------------------------------------------------------
# State setting
# -----------------------------------------------------------------------------

"""
    Randompack.set_state!(rng::RNG, state::AbstractVector{<:Integer})

Set the raw engine state from a vector of integer words.

`state` may have any integer element type. Values are range-checked and then
converted to `UInt64` before calling the underlying C library.

The required length and interpretation of `state` depend on the selected RNG
engine (see `Randompack.engines()` for available engine names and state sizes).

# Examples
```julia
Randompack.set_state!(rng, [1, 2, 3, 4])         # converts Int to UInt64
Randompack.set_state!(rng, UInt64[1, 2, 3, 4])   # no conversion needed
```
"""
function set_state!(rng::RNG, state::AbstractVector{<:Integer})
  if state isa AbstractVector{UInt64}
    return _set_state_u64!(rng, state)
  end
  buf = _u64_vec_from_ints(state)
  return _set_state_u64!(rng, buf)
end

function _set_state_u64!(rng::RNG, state::AbstractVector{UInt64})
  rng.ptr == C_NULL && throw(ErrorException("RNG pointer is NULL"))
  n = length(state)
  if n > typemax(Cint)
    throw(ArgumentError("state too long"))
  end
  ok = GC.@preserve state begin
    ccall(_sym(:randompack_set_state), Bool, (Ptr{UInt64}, Cint, RNGPtr),
          pointer(state), Cint(n), rng.ptr)
  end
  _check_ok(ok, rng.ptr, "randompack_set_state failed")
  return nothing
end

"""
    Randompack.philox_set_key!(rng::RNG, key::AbstractVector{<:Integer})

Set the Philox key directly.
"""
function philox_set_key!(rng::RNG, key::AbstractVector{<:Integer})
  rng.ptr == C_NULL && throw(ErrorException("RNG pointer is NULL"))
  length(key) == 2 || throw(ArgumentError("philox key must have length 2"))
  keyv = _u64_vec_from_ints(key)
  ok = ccall(_sym(:randompack_philox_set_key), Bool,
             (Ptr{UInt64}, RNGPtr), keyv, rng.ptr)
  _check_ok(ok, rng.ptr, "randompack_philox_set_key failed")
  return nothing
end

"""
    Randompack.squares_set_key!(rng::RNG, key::Integer)

Set the Squares64 key directly.

Values are range-checked and converted to `UInt64` before calling the C
library. Throws if the RNG is not a squares engine.

# Examples
```julia
Randompack.squares_set_key!(rng, 4)
```

"""
function squares_set_key!(rng::RNG, key::Integer)
  rng.ptr == C_NULL && throw(ErrorException("RNG pointer is NULL"))
  k64 = _u64_scalar_checked(key)
  ok = ccall(_sym(:randompack_squares_set_key), Bool,
             (UInt64, RNGPtr), k64, rng.ptr)
  _check_ok(ok, rng.ptr, "randompack_squares_set_key failed")
  return nothing
end

"""
    Randompack.pcg64_set_inc!(rng::RNG, inc)

Set the 128-bit PCG64 increment.

Values are range-checked and converted to `UInt64` before calling the C
library. `inc` must be a length-2 vector `[low, high]` and the low word must
be odd. Throws if the RNG is not a pcg64 engine.

# Examples
```julia
Randompack.pcg64_set_inc!(rng, [3, 5])
```

"""
function pcg64_set_inc!(rng::RNG, inc)
  rng.ptr == C_NULL && throw(ErrorException("RNG pointer is NULL"))
  length(inc) == 2 || throw(ArgumentError("inc must have length 2"))
  c_inc = _u64_vec_from_ints(inc)
  ok = ccall(_sym(:randompack_pcg64_set_inc), Bool,
             (Ptr{UInt64}, RNGPtr), c_inc, rng.ptr)
  _check_ok(ok, rng.ptr, "randompack_pcg64_set_inc failed")
  return nothing
end

"""
    Randompack.cwg128_set_weyl!(rng::RNG, weyl)

Set the 128-bit CWG128 Weyl increment.

Values are range-checked and converted to `UInt64` before calling the C
library. `weyl` must be a length-2 vector `[low, high]` and the low word must
be odd.

# Examples
```julia
Randompack.cwg128_set_weyl!(rng, [3, 5])
```

"""
function cwg128_set_weyl!(rng::RNG, weyl)
  rng.ptr == C_NULL && throw(ErrorException("RNG pointer is NULL"))
  length(weyl) == 2 || throw(ArgumentError("weyl must have length 2"))
  c_weyl = _u64_vec_from_ints(weyl)
  ok = ccall(_sym(:randompack_cwg128_set_weyl), Bool,
             (Ptr{UInt64}, RNGPtr), c_weyl, rng.ptr)
  _check_ok(ok, rng.ptr, "randompack_cwg128_set_weyl failed")
  return nothing
end

"""
    Randompack.sfc64_set_abc!(rng::RNG, abc)

Set the `sfc64` `a`, `b`, `c` state words directly, leaving the counter
unchanged.

Values are range-checked and converted to `UInt64` before calling the C
library. `abc` must be a length-3 vector `[a, b, c]`.

# Examples
```julia
Randompack.sfc64_set_abc!(rng, [7, 11, 13])
```

"""
function sfc64_set_abc!(rng::RNG, abc)
  rng.ptr == C_NULL && throw(ErrorException("RNG pointer is NULL"))
  length(abc) == 3 || throw(ArgumentError("abc must have length 3"))
  c_abc = _u64_vec_from_ints(abc)
  ok = ccall(_sym(:randompack_sfc64_set_abc), Bool,
             (Ptr{UInt64}, RNGPtr), c_abc, rng.ptr)
  _check_ok(ok, rng.ptr, "randompack_sfc64_set_abc failed")
  return nothing
end

"""
    Randompack.chacha_set_nonce!(rng::RNG, nonce)

Set the 96-bit ChaCha20 nonce.

Values are range-checked and converted to `UInt32` before calling the C
library. `nonce` must be a length-3 vector `[n0, n1, n2]`.

# Examples
```julia
Randompack.chacha_set_nonce!(rng, [7, 11, 13])
```

"""
function chacha_set_nonce!(rng::RNG, nonce)
  rng.ptr == C_NULL && throw(ErrorException("RNG pointer is NULL"))
  length(nonce) == 3 || throw(ArgumentError("nonce must have length 3"))
  c_nonce = _u32_vec_from_ints(nonce)
  ok = ccall(_sym(:randompack_chacha_set_nonce), Bool,
             (Ptr{UInt32}, RNGPtr), c_nonce, rng.ptr)
  _check_ok(ok, rng.ptr, "randompack_chacha_set_nonce failed")
  return nothing
end

# -----------------------------------------------------------------------------
# Serialization
# -----------------------------------------------------------------------------

"""
    Randompack.serialize(rng::RNG)

Serialize RNG state to a byte vector.

The result can be restored with `Randompack.deserialize!`.

# Examples
```julia
state = Randompack.serialize(rng)
```

# See also
- Randompack.deserialize!
"""
function serialize(rng::RNG)
  rng.ptr == C_NULL && throw(ErrorException("RNG pointer is NULL"))
  lenref = Ref{Cint}(0)
  ok = ccall(_sym(:randompack_serialize), Bool, (Ptr{UInt8}, Ptr{Cint},
             RNGPtr), C_NULL, lenref, rng.ptr)
  _check_ok(ok, rng.ptr, "randompack_serialize failed")
  n = lenref[]
  if n < 0
    throw(ErrorException("randompack_serialize returned negative length"))
  end
  buf = Vector{UInt8}(undef, n)
  lenref[] = Cint(n)
  ok = GC.@preserve buf begin
    ccall(_sym(:randompack_serialize), Bool, (Ptr{UInt8}, Ptr{Cint},
           RNGPtr), pointer(buf), lenref, rng.ptr)
  end
  _check_ok(ok, rng.ptr, "randompack_serialize failed")
  if lenref[] != n
    resize!(buf, lenref[])
  end
  return buf
end

"""
    Randompack.deserialize!(rng::RNG, bytes::AbstractVector{UInt8})

Restore RNG state from a byte vector, typically obtained by
`Randompack.serialize`.

# Examples
```julia
Randompack.deserialize!(rng, state)
```

# See also
- Randompack.serialize

"""
function deserialize!(rng::RNG, bytes::AbstractVector{UInt8})
  rng.ptr == C_NULL && throw(ErrorException("RNG pointer is NULL"))
  buf = bytes isa StridedVector{UInt8} ? bytes : collect(bytes)
  n = length(buf)
  if n > typemax(Cint)
    throw(ArgumentError("buffer too long"))
  end
  ok = GC.@preserve buf begin
    ccall(_sym(:randompack_deserialize), Bool, (Ptr{UInt8}, Cint,
           RNGPtr), pointer(buf), Cint(n), rng.ptr)
  end
  _check_ok(ok, rng.ptr, "randompack_deserialize failed")
  return nothing
end
