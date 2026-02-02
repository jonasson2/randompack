function _free(ptr::RNGPtr)
  ptr == C_NULL && return nothing
  ccall((:randompack_free, _libpath[]), Cvoid, (RNGPtr,), ptr)
  return nothing
end

function _last_error(ptr::RNGPtr)
  msg = ccall((:randompack_last_error, _libpath[]), Cstring, (RNGPtr,), ptr)
  msg == C_NULL && return nothing
  s = unsafe_string(msg)
  return isempty(s) ? nothing : s
end

function _check_ok(ok::Bool, rngptr::RNGPtr, fallback::AbstractString)
  ok && return nothing
  msgp = ccall((:randompack_last_error, _libpath[]), Cstring, (RNGPtr,), rngptr)
  if msgp == C_NULL
    throw(ErrorException(fallback))
  end
  msg = unsafe_string(msgp)
  throw(ErrorException(isempty(msg) ? fallback : msg))
end

function _is_platform_dependent_engine(name::AbstractString)::Bool
  return name == "philox" || name == "pcg64" || name == "cwg128"
end

# -----------------------------------------------------------------------------
# Internal integer-conversion helpers with bounds checks.
#
# Rules:
# - If the input element type is signed, each element must fit in the
#   corresponding signed width (Int32/Int64/Int128).
# - If the input element type is unsigned, each element must fit in the
#   corresponding unsigned width (UInt32/UInt64/UInt128).
# - On success, values are converted to the unsigned type (UIntN) before
#   calling the C API. (Signed negatives are converted via UIntN(x).)
#
# These helpers are used by state-setters and spawn-key conversion.
# -----------------------------------------------------------------------------

@inline function _fail_range_uN(i::Int, v::Integer, T::Type)
  throw(ArgumentError("value out of range for $(T): x[$i] = $v"))
end

@inline function _fail_range_scalar_uN(v::Integer, T::Type)
  throw(ArgumentError("value out of range for $(T): x = $v"))
end

function _u32_vec_from_ints(x::AbstractVector{<:Integer})
  n = length(x)
  y = Vector{UInt32}(undef, n)
  lo = typemin(Int32)
  hi = typemax(Int32)
  uhi = typemax(UInt32)
  @inbounds for i in 1:n
    v = x[i]
    if v isa Signed
      if v < lo || v > hi
        _fail_range_uN(i, v, UInt32)
      end
    else
      if v > uhi
        _fail_range_uN(i, v, UInt32)
      end
    end
    y[i] = UInt32(v)
  end
  return y
end

function _u64_vec_from_ints(x::AbstractVector{<:Integer})
  n = length(x)
  y = Vector{UInt64}(undef, n)
  lo = typemin(Int64)
  hi = typemax(Int64)
  uhi = typemax(UInt64)
  @inbounds for i in 1:n
    v = x[i]
    if v isa Signed
      if v < lo || v > hi
        _fail_range_uN(i, v, UInt64)
      end
    else
      if v > uhi
        _fail_range_uN(i, v, UInt64)
      end
    end
    y[i] = UInt64(v)
  end
  return y
end

@inline function _u64_scalar_checked(v::Integer)
  if v isa Signed
    lo = typemin(Int64)
    hi = typemax(Int64)
    if v < lo || v > hi
      _fail_range_scalar_uN(v, UInt64)
    end
  else
    hi = typemax(UInt64)
    if v > hi
      _fail_range_scalar_uN(v, UInt64)
    end
  end
  return UInt64(v)
end

@inline function _u128_scalar_checked(v::Integer)
  if v isa Signed
    lo = typemin(Int128)
    hi = typemax(Int128)
    if v < lo || v > hi
      _fail_range_scalar_uN(v, UInt128)
    end
  else
    hi = typemax(UInt128)
    if v > hi
      _fail_range_scalar_uN(v, UInt128)
    end
  end
  return UInt128(v)
end

struct _PhiloxCtr
  v::NTuple{4, UInt64}
end

struct _PhiloxKey
  v::NTuple{2, UInt64}
end
const _PermTypes = Union{Int8, Int16, Int32, Int64, UInt8, UInt16, UInt32, UInt64}
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
  ok = ccall((:randompack_uint8, _libpath[]), Bool, (Ptr{UInt8}, Csize_t, UInt8, RNGPtr), x, n, bound, rngptr)
  _check_ok(ok, rngptr, fallback)
  return nothing
end

function _kernel_uint!(rngptr::RNGPtr, x::Ptr{UInt16}, n::Csize_t, bound::UInt16, fallback::AbstractString)
  ok = ccall((:randompack_uint16, _libpath[]), Bool, (Ptr{UInt16}, Csize_t, UInt16, RNGPtr), x, n, bound, rngptr)
  _check_ok(ok, rngptr, fallback)
  return nothing
end

function _kernel_uint!(rngptr::RNGPtr, x::Ptr{UInt32}, n::Csize_t, bound::UInt32, fallback::AbstractString)
  ok = ccall((:randompack_uint32, _libpath[]), Bool, (Ptr{UInt32}, Csize_t, UInt32, RNGPtr), x, n, bound, rngptr)
  _check_ok(ok, rngptr, fallback)
  return nothing
end

function _kernel_uint!(rngptr::RNGPtr, x::Ptr{UInt64}, n::Csize_t, bound::UInt64, fallback::AbstractString)
  ok = ccall((:randompack_uint64, _libpath[]), Bool, (Ptr{UInt64}, Csize_t, UInt64, RNGPtr), x, n, bound, rngptr)
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
const _FTypes = Union{Float64, Float32}

function _fill1!(rng::RNG, A::AbstractArray{Float64}, ::Val{sym},
  p1::Float64, fallback::AbstractString) where {sym}
  rng.ptr == C_NULL && throw(ErrorException("RNG pointer is NULL"))
  fill_strided! = function(S::StridedArray{Float64})
    x = reshape(S, :)
    ok = ccall((sym, _libpath[]), Bool, (Ptr{Float64}, Csize_t, Float64, RNGPtr),
               pointer(x), Csize_t(length(x)), p1, rng.ptr)
    _check_ok(ok, rng.ptr, fallback)
    return nothing
  end
  fill_tmp! = function(tmp::Vector{Float64})
    ok = ccall((sym, _libpath[]), Bool, (Ptr{Float64}, Csize_t, Float64, RNGPtr),
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
    ok = ccall((sym, _libpath[]), Bool, (Ptr{Float32}, Csize_t, Float32, RNGPtr),
               pointer(x), Csize_t(length(x)), p1, rng.ptr)
    _check_ok(ok, rng.ptr, fallback)
    return nothing
  end
  fill_tmp! = function(tmp::Vector{Float32})
    ok = ccall((sym, _libpath[]), Bool, (Ptr{Float32}, Csize_t, Float32, RNGPtr),
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
    ok = ccall((sym, _libpath[]), Bool,
               (Ptr{Float64}, Csize_t, Float64, Float64, RNGPtr), pointer(x),
               Csize_t(length(x)), p1, p2, rng.ptr)
    _check_ok(ok, rng.ptr, fallback)
    return nothing
  end
  fill_tmp! = function(tmp::Vector{Float64})
    ok = ccall((sym, _libpath[]), Bool,
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
    ok = ccall((sym, _libpath[]), Bool,
               (Ptr{Float32}, Csize_t, Float32, Float32, RNGPtr), pointer(x),
               Csize_t(length(x)), p1, p2, rng.ptr)
    _check_ok(ok, rng.ptr, fallback)
    return nothing
  end
  fill_tmp! = function(tmp::Vector{Float32})
    ok = ccall((sym, _libpath[]), Bool,
               (Ptr{Float32}, Csize_t, Float32, Float32, RNGPtr), pointer(tmp),
               Csize_t(length(tmp)), p1, p2, rng.ptr)
    _check_ok(ok, rng.ptr, fallback)
    return nothing
  end
  return _fill_array!(A, fill_strided!, fill_tmp!)
end

function _fill3!(rng::RNG, A::AbstractArray{Float64}, ::Val{sym},
  p1::Float64, p2::Float64, p3::Float64, fallback::AbstractString) where {sym}
  rng.ptr == C_NULL && throw(ErrorException("RNG pointer is NULL"))
  fill_strided! = function(S::StridedArray{Float64})
    x = reshape(S, :)
    ok = ccall((sym, _libpath[]), Bool,
               (Ptr{Float64}, Csize_t, Float64, Float64, Float64, RNGPtr),
               pointer(x), Csize_t(length(x)), p1, p2, p3, rng.ptr)
    _check_ok(ok, rng.ptr, fallback)
    return nothing
  end
  fill_tmp! = function(tmp::Vector{Float64})
    ok = ccall((sym, _libpath[]), Bool,
               (Ptr{Float64}, Csize_t, Float64, Float64, Float64, RNGPtr),
               pointer(tmp), Csize_t(length(tmp)), p1, p2, p3, rng.ptr)
    _check_ok(ok, rng.ptr, fallback)
    return nothing
  end
  return _fill_array!(A, fill_strided!, fill_tmp!)
end

function _fill3!(rng::RNG, A::AbstractArray{Float32}, ::Val{sym},
  p1::Float32, p2::Float32, p3::Float32, fallback::AbstractString) where {sym}
  rng.ptr == C_NULL && throw(ErrorException("RNG pointer is NULL"))
  fill_strided! = function(S::StridedArray{Float32})
    x = reshape(S, :)
    ok = ccall((sym, _libpath[]), Bool,
               (Ptr{Float32}, Csize_t, Float32, Float32, Float32, RNGPtr),
               pointer(x), Csize_t(length(x)), p1, p2, p3, rng.ptr)
    _check_ok(ok, rng.ptr, fallback)
    return nothing
  end
  fill_tmp! = function(tmp::Vector{Float32})
    ok = ccall((sym, _libpath[]), Bool,
               (Ptr{Float32}, Csize_t, Float32, Float32, Float32, RNGPtr),
               pointer(tmp), Csize_t(length(tmp)), p1, p2, p3, rng.ptr)
    _check_ok(ok, rng.ptr, fallback)
    return nothing
  end
  return _fill_array!(A, fill_strided!, fill_tmp!)
end
