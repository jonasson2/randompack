module Randompack

if Sys.WORD_SIZE != 64
  error("Randompack requires a 64-bit Julia (Sys.WORD_SIZE == 64)")
end

export rng_create, rng_seed!, random_unif, random_unif!, random_normal,
       random_normal!, random_lognormal, random_lognormal!,
       random_exp, random_exp!, random_gamma, random_gamma!, random_chi2,
       random_chi2!, random_beta, random_beta!, random_t, random_t!, random_f,
       random_f!, random_gumbel, random_gumbel!, random_pareto, random_pareto!,
       random_weibull, random_weibull!, random_mvn, random_mvn!, random_int,
       random_int!, random_perm, random_perm!, random_sample, random_sample!

const RNGPtr = Ptr{Cvoid}
const _LIBSPEC = Ref{String}("")

@inline function _libspec()::String
  s = _LIBSPEC[]
  if isempty(s)
    path = get(ENV, "RANDOMPACK_LIB", "")
    _LIBSPEC[] = isempty(path) ? "librandompack" : path
    s = _LIBSPEC[]
  end
  return s
end

mutable struct RNG
  ptr::RNGPtr
end

function _free(ptr::RNGPtr)
  ptr == C_NULL && return nothing
  ccall((:randompack_free, _libspec()), Cvoid, (RNGPtr,), ptr)
  return nothing
end

function _last_error(ptr::RNGPtr)
  msg = ccall((:randompack_last_error, _libspec()), Cstring, (RNGPtr,), ptr)
  msg == C_NULL && return nothing
  s = unsafe_string(msg)
  return isempty(s) ? nothing : s
end

function _check_ok(ok::Bool, rngptr::RNGPtr, fallback::AbstractString)
  ok && return nothing
  msgp = ccall((:randompack_last_error, _libspec()), Cstring, (RNGPtr,), rngptr)
  if msgp == C_NULL
    throw(ErrorException(fallback))
  end
  msg = unsafe_string(msgp)
  throw(ErrorException(isempty(msg) ? fallback : msg))
end

function _is_platform_dependent_engine(name::AbstractString)::Bool
  return name == "philox" || name == "pcg64" || name == "cwg128"
end

function rng_create(engine::AbstractString)
  p = ccall((:randompack_create, _libspec()), RNGPtr, (Cstring,), engine)
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

function rng_seed!(rng::RNG, seed::Integer; spawn_key::Vector{UInt32}=UInt32[])
  rng.ptr == C_NULL && throw(ErrorException("RNG pointer is NULL"))
  if seed < typemin(Cint) || seed > typemax(Cint)
    throw(ArgumentError("seed out of range for C int (must fit in Int32)"))
  end
  s = Cint(seed)
  n = length(spawn_key)
  ok = if n == 0
    ccall((:randompack_seed, _libspec()), Bool,
          (Cint, Ptr{UInt32}, Cint, RNGPtr),
          s, Ptr{UInt32}(C_NULL), Cint(0), rng.ptr)
  else
    GC.@preserve spawn_key begin
      ccall((:randompack_seed, _libspec()), Bool,
            (Cint, Ptr{UInt32}, Cint, RNGPtr),
            s, pointer(spawn_key), Cint(n), rng.ptr)
    end
  end
  _check_ok(ok, rng.ptr, "randompack_seed failed")
  return nothing
end

function duplicate(rng::RNG)
  rng.ptr == C_NULL && throw(ErrorException("RNG pointer is NULL"))
  p = ccall((:randompack_duplicate, _libspec()), RNGPtr, (RNGPtr,), rng.ptr)
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

function randomize!(rng::RNG)
  rng.ptr == C_NULL && throw(ErrorException("RNG pointer is NULL"))
  ok = ccall((:randompack_randomize, _libspec()), Bool, (RNGPtr,), rng.ptr)
  _check_ok(ok, rng.ptr, "randompack_randomize failed")
  return nothing
end

function full_mantissa!(rng::RNG, enable::Bool=true)
  rng.ptr == C_NULL && throw(ErrorException("RNG pointer is NULL"))
  ok = ccall((:randompack_full_mantissa, _libspec()), Bool, (RNGPtr, Bool),
             rng.ptr, enable)
  _check_ok(ok, rng.ptr, "randompack_full_mantissa failed")
  return nothing
end

function engines()
  n = Ref{Cint}(0)
  eng_max = Ref{Cint}(0)
  desc_max = Ref{Cint}(0)
  ok = ccall((:randompack_engines, _libspec()), Bool,
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
    ccall((:randompack_engines, _libspec()), Bool,
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
  return (engine = names, description = descs)
end

function set_state!(rng::RNG, state::AbstractVector{UInt64})
  rng.ptr == C_NULL && throw(ErrorException("RNG pointer is NULL"))
  n = length(state)
  if n > typemax(Cint)
    throw(ArgumentError("state too long"))
  end
  ok = GC.@preserve state begin
    ccall((:randompack_set_state, _libspec()), Bool, (Ptr{UInt64}, Cint, RNGPtr),
          pointer(state), Cint(n), rng.ptr)
  end
  _check_ok(ok, rng.ptr, "randompack_set_state failed")
  return nothing
end

function set_state!(rng::RNG, state::NTuple{N, UInt64}) where {N}
  return set_state!(rng, collect(state))
end

struct RandompackCounter
  v::NTuple{4, UInt64}
end

struct RandompackPhiloxKey
  v::NTuple{2, UInt64}
end

function philox_set_state!(rng::RNG, counter::NTuple{4, UInt64},
  key::NTuple{2, UInt64})
  rng.ptr == C_NULL && throw(ErrorException("RNG pointer is NULL"))
  ctr = RandompackCounter(counter)
  k = RandompackPhiloxKey(key)
  ok = ccall((:randompack_philox_set_state, _libspec()), Bool,
             (RandompackCounter, RandompackPhiloxKey, RNGPtr), ctr, k, rng.ptr)
  _check_ok(ok, rng.ptr, "randompack_philox_set_state failed")
  return nothing
end

function philox_set_state!(rng::RNG; ctr::NTuple{4, UInt64},
  key::NTuple{2, UInt64})
  return philox_set_state!(rng, ctr, key)
end

function squares_set_state!(rng::RNG, counter::UInt64, key::UInt64)
  rng.ptr == C_NULL && throw(ErrorException("RNG pointer is NULL"))
  ok = ccall((:randompack_squares_set_state, _libspec()), Bool,
             (UInt64, UInt64, RNGPtr), counter, key, rng.ptr)
  _check_ok(ok, rng.ptr, "randompack_squares_set_state failed")
  return nothing
end

function squares_set_state!(rng::RNG; ctr::UInt64, key::UInt64)
  return squares_set_state!(rng, ctr, key)
end

function pcg64_set_state!(rng::RNG, state::UInt128, inc::UInt128)
  rng.ptr == C_NULL && throw(ErrorException("RNG pointer is NULL"))
  ok = ccall((:randompack_pcg64_set_state, _libspec()), Bool,
             (UInt128, UInt128, RNGPtr), state, inc, rng.ptr)
  _check_ok(ok, rng.ptr, "randompack_pcg64_set_state failed")
  return nothing
end

function pcg64_set_state!(rng::RNG; state::UInt128, inc::UInt128)
  return pcg64_set_state!(rng, state, inc)
end

function serialize(rng::RNG)
  rng.ptr == C_NULL && throw(ErrorException("RNG pointer is NULL"))
  lenref = Ref{Cint}(0)
  ok = ccall((:randompack_serialize, _libspec()), Bool, (Ptr{UInt8}, Ptr{Cint},
             RNGPtr), C_NULL, lenref, rng.ptr)
  _check_ok(ok, rng.ptr, "randompack_serialize failed")
  n = lenref[]
  if n < 0
    throw(ErrorException("randompack_serialize returned negative length"))
  end
  buf = Vector{UInt8}(undef, n)
  lenref[] = Cint(n)
  ok = GC.@preserve buf begin
    ccall((:randompack_serialize, _libspec()), Bool, (Ptr{UInt8}, Ptr{Cint},
           RNGPtr), pointer(buf), lenref, rng.ptr)
  end
  _check_ok(ok, rng.ptr, "randompack_serialize failed")
  if lenref[] != n
    resize!(buf, lenref[])
  end
  return buf
end

function deserialize!(rng::RNG, bytes::AbstractVector{UInt8})
  rng.ptr == C_NULL && throw(ErrorException("RNG pointer is NULL"))
  buf = bytes isa StridedVector{UInt8} ? bytes : collect(bytes)
  n = length(buf)
  if n > typemax(Cint)
    throw(ArgumentError("buffer too long"))
  end
  ok = GC.@preserve buf begin
    ccall((:randompack_deserialize, _libspec()), Bool, (Ptr{UInt8}, Cint,
           RNGPtr), pointer(buf), Cint(n), rng.ptr)
  end
  _check_ok(ok, rng.ptr, "randompack_deserialize failed")
  return nothing
end

function random_mvn!(rng::RNG, X::AbstractMatrix{Float64},
  Sigma::AbstractMatrix{<:Real}; mu::Union{Nothing,AbstractVector{<:Real}}=nothing)
  rng.ptr == C_NULL && throw(ErrorException("RNG pointer is NULL"))
  n, d = size(X)
  if n < 0
    throw(ArgumentError("n must be non-negative"))
  end
  d1, d2 = size(Sigma)
  if d1 <= 0 || d2 != d1
    throw(ArgumentError("Sigma must be a non-empty square matrix"))
  end
  if d != d1
    throw(ArgumentError("Sigma dimension must match output matrix"))
  end
  sig = Matrix{Float64}(Sigma)
  muvec = nothing
  if mu !== nothing
    muvec = Vector{Float64}(mu)
    if length(muvec) != d1
      throw(ArgumentError("mu length must match Sigma"))
    end
  end
  transp = Ref{Cchar}('N')
  mu_ptr = muvec === nothing ? Ptr{Float64}(C_NULL) : pointer(muvec)
  call = function(out::StridedMatrix{Float64})
    ok = GC.@preserve sig out muvec begin
      ccall((:randompack_mvn, _libspec()), Bool,
            (Ptr{Cchar}, Ptr{Float64}, Ptr{Float64}, Cint, Csize_t, Ptr{Float64},
             Cint, Ptr{Float64}, RNGPtr),
            transp, mu_ptr, pointer(sig), Cint(d1), Csize_t(n), pointer(out),
            Cint(n), Ptr{Float64}(C_NULL), rng.ptr)
    end
    _check_ok(ok, rng.ptr, "randompack_mvn failed")
    return nothing
  end
  if X isa StridedMatrix{Float64}
    if !(X isa SubArray) || Base.iscontiguous(X)
      call(X)
      return X
    end
  end
  tmp = Matrix{Float64}(undef, n, d)
  call(tmp)
  for j in 1:d, i in 1:n
    @inbounds X[i, j] = tmp[i, j]
  end
  return X
end

function random_mvn(rng::RNG, n::Integer, Sigma::AbstractMatrix{<:Real};
  mu::Union{Nothing,AbstractVector{<:Real}}=nothing)
  X = Array{Float64}(undef, n, size(Sigma, 1))
  random_mvn!(rng, X, Sigma; mu=mu)
  return X
end

const _PermTypes = Union{Int8, Int16, Int32, Int64, UInt8, UInt16, UInt32, UInt64}

function random_perm!(rng::RNG, p::AbstractVector{T}) where {T<:_PermTypes}
  rng.ptr == C_NULL && throw(ErrorException("RNG pointer is NULL"))
  n = length(p)
  if n > typemax(Cint)
    throw(ArgumentError("n too large"))
  end
  if n > typemax(T)
    throw(ArgumentError("n too large for output type"))
  end
  tmp = Vector{Cint}(undef, n)
  ok = GC.@preserve tmp begin
    ccall((:randompack_perm, _libspec()), Bool, (Ptr{Cint}, Cint, RNGPtr),
          pointer(tmp), Cint(n), rng.ptr)
  end
  _check_ok(ok, rng.ptr, "randompack_perm failed")
  for i in 1:n
    @inbounds p[i] = T(tmp[i] + 1)
  end
  return p
end

function random_perm(rng::RNG, n::Integer, ::Type{T}=Int) where {T<:_PermTypes}
  if n < 0
    throw(ArgumentError("n must be non-negative"))
  end
  p = Vector{T}(undef, n)
  random_perm!(rng, p)
  return p
end

function random_sample!(rng::RNG, s::AbstractVector{T}, n::Integer) where {T<:_PermTypes}
  rng.ptr == C_NULL && throw(ErrorException("RNG pointer is NULL"))
  if n < 0
    throw(ArgumentError("n must be non-negative"))
  end
  k = length(s)
  if k > n
    throw(ArgumentError("k must be <= n"))
  end
  if n > typemax(Cint) || k > typemax(Cint)
    throw(ArgumentError("n or k too large"))
  end
  if n > typemax(T)
    throw(ArgumentError("n too large for output type"))
  end
  tmp = Vector{Cint}(undef, k)
  ok = GC.@preserve tmp begin
    ccall((:randompack_sample, _libspec()), Bool, (Ptr{Cint}, Cint, Cint, RNGPtr),
          pointer(tmp), Cint(n), Cint(k), rng.ptr)
  end
  _check_ok(ok, rng.ptr, "randompack_sample failed")
  for i in 1:k
    @inbounds s[i] = T(tmp[i] + 1)
  end
  return s
end

function random_sample(rng::RNG, n::Integer, k::Integer, ::Type{T}=Int) where {T<:_PermTypes}
  if n < 0
    throw(ArgumentError("n must be non-negative"))
  end
  if k < 0
    throw(ArgumentError("k must be non-negative"))
  end
  s = Vector{T}(undef, k)
  random_sample!(rng, s, n)
  return s
end

function _fill_array!(A::AbstractArray{T}, fill_strided!, fill_tmp!) where {T}
  if A isa StridedArray{T}
    if !(A isa SubArray) || Base.iscontiguous(A)
      fill_strided!(A)
      return A
    end
  end
  tmp = Vector{T}(undef, length(A))
  fill_tmp!(tmp)
  i = 1
  for idx in eachindex(A)
    @inbounds A[idx] = tmp[i]
    i += 1
  end
  return A
end

function _alloc_and_fill(rng::RNG, ::Type{T}, fill!, dims::Tuple{Vararg{Int}}) where {T}
  A = Array{T}(undef, dims...)
  fill!(rng, A)
  return A
end

function _scalar_and_fill(rng::RNG, ::Type{T}, fill!) where {T}
  x = Vector{T}(undef, 1)
  fill!(rng, x)
  return x[1]
end

# ------------------------------------------------------------
# Integer draws (unchanged)
# ------------------------------------------------------------

const _IntTypes = Union{Int8, Int16, Int32, Int64, UInt8, UInt16, UInt32, UInt64}

@inline function _uint_type(::Type{Int8});  return UInt8;  end
@inline function _uint_type(::Type{Int16}); return UInt16; end
@inline function _uint_type(::Type{Int32}); return UInt32; end
@inline function _uint_type(::Type{Int64}); return UInt64; end
@inline function _uint_type(::Type{UInt8});  return UInt8;  end
@inline function _uint_type(::Type{UInt16}); return UInt16; end
@inline function _uint_type(::Type{UInt32}); return UInt32; end
@inline function _uint_type(::Type{UInt64}); return UInt64; end

@inline function _uint_fallback(::Type{UInt8});  return "randompack_uint8 failed";  end
@inline function _uint_fallback(::Type{UInt16}); return "randompack_uint16 failed"; end
@inline function _uint_fallback(::Type{UInt32}); return "randompack_uint32 failed"; end
@inline function _uint_fallback(::Type{UInt64}); return "randompack_uint64 failed"; end

function _kernel_uint!(rngptr::RNGPtr, x::Ptr{UInt8}, n::Csize_t, bound::UInt8, fallback::AbstractString)
  ok = ccall((:randompack_uint8, _libspec()), Bool, (Ptr{UInt8}, Csize_t, UInt8, RNGPtr), x, n, bound, rngptr)
  _check_ok(ok, rngptr, fallback)
  return nothing
end

function _kernel_uint!(rngptr::RNGPtr, x::Ptr{UInt16}, n::Csize_t, bound::UInt16, fallback::AbstractString)
  ok = ccall((:randompack_uint16, _libspec()), Bool, (Ptr{UInt16}, Csize_t, UInt16, RNGPtr), x, n, bound, rngptr)
  _check_ok(ok, rngptr, fallback)
  return nothing
end

function _kernel_uint!(rngptr::RNGPtr, x::Ptr{UInt32}, n::Csize_t, bound::UInt32, fallback::AbstractString)
  ok = ccall((:randompack_uint32, _libspec()), Bool, (Ptr{UInt32}, Csize_t, UInt32, RNGPtr), x, n, bound, rngptr)
  _check_ok(ok, rngptr, fallback)
  return nothing
end

function _kernel_uint!(rngptr::RNGPtr, x::Ptr{UInt64}, n::Csize_t, bound::UInt64, fallback::AbstractString)
  ok = ccall((:randompack_uint64, _libspec()), Bool, (Ptr{UInt64}, Csize_t, UInt64, RNGPtr), x, n, bound, rngptr)
  _check_ok(ok, rngptr, fallback)
  return nothing
end

function _range_params(r::AbstractUnitRange, ::Type{T}) where {T<:_IntTypes}
  a = T(first(r))
  b = T(last(r))
  if b < a
    throw(ArgumentError("require a <= b"))
  end
  n = b - a + one(T)
  U = _uint_type(T)
  return a, U(n)
end

function random_int!(rng::RNG, A::AbstractArray{T}) where {T<:_IntTypes}
  rng.ptr == C_NULL && throw(ErrorException("RNG pointer is NULL"))
  U = _uint_type(T)
  fallback = _uint_fallback(U)
  fill_strided! = function(S::StridedArray{T})
    x = reshape(S, :)
    ptr = Base.unsafe_convert(Ptr{U}, pointer(x))
    _kernel_uint!(rng.ptr, ptr, Csize_t(length(x)), zero(U), fallback)
    return nothing
  end
  fill_tmp! = function(tmp::Vector{T})
    ptr = Base.unsafe_convert(Ptr{U}, pointer(tmp))
    _kernel_uint!(rng.ptr, ptr, Csize_t(length(tmp)), zero(U), fallback)
    return nothing
  end
  return _fill_array!(A, fill_strided!, fill_tmp!)
end

function random_int!(rng::RNG, A::AbstractArray{T}, r::AbstractUnitRange) where {T<:_IntTypes}
  rng.ptr == C_NULL && throw(ErrorException("RNG pointer is NULL"))
  a, bound = _range_params(r, T)
  U = _uint_type(T)
  fallback = _uint_fallback(U)
  fill_strided! = function(S::StridedArray{T})
    x = reshape(S, :)
    ptr = Base.unsafe_convert(Ptr{U}, pointer(x))
    _kernel_uint!(rng.ptr, ptr, Csize_t(length(x)), bound, fallback)
    return nothing
  end
  fill_tmp! = function(tmp::Vector{T})
    ptr = Base.unsafe_convert(Ptr{U}, pointer(tmp))
    _kernel_uint!(rng.ptr, ptr, Csize_t(length(tmp)), bound, fallback)
    return nothing
  end
  _fill_array!(A, fill_strided!, fill_tmp!)
  if a != 0
    for idx in eachindex(A)
      @inbounds A[idx] = A[idx] + a
    end
  end
  return A
end

random_int(rng::RNG, ::Type{T}) where {T<:_IntTypes} =
  _scalar_and_fill(rng, T, random_int!)
random_int(rng::RNG, ::Type{T}, dims::Int...) where {T<:_IntTypes} =
  _alloc_and_fill(rng, T, random_int!, dims)
random_int(rng::RNG, ::Type{T}, dims::Tuple{Vararg{Int}}) where {T<:_IntTypes} =
  _alloc_and_fill(rng, T, random_int!, dims)
random_int(rng::RNG, r::AbstractUnitRange{T}) where {T<:_IntTypes} =
  _scalar_and_fill(rng, T, (r_,x)->random_int!(r_, x, r))
random_int(rng::RNG, r::AbstractUnitRange{T}, dims::Int...) where {T<:_IntTypes} =
  _alloc_and_fill(rng, T, (r_,x)->random_int!(r_, x, r), dims)
random_int(rng::RNG, r::AbstractUnitRange{T}, dims::Tuple{Vararg{Int}}) where {T<:_IntTypes} =
  _alloc_and_fill(rng, T, (r_,x)->random_int!(r_, x, r), dims)

# ------------------------------------------------------------
# Float distribution refactor (new)
# ------------------------------------------------------------

const _FTypes = Union{Float64, Float32}

function _fill1!(rng::RNG, A::AbstractArray{Float64}, ::Val{sym},
  p1::Float64, fallback::AbstractString) where {sym}
  rng.ptr == C_NULL && throw(ErrorException("RNG pointer is NULL"))
  fill_strided! = function(S::StridedArray{Float64})
    x = reshape(S, :)
    ok = ccall((sym, _libspec()), Bool, (Ptr{Float64}, Csize_t, Float64, RNGPtr),
               pointer(x), Csize_t(length(x)), p1, rng.ptr)
    _check_ok(ok, rng.ptr, fallback)
    return nothing
  end
  fill_tmp! = function(tmp::Vector{Float64})
    ok = ccall((sym, _libspec()), Bool, (Ptr{Float64}, Csize_t, Float64, RNGPtr),
               pointer(tmp), Csize_t(length(tmp)), p1, rng.ptr)
    _check_ok(ok, rng.ptr, fallback)
    return nothing
  end
  return _fill_array!(A, fill_strided!, fill_tmp!)
end

function _fill1!(rng::RNG, A::AbstractArray{Float32}, ::Val{sym},
  p1::Float32, fallback::AbstractString) where {sym}
  rng.ptr == C_NULL && throw(ErrorException("RNG pointer is NULL"))
  fill_strided! = function(S::StridedArray{Float32})
    x = reshape(S, :)
    ok = ccall((sym, _libspec()), Bool, (Ptr{Float32}, Csize_t, Float32, RNGPtr),
               pointer(x), Csize_t(length(x)), p1, rng.ptr)
    _check_ok(ok, rng.ptr, fallback)
    return nothing
  end
  fill_tmp! = function(tmp::Vector{Float32})
    ok = ccall((sym, _libspec()), Bool, (Ptr{Float32}, Csize_t, Float32, RNGPtr),
               pointer(tmp), Csize_t(length(tmp)), p1, rng.ptr)
    _check_ok(ok, rng.ptr, fallback)
    return nothing
  end
  return _fill_array!(A, fill_strided!, fill_tmp!)
end

function _fill2!(rng::RNG, A::AbstractArray{Float64}, ::Val{sym},
  p1::Float64, p2::Float64, fallback::AbstractString) where {sym}
  rng.ptr == C_NULL && throw(ErrorException("RNG pointer is NULL"))
  fill_strided! = function(S::StridedArray{Float64})
    x = reshape(S, :)
    ok = ccall((sym, _libspec()), Bool,
               (Ptr{Float64}, Csize_t, Float64, Float64, RNGPtr), pointer(x),
               Csize_t(length(x)), p1, p2, rng.ptr)
    _check_ok(ok, rng.ptr, fallback)
    return nothing
  end
  fill_tmp! = function(tmp::Vector{Float64})
    ok = ccall((sym, _libspec()), Bool,
               (Ptr{Float64}, Csize_t, Float64, Float64, RNGPtr), pointer(tmp),
               Csize_t(length(tmp)), p1, p2, rng.ptr)
    _check_ok(ok, rng.ptr, fallback)
    return nothing
  end
  return _fill_array!(A, fill_strided!, fill_tmp!)
end

function _fill2!(rng::RNG, A::AbstractArray{Float32}, ::Val{sym},
  p1::Float32, p2::Float32, fallback::AbstractString) where {sym}
  rng.ptr == C_NULL && throw(ErrorException("RNG pointer is NULL"))
  fill_strided! = function(S::StridedArray{Float32})
    x = reshape(S, :)
    ok = ccall((sym, _libspec()), Bool,
               (Ptr{Float32}, Csize_t, Float32, Float32, RNGPtr), pointer(x),
               Csize_t(length(x)), p1, p2, rng.ptr)
    _check_ok(ok, rng.ptr, fallback)
    return nothing
  end
  fill_tmp! = function(tmp::Vector{Float32})
    ok = ccall((sym, _libspec()), Bool,
               (Ptr{Float32}, Csize_t, Float32, Float32, RNGPtr), pointer(tmp),
               Csize_t(length(tmp)), p1, p2, rng.ptr)
    _check_ok(ok, rng.ptr, fallback)
    return nothing
  end
  return _fill_array!(A, fill_strided!, fill_tmp!)
end

# --- uniform
function random_unif!(rng::RNG, A::AbstractArray{Float64}; a::Real=0, b::Real=1)
  a64 = Float64(a)
  b64 = Float64(b)
  !(a64 < b64) && throw(ArgumentError("require a < b"))
  return _fill2!(rng, A, Val(:randompack_unif), a64, b64, "randompack_unif failed")
end

function random_unif!(rng::RNG, A::AbstractArray{Float32}; a::Real=0, b::Real=1)
  a32 = Float32(a)
  b32 = Float32(b)
  !(a32 < b32) && throw(ArgumentError("require a < b"))
  return _fill2!(rng, A, Val(:randompack_uniff), a32, b32, "randompack_uniff failed")
end

random_unif(rng::RNG, ::Type{Float64}; a::Real=0, b::Real=1) =
  _scalar_and_fill(rng, Float64, (r,x)->random_unif!(r, x; a=a, b=b))
random_unif(rng::RNG, ::Type{Float32}; a::Real=0, b::Real=1) =
  _scalar_and_fill(rng, Float32, (r,x)->random_unif!(r, x; a=a, b=b))
random_unif(rng::RNG, ::Type{T}, dims::Int...; a::Real=0, b::Real=1) where {T<:_FTypes} =
  _alloc_and_fill(rng, T, (r,x)->random_unif!(r, x; a=a, b=b), dims)
random_unif(rng::RNG, ::Type{T}, dims::Tuple{Vararg{Int}}; a::Real=0, b::Real=1) where {T<:_FTypes} =
  _alloc_and_fill(rng, T, (r,x)->random_unif!(r, x; a=a, b=b), dims)

# --- normal
function random_normal!(rng::RNG, A::AbstractArray{Float64}; mu::Real=0, sigma::Real=1)
  mu64 = Float64(mu)
  sig64 = Float64(sigma)
  !(sig64 > 0) && throw(ArgumentError("require sigma > 0"))
  return _fill2!(rng, A, Val(:randompack_normal), mu64, sig64, "randompack_normal failed")
end

function random_normal!(rng::RNG, A::AbstractArray{Float32}; mu::Real=0, sigma::Real=1)
  mu32 = Float32(mu)
  sig32 = Float32(sigma)
  !(sig32 > 0) && throw(ArgumentError("require sigma > 0"))
  return _fill2!(rng, A, Val(:randompack_normalf), mu32, sig32, "randompack_normalf failed")
end

random_normal(rng::RNG, ::Type{Float64}; mu::Real=0, sigma::Real=1) =
  _scalar_and_fill(rng, Float64, (r,x)->random_normal!(r, x; mu=mu, sigma=sigma))
random_normal(rng::RNG, ::Type{Float32}; mu::Real=0, sigma::Real=1) =
  _scalar_and_fill(rng, Float32, (r,x)->random_normal!(r, x; mu=mu, sigma=sigma))
random_normal(rng::RNG, ::Type{T}, dims::Int...; mu::Real=0, sigma::Real=1) where {T<:_FTypes} =
  _alloc_and_fill(rng, T, (r,x)->random_normal!(r, x; mu=mu, sigma=sigma), dims)
random_normal(rng::RNG, ::Type{T}, dims::Tuple{Vararg{Int}}; mu::Real=0, sigma::Real=1) where {T<:_FTypes} =
  _alloc_and_fill(rng, T, (r,x)->random_normal!(r, x; mu=mu, sigma=sigma), dims)

# --- lognormal
function random_lognormal!(rng::RNG, A::AbstractArray{Float64}; mu::Real=0, sigma::Real=1)
  mu64 = Float64(mu)
  sig64 = Float64(sigma)
  !(sig64 > 0) && throw(ArgumentError("require sigma > 0"))
  return _fill2!(rng, A, Val(:randompack_lognormal), mu64, sig64, "randompack_lognormal failed")
end

function random_lognormal!(rng::RNG, A::AbstractArray{Float32}; mu::Real=0, sigma::Real=1)
  mu32 = Float32(mu)
  sig32 = Float32(sigma)
  !(sig32 > 0) && throw(ArgumentError("require sigma > 0"))
  return _fill2!(rng, A, Val(:randompack_lognormalf), mu32, sig32, "randompack_lognormalf failed")
end

random_lognormal(rng::RNG, ::Type{Float64}; mu::Real=0, sigma::Real=1) =
  _scalar_and_fill(rng, Float64, (r,x)->random_lognormal!(r, x; mu=mu, sigma=sigma))
random_lognormal(rng::RNG, ::Type{Float32}; mu::Real=0, sigma::Real=1) =
  _scalar_and_fill(rng, Float32, (r,x)->random_lognormal!(r, x; mu=mu, sigma=sigma))
random_lognormal(rng::RNG, ::Type{T}, dims::Int...; mu::Real=0, sigma::Real=1) where {T<:_FTypes} =
  _alloc_and_fill(rng, T, (r,x)->random_lognormal!(r, x; mu=mu, sigma=sigma), dims)
random_lognormal(rng::RNG, ::Type{T}, dims::Tuple{Vararg{Int}}; mu::Real=0, sigma::Real=1) where {T<:_FTypes} =
  _alloc_and_fill(rng, T, (r,x)->random_lognormal!(r, x; mu=mu, sigma=sigma), dims)

# --- exp
function random_exp!(rng::RNG, A::AbstractArray{Float64}; scale::Real=1)
  sc64 = Float64(scale)
  !(sc64 > 0) && throw(ArgumentError("require scale > 0"))
  return _fill1!(rng, A, Val(:randompack_exp), sc64, "randompack_exp failed")
end

function random_exp!(rng::RNG, A::AbstractArray{Float32}; scale::Real=1)
  sc32 = Float32(scale)
  !(sc32 > 0) && throw(ArgumentError("require scale > 0"))
  return _fill1!(rng, A, Val(:randompack_expf), sc32, "randompack_expf failed")
end

random_exp(rng::RNG, ::Type{Float64}; scale::Real=1) =
  _scalar_and_fill(rng, Float64, (r,x)->random_exp!(r, x; scale=scale))
random_exp(rng::RNG, ::Type{Float32}; scale::Real=1) =
  _scalar_and_fill(rng, Float32, (r,x)->random_exp!(r, x; scale=scale))
random_exp(rng::RNG, ::Type{T}, dims::Int...; scale::Real=1) where {T<:_FTypes} =
  _alloc_and_fill(rng, T, (r,x)->random_exp!(r, x; scale=scale), dims)
random_exp(rng::RNG, ::Type{T}, dims::Tuple{Vararg{Int}}; scale::Real=1) where {T<:_FTypes} =
  _alloc_and_fill(rng, T, (r,x)->random_exp!(r, x; scale=scale), dims)

# --- gamma
function random_gamma!(rng::RNG, A::AbstractArray{Float64}; shape::Real, scale::Real=1)
  sh64 = Float64(shape)
  sc64 = Float64(scale)
  !(sh64 > 0 && sc64 > 0) && throw(ArgumentError("require shape > 0 and scale > 0"))
  return _fill2!(rng, A, Val(:randompack_gamma), sh64, sc64, "randompack_gamma failed")
end

function random_gamma!(rng::RNG, A::AbstractArray{Float32}; shape::Real, scale::Real=1)
  sh32 = Float32(shape)
  sc32 = Float32(scale)
  !(sh32 > 0 && sc32 > 0) && throw(ArgumentError("require shape > 0 and scale > 0"))
  return _fill2!(rng, A, Val(:randompack_gammaf), sh32, sc32, "randompack_gammaf failed")
end

random_gamma(rng::RNG, ::Type{Float64}; shape::Real, scale::Real=1) =
  _scalar_and_fill(rng, Float64, (r,x)->random_gamma!(r, x; shape=shape, scale=scale))
random_gamma(rng::RNG, ::Type{Float32}; shape::Real, scale::Real=1) =
  _scalar_and_fill(rng, Float32, (r,x)->random_gamma!(r, x; shape=shape, scale=scale))
random_gamma(rng::RNG, ::Type{T}, dims::Int...; shape::Real, scale::Real=1) where {T<:_FTypes} =
  _alloc_and_fill(rng, T, (r,x)->random_gamma!(r, x; shape=shape, scale=scale), dims)
random_gamma(rng::RNG, ::Type{T}, dims::Tuple{Vararg{Int}}; shape::Real, scale::Real=1) where {T<:_FTypes} =
  _alloc_and_fill(rng, T, (r,x)->random_gamma!(r, x; shape=shape, scale=scale), dims)

# --- chi2
function random_chi2!(rng::RNG, A::AbstractArray{Float64}; nu::Real)
  nu64 = Float64(nu)
  !(nu64 > 0) && throw(ArgumentError("require nu > 0"))
  return _fill1!(rng, A, Val(:randompack_chi2), nu64, "randompack_chi2 failed")
end

function random_chi2!(rng::RNG, A::AbstractArray{Float32}; nu::Real)
  nu32 = Float32(nu)
  !(nu32 > 0) && throw(ArgumentError("require nu > 0"))
  return _fill1!(rng, A, Val(:randompack_chi2f), nu32, "randompack_chi2f failed")
end

random_chi2(rng::RNG, ::Type{Float64}; nu::Real) =
  _scalar_and_fill(rng, Float64, (r,x)->random_chi2!(r, x; nu=nu))
random_chi2(rng::RNG, ::Type{Float32}; nu::Real) =
  _scalar_and_fill(rng, Float32, (r,x)->random_chi2!(r, x; nu=nu))
random_chi2(rng::RNG, ::Type{T}, dims::Int...; nu::Real) where {T<:_FTypes} =
  _alloc_and_fill(rng, T, (r,x)->random_chi2!(r, x; nu=nu), dims)
random_chi2(rng::RNG, ::Type{T}, dims::Tuple{Vararg{Int}}; nu::Real) where {T<:_FTypes} =
  _alloc_and_fill(rng, T, (r,x)->random_chi2!(r, x; nu=nu), dims)

# --- beta
function random_beta!(rng::RNG, A::AbstractArray{Float64}; a::Real, b::Real)
  a64 = Float64(a)
  b64 = Float64(b)
  !(a64 > 0 && b64 > 0) && throw(ArgumentError("require a > 0 and b > 0"))
  return _fill2!(rng, A, Val(:randompack_beta), a64, b64, "randompack_beta failed")
end

function random_beta!(rng::RNG, A::AbstractArray{Float32}; a::Real, b::Real)
  a32 = Float32(a)
  b32 = Float32(b)
  !(a32 > 0 && b32 > 0) && throw(ArgumentError("require a > 0 and b > 0"))
  return _fill2!(rng, A, Val(:randompack_betaf), a32, b32, "randompack_betaf failed")
end

random_beta(rng::RNG, ::Type{Float64}; a::Real, b::Real) =
  _scalar_and_fill(rng, Float64, (r,x)->random_beta!(r, x; a=a, b=b))
random_beta(rng::RNG, ::Type{Float32}; a::Real, b::Real) =
  _scalar_and_fill(rng, Float32, (r,x)->random_beta!(r, x; a=a, b=b))
random_beta(rng::RNG, ::Type{T}, dims::Int...; a::Real, b::Real) where {T<:_FTypes} =
  _alloc_and_fill(rng, T, (r,x)->random_beta!(r, x; a=a, b=b), dims)
random_beta(rng::RNG, ::Type{T}, dims::Tuple{Vararg{Int}}; a::Real, b::Real) where {T<:_FTypes} =
  _alloc_and_fill(rng, T, (r,x)->random_beta!(r, x; a=a, b=b), dims)

# --- t
function random_t!(rng::RNG, A::AbstractArray{Float64}; nu::Real)
  nu64 = Float64(nu)
  !(nu64 > 0) && throw(ArgumentError("require nu > 0"))
  return _fill1!(rng, A, Val(:randompack_t), nu64, "randompack_t failed")
end

function random_t!(rng::RNG, A::AbstractArray{Float32}; nu::Real)
  nu32 = Float32(nu)
  !(nu32 > 0) && throw(ArgumentError("require nu > 0"))
  return _fill1!(rng, A, Val(:randompack_tf), nu32, "randompack_tf failed")
end

random_t(rng::RNG, ::Type{Float64}; nu::Real) =
  _scalar_and_fill(rng, Float64, (r,x)->random_t!(r, x; nu=nu))
random_t(rng::RNG, ::Type{Float32}; nu::Real) =
  _scalar_and_fill(rng, Float32, (r,x)->random_t!(r, x; nu=nu))
random_t(rng::RNG, ::Type{T}, dims::Int...; nu::Real) where {T<:_FTypes} =
  _alloc_and_fill(rng, T, (r,x)->random_t!(r, x; nu=nu), dims)
random_t(rng::RNG, ::Type{T}, dims::Tuple{Vararg{Int}}; nu::Real) where {T<:_FTypes} =
  _alloc_and_fill(rng, T, (r,x)->random_t!(r, x; nu=nu), dims)

# --- f
function random_f!(rng::RNG, A::AbstractArray{Float64}; nu1::Real, nu2::Real)
  n1 = Float64(nu1)
  n2 = Float64(nu2)
  !(n1 > 0 && n2 > 0) && throw(ArgumentError("require nu1 > 0 and nu2 > 0"))
  return _fill2!(rng, A, Val(:randompack_f), n1, n2, "randompack_f failed")
end

function random_f!(rng::RNG, A::AbstractArray{Float32}; nu1::Real, nu2::Real)
  n1 = Float32(nu1)
  n2 = Float32(nu2)
  !(n1 > 0 && n2 > 0) && throw(ArgumentError("require nu1 > 0 and nu2 > 0"))
  return _fill2!(rng, A, Val(:randompack_ff), n1, n2, "randompack_ff failed")
end

random_f(rng::RNG, ::Type{Float64}; nu1::Real, nu2::Real) =
  _scalar_and_fill(rng, Float64, (r,x)->random_f!(r, x; nu1=nu1, nu2=nu2))
random_f(rng::RNG, ::Type{Float32}; nu1::Real, nu2::Real) =
  _scalar_and_fill(rng, Float32, (r,x)->random_f!(r, x; nu1=nu1, nu2=nu2))
random_f(rng::RNG, ::Type{T}, dims::Int...; nu1::Real, nu2::Real) where {T<:_FTypes} =
  _alloc_and_fill(rng, T, (r,x)->random_f!(r, x; nu1=nu1, nu2=nu2), dims)
random_f(rng::RNG, ::Type{T}, dims::Tuple{Vararg{Int}}; nu1::Real, nu2::Real) where {T<:_FTypes} =
  _alloc_and_fill(rng, T, (r,x)->random_f!(r, x; nu1=nu1, nu2=nu2), dims)

# --- gumbel
function random_gumbel!(rng::RNG, A::AbstractArray{Float64}; mu::Real=0, beta::Real=1)
  mu64 = Float64(mu)
  be64 = Float64(beta)
  !(be64 > 0) && throw(ArgumentError("require beta > 0"))
  return _fill2!(rng, A, Val(:randompack_gumbel), mu64, be64, "randompack_gumbel failed")
end

function random_gumbel!(rng::RNG, A::AbstractArray{Float32}; mu::Real=0, beta::Real=1)
  mu32 = Float32(mu)
  be32 = Float32(beta)
  !(be32 > 0) && throw(ArgumentError("require beta > 0"))
  return _fill2!(rng, A, Val(:randompack_gumbelf), mu32, be32, "randompack_gumbelf failed")
end

random_gumbel(rng::RNG, ::Type{Float64}; mu::Real=0, beta::Real=1) =
  _scalar_and_fill(rng, Float64, (r,x)->random_gumbel!(r, x; mu=mu, beta=beta))
random_gumbel(rng::RNG, ::Type{Float32}; mu::Real=0, beta::Real=1) =
  _scalar_and_fill(rng, Float32, (r,x)->random_gumbel!(r, x; mu=mu, beta=beta))
random_gumbel(rng::RNG, ::Type{T}, dims::Int...; mu::Real=0, beta::Real=1) where {T<:_FTypes} =
  _alloc_and_fill(rng, T, (r,x)->random_gumbel!(r, x; mu=mu, beta=beta), dims)
random_gumbel(rng::RNG, ::Type{T}, dims::Tuple{Vararg{Int}}; mu::Real=0, beta::Real=1) where {T<:_FTypes} =
  _alloc_and_fill(rng, T, (r,x)->random_gumbel!(r, x; mu=mu, beta=beta), dims)

# --- pareto
function random_pareto!(rng::RNG, A::AbstractArray{Float64}; xm::Real, alpha::Real)
  xm64 = Float64(xm)
  al64 = Float64(alpha)
  !(xm64 > 0 && al64 > 0) && throw(ArgumentError("require xm > 0 and alpha > 0"))
  return _fill2!(rng, A, Val(:randompack_pareto), xm64, al64, "randompack_pareto failed")
end

function random_pareto!(rng::RNG, A::AbstractArray{Float32}; xm::Real, alpha::Real)
  xm32 = Float32(xm)
  al32 = Float32(alpha)
  !(xm32 > 0 && al32 > 0) && throw(ArgumentError("require xm > 0 and alpha > 0"))
  return _fill2!(rng, A, Val(:randompack_paretof), xm32, al32, "randompack_paretof failed")
end

random_pareto(rng::RNG, ::Type{Float64}; xm::Real, alpha::Real) =
  _scalar_and_fill(rng, Float64, (r,x)->random_pareto!(r, x; xm=xm, alpha=alpha))
random_pareto(rng::RNG, ::Type{Float32}; xm::Real, alpha::Real) =
  _scalar_and_fill(rng, Float32, (r,x)->random_pareto!(r, x; xm=xm, alpha=alpha))
random_pareto(rng::RNG, ::Type{T}, dims::Int...; xm::Real, alpha::Real) where {T<:_FTypes} =
  _alloc_and_fill(rng, T, (r,x)->random_pareto!(r, x; xm=xm, alpha=alpha), dims)
random_pareto(rng::RNG, ::Type{T}, dims::Tuple{Vararg{Int}}; xm::Real, alpha::Real) where {T<:_FTypes} =
  _alloc_and_fill(rng, T, (r,x)->random_pareto!(r, x; xm=xm, alpha=alpha), dims)

# --- weibull
function random_weibull!(rng::RNG, A::AbstractArray{Float64}; shape::Real, scale::Real=1)
  sh64 = Float64(shape)
  sc64 = Float64(scale)
  !(sh64 > 0 && sc64 > 0) && throw(ArgumentError("require shape > 0 and scale > 0"))
  return _fill2!(rng, A, Val(:randompack_weibull), sh64, sc64, "randompack_weibull failed")
end

function random_weibull!(rng::RNG, A::AbstractArray{Float32}; shape::Real, scale::Real=1)
  sh32 = Float32(shape)
  sc32 = Float32(scale)
  !(sh32 > 0 && sc32 > 0) && throw(ArgumentError("require shape > 0 and scale > 0"))
  return _fill2!(rng, A, Val(:randompack_weibullf), sh32, sc32, "randompack_weibullf failed")
end

random_weibull(rng::RNG, ::Type{Float64}; shape::Real, scale::Real=1) =
  _scalar_and_fill(rng, Float64, (r,x)->random_weibull!(r, x; shape=shape, scale=scale))
random_weibull(rng::RNG, ::Type{Float32}; shape::Real, scale::Real=1) =
  _scalar_and_fill(rng, Float32, (r,x)->random_weibull!(r, x; shape=shape, scale=scale))
random_weibull(rng::RNG, ::Type{T}, dims::Int...; shape::Real, scale::Real=1) where {T<:_FTypes} =
  _alloc_and_fill(rng, T, (r,x)->random_weibull!(r, x; shape=shape, scale=scale), dims)
random_weibull(rng::RNG, ::Type{T}, dims::Tuple{Vararg{Int}}; shape::Real, scale::Real=1) where {T<:_FTypes} =
  _alloc_and_fill(rng, T, (r,x)->random_weibull!(r, x; shape=shape, scale=scale), dims)

end # module
