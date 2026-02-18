"""
    random_int(rng)
    random_int(rng, dims...)
    random_int(rng, T, dims...)
    random_int(rng, a:b)
    random_int(rng, a:b, dims...)

Draw uniform integer variates. When a range a:b is specified, values are drawn uniformly
from the set {a, a+1, …, b}; otherwise from the full representable range of the output
type. With a specified range, its element type determines the output type; otherwise the
output type may be controlled by specifying an integer type T (for example Int32 or
UInt64), and if both are absent the default integer type Int is used.

The scalar form returns a single draw and the dims form allocates and
returns an array; dims can be given as a sequence or a tuple of integers.

# Examples
```julia
rng = rng_create()
x = random_int(rng, 5:10)
y = random_int(rng, Int32)
z = random_int(rng, Int16, 10)
A = random_int(rng, 1:6, (3,4))
```

# See also
- random_int!
- random_perm
- random_sample
"""
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


"""
    random_int!(rng, A)
    random_int!(rng, A, a:b)

Fill an existing integer scalar or array `A` with uniform integer draws. If a range `a:b`
is provided, values are drawn uniformly from the set {a, a+1,..., b}, otherwise from the
full representable range of the element type of `A`.

# Examples
```julia
rng = rng_create()
A = zeros(Int32, 10, 10)
B = Vector{UInt64}(undef, 5)
random_int!(rng, A)
random_int!(rng, B, 5:10)
```

# See also
- random_int
- random_perm!
- random_sample!
"""
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

"""
    random_perm(rng, n, T)

Return a random permutation of the integers `1:n` as a vector of type `T`.
If `T` is not specified, the default integer type `Int` is used.

# Examples
```julia
rng = rng_create()
p = random_perm(rng, 10)
q = random_perm(rng, 5, Int32)
```

# See also
- random_perm!
- random_int
- random_sample
"""

function random_perm(rng::RNG, n::Integer, ::Type{T}=Int) where {T<:_PermTypes}
  if n < 0
    throw(ArgumentError("n must be non-negative"))
  end
  p = Vector{T}(undef, n)
  random_perm!(rng, p)
  return p
end

"""
    random_perm!(rng, p)

Fill the integer vector `p` with a random permutation of the integers `1:n`, where `n =
length(p)`. The result is 1-based.

# Examples
```julia
rng = rng_create()
p = Vector{Int}(undef, 10)
random_perm!(rng, p)
```

# See also
- random_perm
- random_int!
- random_sample!
"""
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
    ccall(_sym(:randompack_perm), Bool, (Ptr{Cint}, Cint, RNGPtr),
          pointer(tmp), Cint(n), rng.ptr)
  end
  _check_ok(ok, rng.ptr, "randompack_perm failed")
  for i in 1:n
    @inbounds p[i] = T(tmp[i] + 1)
  end
  return p
end

"""
    random_sample(rng, n, k)
    random_sample(rng, n, k, T)

Return `k` samples without replacement from the integers `1:n` as a vector of type `T`.
If `T` is not specified, the default integer type `Int` is used.

# Examples
```julia
rng = rng_create()
s = random_sample(rng, 10, 4)
t = random_sample(rng, 12, 3, Int16)
```

# See also
- random_sample!
- random_int
- random_perm
"""
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

"""
    random_sample!(rng, s, n)

Fill the integer vector `s` with a sample *without replacement* from the integers `1:n`.
The sample size is `length(s)`. The result is 1-based.

# Examples
```julia
rng = rng_create()
s = Vector{Int32}(undef, 4)
random_sample!(rng, s, 10)
```

# See also
- random_sample
- random_int!
- random_perm!
"""
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
    ccall(_sym(:randompack_sample), Bool, (Ptr{Cint}, Cint, Cint, RNGPtr),
          pointer(tmp), Cint(n), Cint(k), rng.ptr)
  end
  _check_ok(ok, rng.ptr, "randompack_sample failed")
  for i in 1:k
    @inbounds s[i] = T(tmp[i] + 1)
  end
  return s
end
