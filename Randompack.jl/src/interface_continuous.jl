# -----------------------------------------------------------------------------
# Distributions
# -----------------------------------------------------------------------------

# --- uniform
"""
    random_unif(rng; a=0, b=1)
    random_unif(rng, dims; a=0, b=1)
    random_unif(rng, Float32, dims; a=0, b=1)

Draw uniform variates on an interval [a,b]. When a and b are absent variates from
Uniform(0,1) are drawn. The scalar form returns a single draw and the dims form allocates
and returns an array; dims can be given as a sequence or a tuple of integers. By default
Float64 variates are drawn; use `Float32` explicitly to draw single-precision variates.

# Examples
```julia
rng = rng_create()
u1 = random_unif(rng)
u2 = random_unif(rng, 10)
u3 = random_unif(rng, 10; a=2, b=5)
U1 = random_unif(rng, Float32, (10,10))
```

# See also
- random_unif!
"""
random_unif(rng::RNG; a=0, b=1) =
  _scalar_and_fill(rng, Float64, (r,x)->random_unif!(r, x; a=a, b=b))

random_unif(rng::RNG, dims::Int...; a=0, b=1) =
  _alloc_and_fill(rng, Float64, (r,x)->random_unif!(r, x; a=a, b=b), dims)

random_unif(rng::RNG, dims::Tuple{Vararg{Int}}; a=0, b=1) =
  _alloc_and_fill(rng, Float64, (r,x)->random_unif!(r, x; a=a, b=b), dims)

# explicit Float64 (kept for Base-like symmetry)
random_unif(rng::RNG, ::Type{Float64}; a=0, b=1) =
  random_unif(rng; a=a, b=b)

random_unif(rng::RNG, ::Type{Float64}, dims::Int...; a=0, b=1) =
  random_unif(rng, dims...; a=a, b=b)

random_unif(rng::RNG, ::Type{Float64}, dims::Tuple{Vararg{Int}}; a=0, b=1) =
  random_unif(rng, dims; a=a, b=b)

# explicit Float32
random_unif(rng::RNG, ::Type{Float32}; a=0, b=1) =
  _scalar_and_fill(rng, Float32, (r,x)->random_unif!(r, x; a=a, b=b))

random_unif(rng::RNG, ::Type{Float32}, dims::Int...; a=0, b=1) =
  _alloc_and_fill(rng, Float32, (r,x)->random_unif!(r, x; a=a, b=b), dims)

random_unif(rng::RNG, ::Type{Float32}, dims::Tuple{Vararg{Int}}; a=0, b=1) =
  _alloc_and_fill(rng, Float32, (r,x)->random_unif!(r, x; a=a, b=b), dims)

"""
    random_unif!(rng, A; a=0, b=1)

Fill an existing scalar or array `A` with uniform variates on the interval `[a,b]`. When
`a` and `b` are absent, Uniform(0,1) variates are drawn. `A` must have element type
`Float32` or `Float64`.

# Examples
```julia
rng = rng_create()
A = zeros(10, 20)
B = zeros(10, 20)
random_unif!(rng, A)              # sample from Uniform(0,1)
random_unif!(rng, B; a=2, b=5)    # sample from Uniform(2,5)
```

# See also
- random_unif
"""
function random_unif!(rng::RNG, A::AbstractArray{Float64}; a=0, b=1)
  a64 = Float64(a)
  b64 = Float64(b)
  !(a64 < b64) && throw(ArgumentError("require a < b"))
  return _fill2!(rng, A, Val(:randompack_unif), a64, b64, "randompack_unif failed")
end

function random_unif!(rng::RNG, A::AbstractArray{Float32}; a=0, b=1)
  a32 = Float32(a)
  b32 = Float32(b)
  !(a32 < b32) && throw(ArgumentError("require a < b"))
  return _fill2!(rng, A, Val(:randompack_uniff), a32, b32, "randompack_uniff failed")
end

# --- normal
"""
    random_normal(rng; mu=0, sigma=1)
    random_normal(rng, dims; mu=0, sigma=1)
    random_normal(rng, Float32, dims; mu=0, sigma=1)

Draw normal variates N(mu, sigma) for sigma > 0. When mu and sigma are absent standard
Normal variates fare drawn. The scalar form returns a single draw and the `dims` form
allocates and returns an array; dims can be given as a sequence or a tuple of integers. By
default Float64 variates are drawn; use `Float32` explicitly to draw single-precision
variates.

# Examples
```julia
rng = rng_create()
x1 = random_normal(rng)
x2 = random_normal(rng, Float32, (10,10); mu=1, sigma=0.5)
random_normal!(rng, X; mu=1, sigma=0.5)
```

# See also
- random_normal!
"""
# scalar
random_normal(rng::RNG; mu=0, sigma=1) =
  _scalar_and_fill(rng, Float64, (r,x)->random_normal!(r, x; mu=mu, sigma=sigma))

random_normal(rng::RNG, dims::Int...; mu=0, sigma=1) =
  _alloc_and_fill(rng, Float64, (r,x)->random_normal!(r, x; mu=mu, sigma=sigma), dims)

random_normal(rng::RNG, dims::Tuple{Vararg{Int}}; mu=0, sigma=1) =
  _alloc_and_fill(rng, Float64, (r,x)->random_normal!(r, x; mu=mu, sigma=sigma), dims)

# explicit Float64 (kept for Base-like symmetry)
random_normal(rng::RNG, ::Type{Float64}; mu=0, sigma=1) =
  random_normal(rng; mu=mu, sigma=sigma)

random_normal(rng::RNG, ::Type{Float64}, dims::Int...; mu=0, sigma=1) =
  random_normal(rng, dims...; mu=mu, sigma=sigma)

random_normal(rng::RNG, ::Type{Float64}, dims::Tuple{Vararg{Int}}; mu=0,
  sigma=1) =
  random_normal(rng, dims; mu=mu, sigma=sigma)

# Float32
random_normal(rng::RNG, ::Type{Float32}; mu=0, sigma=1) =
  _scalar_and_fill(rng, Float32, (r,x)->random_normal!(r, x; mu=mu, sigma=sigma))

random_normal(rng::RNG, ::Type{Float32}, dims::Int...; mu=0, sigma=1) =
  _alloc_and_fill(rng, Float32, (r,x)->random_normal!(r, x; mu=mu, sigma=sigma), dims)

random_normal(rng::RNG, ::Type{Float32}, dims::Tuple{Vararg{Int}}; mu=0,
  sigma=1) =
  _alloc_and_fill(rng, Float32, (r,x)->random_normal!(r, x; mu=mu, sigma=sigma), dims)
"""
    random_normal!(rng, A; mu=0, sigma=1)

Fill an existing scalar or array `A` with `N(mu,sigma)` variates. When `mu` and `sigma`
are absent, standard normal variates are drawn. `A` must have element type `Float32` or
`Float64`.

# Examples
```julia
rng = rng_create()
X = zeros(Float32, 10, 10)
Y = zeros(10, 10)
random_normal!(rng, X; mu=1f0, sigma=2f0)  # single precision N(1,2)
random_normal!(rng, Y)                     # double precision standard normal
```

# See also
- random_normal
"""
function random_normal!(rng::RNG, A::AbstractArray{Float64}; mu=0, sigma=1)
  mu64 = Float64(mu)
  sig64 = Float64(sigma)
  !(sig64 > 0) && throw(ArgumentError("require sigma > 0"))
  return _fill2!(rng, A, Val(:randompack_normal), mu64, sig64, "randompack_normal failed")
end

function random_normal!(rng::RNG, A::AbstractArray{Float32}; mu=0, sigma=1)
  mu32 = Float32(mu)
  sig32 = Float32(sigma)
  !(sig32 > 0) && throw(ArgumentError("require sigma > 0"))
  return _fill2!(rng, A, Val(:randompack_normalf), mu32, sig32, "randompack_normalf failed")
end

# --- skew normal
"""
    random_skew_normal(rng; mu=0, sigma=1, alpha)
    random_skew_normal(rng, dims; mu=0, sigma=1, alpha)
    random_skew_normal(rng, Float32, dims; mu=0, sigma=1, alpha)

Draw skew-normal variates with location `mu`, scale `sigma` (positive), and skew parameter
`alpha` (mandatory). The scalar form returns a single draw and the `dims` form allocates
and returns an array; dims can be given as a sequence or a tuple of integers. By default
Float64 variates are drawn; use `Float32` explicitly to draw single-precision variates.

# Examples
```julia
rng = rng_create()
x = random_skew_normal(rng, 8, 8; alpha=1, sigma=2)   # 8 by 8 Float64 matrix, mu=0
y = random_skew_normal(rng; Float32, (8,8); alpha=2)  # defaults to mu=0, sigma=1
z = random_skew_normal(rng; mu=1, sigma=2, alpha=3)   # scalar
```

# See also
- random_skew_normal!
"""
# default Float64
random_skew_normal(rng::RNG; mu=0, sigma=1, alpha) =
  _scalar_and_fill(rng, Float64,
    (r,x)->random_skew_normal!(r, x; mu=mu, sigma=sigma, alpha=alpha))

random_skew_normal(rng::RNG, dims::Int...; mu=0, sigma=1, alpha) =
  _alloc_and_fill(rng, Float64, 
    (r,x)->random_skew_normal!(r, x; mu=mu, sigma=sigma, alpha=alpha), dims)

random_skew_normal(rng::RNG, dims::Tuple{Vararg{Int}}; mu=0,
  sigma=1, alpha) =
    _alloc_and_fill(rng, Float64,
      (r,x)->random_skew_normal!(r, x; mu=mu, sigma=sigma, alpha=alpha), dims)

# explicit Float64 (kept for Base-like symmetry)
random_skew_normal(rng::RNG, ::Type{Float64}; mu=0, sigma=1,
  alpha) =
  random_skew_normal(rng; mu=mu, sigma=sigma, alpha=alpha)

random_skew_normal(rng::RNG, ::Type{Float64}, dims::Int...; mu=0,
  sigma=1, alpha) =
  random_skew_normal(rng, dims...; mu=mu, sigma=sigma, alpha=alpha)

random_skew_normal(rng::RNG, ::Type{Float64}, dims::Tuple{Vararg{Int}}; mu=0,
  sigma=1, alpha) =
  random_skew_normal(rng, dims; mu=mu, sigma=sigma, alpha=alpha)

# Float32
random_skew_normal(rng::RNG, ::Type{Float32}; mu=0, sigma=1,
  alpha) =
  _scalar_and_fill(rng, Float32,
                   (r,x)->random_skew_normal!(r, x; mu=mu, sigma=sigma, alpha=alpha))

random_skew_normal(rng::RNG, ::Type{Float32}, dims::Int...; mu=0,
  sigma=1, alpha) =
  _alloc_and_fill(rng, Float32,
                  (r,x)->random_skew_normal!(r, x; mu=mu, sigma=sigma, alpha=alpha),
                  dims)

random_skew_normal(rng::RNG, ::Type{Float32}, dims::Tuple{Vararg{Int}}; mu=0,
  sigma=1, alpha) =
  _alloc_and_fill(rng, Float32,
                  (r,x)->random_skew_normal!(r, x; mu=mu, sigma=sigma, alpha=alpha),
                  dims)

"""
    random_skew_normal!(rng, A; mu=0, sigma=1, alpha)

Fill an existing scalar or array `A` with skew-normal variates with parameters `mu`,
`sigma` (positive), and `alpha`. `A` must have element type `Float32` or `Float64`.

# Examples
```julia
rng = rng_create()
A = zeros(4,4,4)
B = Matrix(8,8)
random_skew_normal!(rng, A; alpha=3, sigma=2, mu=1)  # 4×4×4, order can be changed
random_skew_normal!(rng, B; alpha=3)                 # mu=0 and sigma=1 by default
```

# See also
- random_skew_normal
"""
function random_skew_normal!(rng::RNG, A::AbstractArray{Float64};
  mu=0, sigma=1, alpha)
  mu64 = Float64(mu)
  sig64 = Float64(sigma)
  al64 = Float64(alpha)
  !(sig64 > 0) && throw(ArgumentError("require sigma > 0"))
  return _fill3!(rng, A, Val(:randompack_skew_normal), mu64, sig64, al64,
                 "randompack_skew_normal failed")
end

function random_skew_normal!(rng::RNG, A::AbstractArray{Float32};
  mu=0, sigma=1, alpha)
  mu32 = Float32(mu)
  sig32 = Float32(sigma)
  al32 = Float32(alpha)
  !(sig32 > 0) && throw(ArgumentError("require sigma > 0"))
  return _fill3!(rng, A, Val(:randompack_skew_normalf), mu32, sig32, al32,
                 "randompack_skew_normalf failed")
end

# --- lognormal
"""
    random_lognormal(rng; mu=0, sigma=1)
    random_lognormal(rng, dims; mu=0, sigma=1)
    random_lognormal(rng, Float32, dims; mu=0, sigma=1)

Draw lognormal variates with parameters `mu` and `sigma` (positive). The scalar form
returns a single draw and the dims form allocates and returns an array; dims can be given
as a sequence or a tuple of integers. By default Float64 variates are drawn; use
`Float32` explicitly to draw single-precision variates.

# Examples
```julia
rng = rng_create()
x = random_lognormal(rng)
X = random_lognormal(rng, 10; mu=1, sigma=0.5)
Y = random_lognormal(rng, Float32, (4,4); mu=0, sigma=2)
```

# See also
- random_lognormal!
"""
random_lognormal(rng::RNG; mu=0, sigma=1) =
  _scalar_and_fill(rng, Float64, (r,x)->random_lognormal!(r, x; mu=mu, sigma=sigma))

random_lognormal(rng::RNG, dims::Int...; mu=0, sigma=1) =
  _alloc_and_fill(rng, Float64, (r,x)->random_lognormal!(r, x; mu=mu, sigma=sigma),
                  dims)

random_lognormal(rng::RNG, dims::Tuple{Vararg{Int}}; mu=0, sigma=1) =
  _alloc_and_fill(rng, Float64, (r,x)->random_lognormal!(r, x; mu=mu, sigma=sigma),
                  dims)

# explicit Float64 (kept for Base-like symmetry)
random_lognormal(rng::RNG, ::Type{Float64}; mu=0, sigma=1) =
  random_lognormal(rng; mu=mu, sigma=sigma)

random_lognormal(rng::RNG, ::Type{Float64}, dims::Int...; mu=0, sigma=1) =
  random_lognormal(rng, dims...; mu=mu, sigma=sigma)

random_lognormal(rng::RNG, ::Type{Float64}, dims::Tuple{Vararg{Int}}; mu=0,
  sigma=1) =
  random_lognormal(rng, dims; mu=mu, sigma=sigma)

# Float32
random_lognormal(rng::RNG, ::Type{Float32}; mu=0, sigma=1) =
  _scalar_and_fill(rng, Float32, (r,x)->random_lognormal!(r, x; mu=mu, sigma=sigma))

random_lognormal(rng::RNG, ::Type{Float32}, dims::Int...; mu=0, sigma=1) =
  _alloc_and_fill(rng, Float32, (r,x)->random_lognormal!(r, x; mu=mu, sigma=sigma),
                  dims)

random_lognormal(rng::RNG, ::Type{Float32}, dims::Tuple{Vararg{Int}}; mu=0,
  sigma=1) =
  _alloc_and_fill(rng, Float32, (r,x)->random_lognormal!(r, x; mu=mu, sigma=sigma),
                  dims)

"""
    random_lognormal!(rng, A; mu=0, sigma=1)

Fill an existing scalar or array `A` with lognormal variates with parameters `mu` and
`sigma` (positive). `A` must have element type `Float32` or `Float64`.

# Examples
```julia
rng = rng_create()
A = zeros(Float32, 10, 10)
B = zeros(10, 10)
random_lognormal!(rng, A; mu=1f0, sigma=0.5f0)
random_lognormal!(rng, B)
```

# See also
- random_lognormal
"""
function random_lognormal!(rng::RNG, A::AbstractArray{Float64}; mu=0, sigma=1)
  mu64 = Float64(mu)
  sig64 = Float64(sigma)
  !(sig64 > 0) && throw(ArgumentError("require sigma > 0"))
  return _fill2!(rng, A, Val(:randompack_lognormal), mu64, sig64, "randompack_lognormal failed")
end

function random_lognormal!(rng::RNG, A::AbstractArray{Float32}; mu=0, sigma=1)
  mu32 = Float32(mu)
  sig32 = Float32(sigma)
  !(sig32 > 0) && throw(ArgumentError("require sigma > 0"))
  return _fill2!(rng, A, Val(:randompack_lognormalf), mu32, sig32, "randompack_lognormalf failed")
end

# --- exp
"""
    random_exp(rng; scale=1)
    random_exp(rng, dims; scale=1)
    random_exp(rng, Float32, dims; scale=1)

Draw exponential variates with scale parameter `scale` (positive). The scalar form
returns a single draw and the dims form allocates and returns an array; dims can be given
as a sequence or a tuple of integers. By default Float64 variates are drawn; use
`Float32` explicitly to draw single-precision variates.

# Examples
```julia
rng = rng_create()
x = random_exp(rng; scale=2)
X = random_exp(rng, 10)
Y = random_exp(rng, Float32, (4,4); scale=0.5)
```

# See also
- random_exp!
"""
random_exp(rng::RNG; scale=1) =
  _scalar_and_fill(rng, Float64, (r,x)->random_exp!(r, x; scale=scale))

random_exp(rng::RNG, dims::Int...; scale=1) =
  _alloc_and_fill(rng, Float64, (r,x)->random_exp!(r, x; scale=scale), dims)

random_exp(rng::RNG, dims::Tuple{Vararg{Int}}; scale=1) =
  _alloc_and_fill(rng, Float64, (r,x)->random_exp!(r, x; scale=scale), dims)

# explicit Float64 (kept for Base-like symmetry)
random_exp(rng::RNG, ::Type{Float64}; scale=1) =
  random_exp(rng; scale=scale)

random_exp(rng::RNG, ::Type{Float64}, dims::Int...; scale=1) =
  random_exp(rng, dims...; scale=scale)

random_exp(rng::RNG, ::Type{Float64}, dims::Tuple{Vararg{Int}}; scale=1) =
  random_exp(rng, dims; scale=scale)

# Float32
random_exp(rng::RNG, ::Type{Float32}; scale=1) =
  _scalar_and_fill(rng, Float32, (r,x)->random_exp!(r, x; scale=scale))

random_exp(rng::RNG, ::Type{Float32}, dims::Int...; scale=1) =
  _alloc_and_fill(rng, Float32, (r,x)->random_exp!(r, x; scale=scale), dims)

random_exp(rng::RNG, ::Type{Float32}, dims::Tuple{Vararg{Int}}; scale=1) =
  _alloc_and_fill(rng, Float32, (r,x)->random_exp!(r, x; scale=scale), dims)

"""
    random_exp!(rng, A; scale=1)

Fill an existing scalar or array `A` with exponential variates with scale parameter
`scale` (positive). `A` must have element type `Float32` or `Float64`.

# Examples
```julia
rng = rng_create()
A = zeros(Float32, 10, 10)
B = zeros(10, 10)
random_exp!(rng, A; scale=0.5)
random_exp!(rng, B)
```

# See also
- random_exp
"""
function random_exp!(rng::RNG, A::AbstractArray{Float64}; scale=1)
  sc64 = Float64(scale)
  !(sc64 > 0) && throw(ArgumentError("require scale > 0"))
  return _fill1!(rng, A, Val(:randompack_exp), sc64, "randompack_exp failed")
end

function random_exp!(rng::RNG, A::AbstractArray{Float32}; scale=1)
  sc32 = Float32(scale)
  !(sc32 > 0) && throw(ArgumentError("require scale > 0"))
  return _fill1!(rng, A, Val(:randompack_expf), sc32, "randompack_expf failed")
end

# --- gamma
"""
    random_gamma(rng; shape, scale=1)
    random_gamma(rng, dims; shape, scale=1)
    random_gamma(rng, Float32, dims; shape, scale=1)

Draw gamma variates with shape `shape` (positive) and scale `scale` (positive). The
scalar form returns a single draw and the dims form allocates and returns an array; dims
can be given as a sequence or a tuple of integers. By default Float64 variates are drawn;
use `Float32` explicitly to draw single-precision variates.

# Examples
```julia
rng = rng_create()
x = random_gamma(rng; shape=2, scale=3)
X = random_gamma(rng, 10; shape=1.5)
Y = random_gamma(rng, Float32, (4,4); shape=2, scale=0.5)
```

# See also
- random_gamma!
"""
random_gamma(rng::RNG; shape, scale=1) =
  _scalar_and_fill(rng, Float64, (r,x)->random_gamma!(r, x; shape=shape, scale=scale))

random_gamma(rng::RNG, dims::Int...; shape, scale=1) =
  _alloc_and_fill(rng, Float64, (r,x)->random_gamma!(r, x; shape=shape, scale=scale),
                  dims)

random_gamma(rng::RNG, dims::Tuple{Vararg{Int}}; shape, scale=1) =
  _alloc_and_fill(rng, Float64, (r,x)->random_gamma!(r, x; shape=shape, scale=scale),
                  dims)

# explicit Float64 (kept for Base-like symmetry)
random_gamma(rng::RNG, ::Type{Float64}; shape, scale=1) =
  random_gamma(rng; shape=shape, scale=scale)

random_gamma(rng::RNG, ::Type{Float64}, dims::Int...; shape, scale=1) =
  random_gamma(rng, dims...; shape=shape, scale=scale)

random_gamma(rng::RNG, ::Type{Float64}, dims::Tuple{Vararg{Int}}; shape,
  scale=1) =
  random_gamma(rng, dims; shape=shape, scale=scale)

# Float32
random_gamma(rng::RNG, ::Type{Float32}; shape, scale=1) =
  _scalar_and_fill(rng, Float32, (r,x)->random_gamma!(r, x; shape=shape, scale=scale))

random_gamma(rng::RNG, ::Type{Float32}, dims::Int...; shape, scale=1) =
  _alloc_and_fill(rng, Float32, (r,x)->random_gamma!(r, x; shape=shape, scale=scale),
                  dims)

random_gamma(rng::RNG, ::Type{Float32}, dims::Tuple{Vararg{Int}}; shape,
  scale=1) =
  _alloc_and_fill(rng, Float32, (r,x)->random_gamma!(r, x; shape=shape, scale=scale),
                  dims)

"""
    random_gamma!(rng, A; shape, scale=1)

Fill an existing scalar or array `A` with gamma variates with shape `shape` and scale
`scale` (both positive). `A` must have element type `Float32` or `Float64`.

# Examples
```julia
rng = rng_create()
A = zeros(Float32, 10, 10)
B = zeros(10, 10)
random_gamma!(rng, A; shape=2f0, scale=0.5f0)
random_gamma!(rng, B; shape=1, scale=2)
```

# See also
- random_gamma
"""
function random_gamma!(rng::RNG, A::AbstractArray{Float64}; shape, scale=1)
  sh64 = Float64(shape)
  sc64 = Float64(scale)
  !(sh64 > 0 && sc64 > 0) && throw(ArgumentError("require shape > 0 and scale > 0"))
  return _fill2!(rng, A, Val(:randompack_gamma), sh64, sc64, "randompack_gamma failed")
end

function random_gamma!(rng::RNG, A::AbstractArray{Float32}; shape, scale=1)
  sh32 = Float32(shape)
  sc32 = Float32(scale)
  !(sh32 > 0 && sc32 > 0) && throw(ArgumentError("require shape > 0 and scale > 0"))
  return _fill2!(rng, A, Val(:randompack_gammaf), sh32, sc32, "randompack_gammaf failed")
end

# --- chi2
"""
    random_chi2(rng; nu)
    random_chi2(rng, dims; nu)
    random_chi2(rng, Float32, dims; nu)

Draw chi-square variates with degrees of freedom `nu` (positive). The scalar form returns
a single draw and the dims form allocates and returns an array; dims can be given as a
sequence or a tuple of integers. By default Float64 variates are drawn; use `Float32`
explicitly to draw single-precision variates.

# Examples
```julia
rng = rng_create()
x = random_chi2(rng; nu=5)
X = random_chi2(rng, 10; nu=2)
Y = random_chi2(rng, Float32, (4,4); nu=3)
```

# See also
- random_chi2!
"""
random_chi2(rng::RNG; nu) =
  _scalar_and_fill(rng, Float64, (r,x)->random_chi2!(r, x; nu=nu))

random_chi2(rng::RNG, dims::Int...; nu) =
  _alloc_and_fill(rng, Float64, (r,x)->random_chi2!(r, x; nu=nu), dims)

random_chi2(rng::RNG, dims::Tuple{Vararg{Int}}; nu) =
  _alloc_and_fill(rng, Float64, (r,x)->random_chi2!(r, x; nu=nu), dims)

# explicit Float64 (kept for Base-like symmetry)
random_chi2(rng::RNG, ::Type{Float64}; nu) =
  random_chi2(rng; nu=nu)

random_chi2(rng::RNG, ::Type{Float64}, dims::Int...; nu) =
  random_chi2(rng, dims...; nu=nu)

random_chi2(rng::RNG, ::Type{Float64}, dims::Tuple{Vararg{Int}}; nu) =
  random_chi2(rng, dims; nu=nu)

# Float32
random_chi2(rng::RNG, ::Type{Float32}; nu) =
  _scalar_and_fill(rng, Float32, (r,x)->random_chi2!(r, x; nu=nu))

random_chi2(rng::RNG, ::Type{Float32}, dims::Int...; nu) =
  _alloc_and_fill(rng, Float32, (r,x)->random_chi2!(r, x; nu=nu), dims)

random_chi2(rng::RNG, ::Type{Float32}, dims::Tuple{Vararg{Int}}; nu) =
  _alloc_and_fill(rng, Float32, (r,x)->random_chi2!(r, x; nu=nu), dims)

"""
    random_chi2!(rng, A; nu)

Fill an existing scalar or array `A` with chi-square variates with degrees of freedom
`nu` (positive). `A` must have element type `Float32` or `Float64`.

# Examples
```julia
rng = rng_create()
A = zeros(Float32, 10, 10)
B = zeros(10, 10)
random_chi2!(rng, A; nu=2f0)
random_chi2!(rng, B; nu=5)
```

# See also
- random_chi2
"""
function random_chi2!(rng::RNG, A::AbstractArray{Float64}; nu)
  nu64 = Float64(nu)
  !(nu64 > 0) && throw(ArgumentError("require nu > 0"))
  return _fill1!(rng, A, Val(:randompack_chi2), nu64, "randompack_chi2 failed")
end

function random_chi2!(rng::RNG, A::AbstractArray{Float32}; nu)
  nu32 = Float32(nu)
  !(nu32 > 0) && throw(ArgumentError("require nu > 0"))
  return _fill1!(rng, A, Val(:randompack_chi2f), nu32, "randompack_chi2f failed")
end

# --- beta
"""
    random_beta(rng; a, b)
    random_beta(rng, dims; a, b)
    random_beta(rng, Float32, dims; a, b)

Draw beta variates with shape parameters `a` and `b` (both positive). The scalar form
returns a single draw and the dims form allocates and returns an array; dims can be given
as a sequence or a tuple of integers. By default Float64 variates are drawn; use
`Float32` explicitly to draw single-precision variates.

# Examples
```julia
rng = rng_create()
x = random_beta(rng; a=2, b=3)
X = random_beta(rng, 10; a=0.5, b=1.5)
Y = random_beta(rng, Float32, (4,4); a=1, b=2)
```

# See also
- random_beta!
"""
random_beta(rng::RNG; a, b) =
  _scalar_and_fill(rng, Float64, (r,x)->random_beta!(r, x; a=a, b=b))

random_beta(rng::RNG, dims::Int...; a, b) =
  _alloc_and_fill(rng, Float64, (r,x)->random_beta!(r, x; a=a, b=b), dims)

random_beta(rng::RNG, dims::Tuple{Vararg{Int}}; a, b) =
  _alloc_and_fill(rng, Float64, (r,x)->random_beta!(r, x; a=a, b=b), dims)

# explicit Float64 (kept for Base-like symmetry)
random_beta(rng::RNG, ::Type{Float64}; a, b) =
  random_beta(rng; a=a, b=b)

random_beta(rng::RNG, ::Type{Float64}, dims::Int...; a, b) =
  random_beta(rng, dims...; a=a, b=b)

random_beta(rng::RNG, ::Type{Float64}, dims::Tuple{Vararg{Int}}; a, b) =
  random_beta(rng, dims; a=a, b=b)

# Float32
random_beta(rng::RNG, ::Type{Float32}; a, b) =
  _scalar_and_fill(rng, Float32, (r,x)->random_beta!(r, x; a=a, b=b))

random_beta(rng::RNG, ::Type{Float32}, dims::Int...; a, b) =
  _alloc_and_fill(rng, Float32, (r,x)->random_beta!(r, x; a=a, b=b), dims)

random_beta(rng::RNG, ::Type{Float32}, dims::Tuple{Vararg{Int}}; a, b) =
  _alloc_and_fill(rng, Float32, (r,x)->random_beta!(r, x; a=a, b=b), dims)

"""
    random_beta!(rng, A; a, b)

Fill an existing scalar or array `A` with beta variates with shape parameters `a` and
`b` (both positive). `A` must have element type `Float32` or `Float64`.

# Examples
```julia
rng = rng_create()
A = zeros(Float32, 10, 10)
B = zeros(10, 10)
random_beta!(rng, A; a=1f0, b=2f0)
random_beta!(rng, B; a=2, b=3)
```

# See also
- random_beta
"""
function random_beta!(rng::RNG, A::AbstractArray{Float64}; a, b)
  a64 = Float64(a)
  b64 = Float64(b)
  !(a64 > 0 && b64 > 0) && throw(ArgumentError("require a > 0 and b > 0"))
  return _fill2!(rng, A, Val(:randompack_beta), a64, b64, "randompack_beta failed")
end

function random_beta!(rng::RNG, A::AbstractArray{Float32}; a, b)
  a32 = Float32(a)
  b32 = Float32(b)
  !(a32 > 0 && b32 > 0) && throw(ArgumentError("require a > 0 and b > 0"))
  return _fill2!(rng, A, Val(:randompack_betaf), a32, b32, "randompack_betaf failed")
end

# --- t
"""
    random_t(rng; nu)
    random_t(rng, dims; nu)
    random_t(rng, Float32, dims; nu)

Draw Student t variates with degrees of freedom `nu` (positive). The scalar form returns
a single draw and the dims form allocates and returns an array; dims can be given as a
sequence or a tuple of integers. By default Float64 variates are drawn; use `Float32`
explicitly to draw single-precision variates.

# Examples
```julia
rng = rng_create()
x = random_t(rng; nu=5)
X = random_t(rng, 10; nu=2)
Y = random_t(rng, Float32, (4,4); nu=8)
```

# See also
- random_t!
"""
random_t(rng::RNG; nu) =
  _scalar_and_fill(rng, Float64, (r,x)->random_t!(r, x; nu=nu))

random_t(rng::RNG, dims::Int...; nu) =
  _alloc_and_fill(rng, Float64, (r,x)->random_t!(r, x; nu=nu), dims)

random_t(rng::RNG, dims::Tuple{Vararg{Int}}; nu) =
  _alloc_and_fill(rng, Float64, (r,x)->random_t!(r, x; nu=nu), dims)

# explicit Float64 (kept for Base-like symmetry)
random_t(rng::RNG, ::Type{Float64}; nu) =
  random_t(rng; nu=nu)

random_t(rng::RNG, ::Type{Float64}, dims::Int...; nu) =
  random_t(rng, dims...; nu=nu)

random_t(rng::RNG, ::Type{Float64}, dims::Tuple{Vararg{Int}}; nu) =
  random_t(rng, dims; nu=nu)

# Float32
random_t(rng::RNG, ::Type{Float32}; nu) =
  _scalar_and_fill(rng, Float32, (r,x)->random_t!(r, x; nu=nu))

random_t(rng::RNG, ::Type{Float32}, dims::Int...; nu) =
  _alloc_and_fill(rng, Float32, (r,x)->random_t!(r, x; nu=nu), dims)

random_t(rng::RNG, ::Type{Float32}, dims::Tuple{Vararg{Int}}; nu) =
  _alloc_and_fill(rng, Float32, (r,x)->random_t!(r, x; nu=nu), dims)

"""
    random_t!(rng, A; nu)

Fill an existing scalar or array `A` with Student t variates with degrees of freedom
`nu` (positive). `A` must have element type `Float32` or `Float64`.

# Examples
```julia
rng = rng_create()
A = zeros(Float32, 10, 10)
B = zeros(10, 10)
random_t!(rng, A; nu=2f0)
random_t!(rng, B; nu=5)
```

# See also
- random_t
"""
function random_t!(rng::RNG, A::AbstractArray{Float64}; nu)
  nu64 = Float64(nu)
  !(nu64 > 0) && throw(ArgumentError("require nu > 0"))
  return _fill1!(rng, A, Val(:randompack_t), nu64, "randompack_t failed")
end

function random_t!(rng::RNG, A::AbstractArray{Float32}; nu)
  nu32 = Float32(nu)
  !(nu32 > 0) && throw(ArgumentError("require nu > 0"))
  return _fill1!(rng, A, Val(:randompack_tf), nu32, "randompack_tf failed")
end

# --- f
"""
    random_f(rng; nu1, nu2)
    random_f(rng, dims; nu1, nu2)
    random_f(rng, Float32, dims; nu1, nu2)

Draw F-distribution variates with degrees of freedom `nu1` and `nu2` (both positive). The
scalar form returns a single draw and the dims form allocates and returns an array; dims
can be given as a sequence or a tuple of integers. By default Float64 variates are drawn;
use `Float32` explicitly to draw single-precision variates.

# Examples
```julia
rng = rng_create()
x = random_f(rng; nu1=5, nu2=7)
X = random_f(rng, 10; nu1=3, nu2=9)
Y = random_f(rng, Float32, (4,4); nu1=4, nu2=6)
```

# See also
- random_f!
"""
random_f(rng::RNG; nu1, nu2) =
  _scalar_and_fill(rng, Float64, (r,x)->random_f!(r, x; nu1=nu1, nu2=nu2))

random_f(rng::RNG, dims::Int...; nu1, nu2) =
  _alloc_and_fill(rng, Float64, (r,x)->random_f!(r, x; nu1=nu1, nu2=nu2), dims)

random_f(rng::RNG, dims::Tuple{Vararg{Int}}; nu1, nu2) =
  _alloc_and_fill(rng, Float64, (r,x)->random_f!(r, x; nu1=nu1, nu2=nu2), dims)

# explicit Float64 (kept for Base-like symmetry)
random_f(rng::RNG, ::Type{Float64}; nu1, nu2) =
  random_f(rng; nu1=nu1, nu2=nu2)

random_f(rng::RNG, ::Type{Float64}, dims::Int...; nu1, nu2) =
  random_f(rng, dims...; nu1=nu1, nu2=nu2)

random_f(rng::RNG, ::Type{Float64}, dims::Tuple{Vararg{Int}}; nu1, nu2) =
  random_f(rng, dims; nu1=nu1, nu2=nu2)

# Float32
random_f(rng::RNG, ::Type{Float32}; nu1, nu2) =
  _scalar_and_fill(rng, Float32, (r,x)->random_f!(r, x; nu1=nu1, nu2=nu2))

random_f(rng::RNG, ::Type{Float32}, dims::Int...; nu1, nu2) =
  _alloc_and_fill(rng, Float32, (r,x)->random_f!(r, x; nu1=nu1, nu2=nu2), dims)

random_f(rng::RNG, ::Type{Float32}, dims::Tuple{Vararg{Int}}; nu1,
  nu2) =
  _alloc_and_fill(rng, Float32, (r,x)->random_f!(r, x; nu1=nu1, nu2=nu2), dims)

"""
    random_f!(rng, A; nu1, nu2)

Fill an existing scalar or array `A` with F-distribution variates with degrees of
freedom `nu1` and `nu2` (both positive). `A` must have element type `Float32` or
`Float64`.

# Examples
```julia
rng = rng_create()
A = zeros(Float32, 10, 10)
B = zeros(10, 10)
random_f!(rng, A; nu1=2f0, nu2=5f0)
random_f!(rng, B; nu1=3, nu2=6)
```

# See also
- random_f
"""
function random_f!(rng::RNG, A::AbstractArray{Float64}; nu1, nu2)
  n1 = Float64(nu1)
  n2 = Float64(nu2)
  !(n1 > 0 && n2 > 0) && throw(ArgumentError("require nu1 > 0 and nu2 > 0"))
  return _fill2!(rng, A, Val(:randompack_f), n1, n2, "randompack_f failed")
end

function random_f!(rng::RNG, A::AbstractArray{Float32}; nu1, nu2)
  n1 = Float32(nu1)
  n2 = Float32(nu2)
  !(n1 > 0 && n2 > 0) && throw(ArgumentError("require nu1 > 0 and nu2 > 0"))
  return _fill2!(rng, A, Val(:randompack_ff), n1, n2, "randompack_ff failed")
end

# --- gumbel
"""
    random_gumbel(rng; mu=0, beta=1)
    random_gumbel(rng, dims; mu=0, beta=1)
    random_gumbel(rng, Float32, dims; mu=0, beta=1)

Draw Gumbel variates with location `mu` and scale `beta` (positive). The scalar form
returns a single draw and the dims form allocates and returns an array; dims can be given
as a sequence or a tuple of integers. By default Float64 variates are drawn; use
`Float32` explicitly to draw single-precision variates.

# Examples
```julia
rng = rng_create()
x = random_gumbel(rng; mu=0, beta=2)
X = random_gumbel(rng, 10; mu=1, beta=0.5)
Y = random_gumbel(rng, Float32, (4,4); mu=0, beta=1)
```

# See also
- random_gumbel!
"""
random_gumbel(rng::RNG; mu=0, beta=1) =
  _scalar_and_fill(rng, Float64, (r,x)->random_gumbel!(r, x; mu=mu, beta=beta))

random_gumbel(rng::RNG, dims::Int...; mu=0, beta=1) =
  _alloc_and_fill(rng, Float64, (r,x)->random_gumbel!(r, x; mu=mu, beta=beta), dims)

random_gumbel(rng::RNG, dims::Tuple{Vararg{Int}}; mu=0, beta=1) =
  _alloc_and_fill(rng, Float64, (r,x)->random_gumbel!(r, x; mu=mu, beta=beta), dims)

# explicit Float64 (kept for Base-like symmetry)
random_gumbel(rng::RNG, ::Type{Float64}; mu=0, beta=1) =
  random_gumbel(rng; mu=mu, beta=beta)

random_gumbel(rng::RNG, ::Type{Float64}, dims::Int...; mu=0, beta=1) =
  random_gumbel(rng, dims...; mu=mu, beta=beta)

random_gumbel(rng::RNG, ::Type{Float64}, dims::Tuple{Vararg{Int}}; mu=0,
  beta=1) =
  random_gumbel(rng, dims; mu=mu, beta=beta)

# Float32
random_gumbel(rng::RNG, ::Type{Float32}; mu=0, beta=1) =
  _scalar_and_fill(rng, Float32, (r,x)->random_gumbel!(r, x; mu=mu, beta=beta))

random_gumbel(rng::RNG, ::Type{Float32}, dims::Int...; mu=0, beta=1) =
  _alloc_and_fill(rng, Float32, (r,x)->random_gumbel!(r, x; mu=mu, beta=beta), dims)

random_gumbel(rng::RNG, ::Type{Float32}, dims::Tuple{Vararg{Int}}; mu=0,
  beta=1) =
  _alloc_and_fill(rng, Float32, (r,x)->random_gumbel!(r, x; mu=mu, beta=beta), dims)

"""
    random_gumbel!(rng, A; mu=0, beta=1)

Fill an existing scalar or array `A` with Gumbel variates with location `mu` and scale
`beta` (positive). `A` must have element type `Float32` or `Float64`.

# Examples
```julia
rng = rng_create()
A = zeros(Float32, 10, 10)
B = zeros(10, 10)
random_gumbel!(rng, A; mu=0, beta=2f0)
random_gumbel!(rng, B; mu=1, beta=0.5)
```

# See also
- random_gumbel
"""
function random_gumbel!(rng::RNG, A::AbstractArray{Float64}; mu=0, beta=1)
  mu64 = Float64(mu)
  be64 = Float64(beta)
  !(be64 > 0) && throw(ArgumentError("require beta > 0"))
  return _fill2!(rng, A, Val(:randompack_gumbel), mu64, be64, "randompack_gumbel failed")
end

function random_gumbel!(rng::RNG, A::AbstractArray{Float32}; mu=0, beta=1)
  mu32 = Float32(mu)
  be32 = Float32(beta)
  !(be32 > 0) && throw(ArgumentError("require beta > 0"))
  return _fill2!(rng, A, Val(:randompack_gumbelf), mu32, be32, "randompack_gumbelf failed")
end

# --- pareto
"""
    random_pareto(rng; xm, alpha)
    random_pareto(rng, dims; xm, alpha)
    random_pareto(rng, Float32, dims; xm, alpha)

Draw Pareto variates with scale `xm` and shape `alpha` (both positive). The scalar form
returns a single draw and the dims form allocates and returns an array; dims can be given
as a sequence or a tuple of integers. By default Float64 variates are drawn; use
`Float32` explicitly to draw single-precision variates.

# Examples
```julia
rng = rng_create()
x = random_pareto(rng; xm=1, alpha=2)
X = random_pareto(rng, 10; xm=2, alpha=3)
Y = random_pareto(rng, Float32, (4,4); xm=1, alpha=1.5)
```

# See also
- random_pareto!
"""
random_pareto(rng::RNG; xm, alpha) =
  _scalar_and_fill(rng, Float64, (r,x)->random_pareto!(r, x; xm=xm, alpha=alpha))

random_pareto(rng::RNG, dims::Int...; xm, alpha) =
  _alloc_and_fill(rng, Float64, (r,x)->random_pareto!(r, x; xm=xm, alpha=alpha), dims)

random_pareto(rng::RNG, dims::Tuple{Vararg{Int}}; xm, alpha) =
  _alloc_and_fill(rng, Float64, (r,x)->random_pareto!(r, x; xm=xm, alpha=alpha), dims)

# explicit Float64 (kept for Base-like symmetry)
random_pareto(rng::RNG, ::Type{Float64}; xm, alpha) =
  random_pareto(rng; xm=xm, alpha=alpha)

random_pareto(rng::RNG, ::Type{Float64}, dims::Int...; xm, alpha) =
  random_pareto(rng, dims...; xm=xm, alpha=alpha)

random_pareto(rng::RNG, ::Type{Float64}, dims::Tuple{Vararg{Int}}; xm,
  alpha) =
  random_pareto(rng, dims; xm=xm, alpha=alpha)

# Float32
random_pareto(rng::RNG, ::Type{Float32}; xm, alpha) =
  _scalar_and_fill(rng, Float32, (r,x)->random_pareto!(r, x; xm=xm, alpha=alpha))

random_pareto(rng::RNG, ::Type{Float32}, dims::Int...; xm, alpha) =
  _alloc_and_fill(rng, Float32, (r,x)->random_pareto!(r, x; xm=xm, alpha=alpha), dims)

random_pareto(rng::RNG, ::Type{Float32}, dims::Tuple{Vararg{Int}}; xm,
  alpha) =
  _alloc_and_fill(rng, Float32, (r,x)->random_pareto!(r, x; xm=xm, alpha=alpha), dims)

"""
    random_pareto!(rng, A; xm, alpha)

Fill an existing scalar or array `A` with Pareto variates with scale `xm` and shape
`alpha` (both positive). `A` must have element type `Float32` or `Float64`.

# Examples
```julia
rng = rng_create()
A = zeros(Float32, 10, 10)
B = zeros(10, 10)
random_pareto!(rng, A; xm=1f0, alpha=2f0)
random_pareto!(rng, B; xm=2, alpha=3)
```

# See also
- random_pareto
"""
function random_pareto!(rng::RNG, A::AbstractArray{Float64}; xm, alpha)
  xm64 = Float64(xm)
  al64 = Float64(alpha)
  !(xm64 > 0 && al64 > 0) && throw(ArgumentError("require xm > 0 and alpha > 0"))
  return _fill2!(rng, A, Val(:randompack_pareto), xm64, al64, "randompack_pareto failed")
end

function random_pareto!(rng::RNG, A::AbstractArray{Float32}; xm, alpha)
  xm32 = Float32(xm)
  al32 = Float32(alpha)
  !(xm32 > 0 && al32 > 0) && throw(ArgumentError("require xm > 0 and alpha > 0"))
  return _fill2!(rng, A, Val(:randompack_paretof), xm32, al32, "randompack_paretof failed")
end

# --- weibull
"""
    random_weibull(rng; shape, scale=1)
    random_weibull(rng, dims; shape, scale=1)
    random_weibull(rng, Float32, dims; shape, scale=1)

Draw Weibull variates with shape `shape` (positive) and scale `scale` (positive). The
scalar form returns a single draw and the dims form allocates and returns an array; dims
can be given as a sequence or a tuple of integers. By default Float64 variates are drawn;
use `Float32` explicitly to draw single-precision variates.

# Examples
```julia
rng = rng_create()
x = random_weibull(rng; shape=2, scale=3)
X = random_weibull(rng, 10; shape=1.5)
Y = random_weibull(rng, Float32, (4,4); shape=2, scale=0.5)
```

# See also
- random_weibull!
"""
random_weibull(rng::RNG; shape, scale=1) =
  _scalar_and_fill(rng, Float64, (r,x)->random_weibull!(r, x; shape=shape, scale=scale))

random_weibull(rng::RNG, dims::Int...; shape, scale=1) =
  _alloc_and_fill(rng, Float64, (r,x)->random_weibull!(r, x; shape=shape, scale=scale),
                  dims)

random_weibull(rng::RNG, dims::Tuple{Vararg{Int}}; shape, scale=1) =
  _alloc_and_fill(rng, Float64, (r,x)->random_weibull!(r, x; shape=shape, scale=scale),
                  dims)

# explicit Float64 (kept for Base-like symmetry)
random_weibull(rng::RNG, ::Type{Float64}; shape, scale=1) =
  random_weibull(rng; shape=shape, scale=scale)

random_weibull(rng::RNG, ::Type{Float64}, dims::Int...; shape, scale=1) =
  random_weibull(rng, dims...; shape=shape, scale=scale)

random_weibull(rng::RNG, ::Type{Float64}, dims::Tuple{Vararg{Int}}; shape,
  scale=1) =
  random_weibull(rng, dims; shape=shape, scale=scale)

# Float32
random_weibull(rng::RNG, ::Type{Float32}; shape, scale=1) =
  _scalar_and_fill(rng, Float32, (r,x)->random_weibull!(r, x; shape=shape, scale=scale))

random_weibull(rng::RNG, ::Type{Float32}, dims::Int...; shape, scale=1) =
  _alloc_and_fill(rng, Float32, (r,x)->random_weibull!(r, x; shape=shape, scale=scale),
                  dims)

random_weibull(rng::RNG, ::Type{Float32}, dims::Tuple{Vararg{Int}}; shape,
  scale=1) =
  _alloc_and_fill(rng, Float32, (r,x)->random_weibull!(r, x; shape=shape, scale=scale),
                  dims)

"""
    random_weibull!(rng, A; shape, scale=1)

Fill an existing scalar or array `A` with Weibull variates with shape `shape` and scale
`scale` (both positive). `A` must have element type `Float32` or `Float64`.

# Examples
```julia
rng = rng_create()
A = zeros(Float32, 10, 10)
B = zeros(10, 10)
random_weibull!(rng, A; shape=2f0, scale=0.5f0)
random_weibull!(rng, B; shape=1, scale=2)
```

# See also
- random_weibull
"""
function random_weibull!(rng::RNG, A::AbstractArray{Float64}; shape, scale=1)
  sh64 = Float64(shape)
  sc64 = Float64(scale)
  !(sh64 > 0 && sc64 > 0) && throw(ArgumentError("require shape > 0 and scale > 0"))
  return _fill2!(rng, A, Val(:randompack_weibull), sh64, sc64, "randompack_weibull failed")
end

function random_weibull!(rng::RNG, A::AbstractArray{Float32}; shape, scale=1)
  sh32 = Float32(shape)
  sc32 = Float32(scale)
  !(sh32 > 0 && sc32 > 0) && throw(ArgumentError("require shape > 0 and scale > 0"))
  return _fill2!(rng, A, Val(:randompack_weibullf), sh32, sc32, "randompack_weibullf failed")
end

"""
    random_mvn!(rng, X, Sigma)
    random_mvn!(rng, X, Sigma; mu)

Fill `X` with multivariate normal draws.

`X` is an `n×d` `Float64` matrix of output samples, one draw per row. `Sigma` is the `d×d`
covariance matrix which must be positive semidefinite. If `mu` is provided, it is the mean
vector of length `d`; if it is absent, a zero mean is used. If `Sigma` and `mu` are not 
`Float64` they are converted to `Float64` internally.

# Examples
```julia
rng = rng_create()
Sigma = [1.0 0.3; 0.3 2.0]
mu = [2.0, 3.0]
X = zeros(100, 2)
random_mvn!(rng, X, Sigma)
random_mvn!(rng, X, Sigma; mu)
```

# See also
- random_mvn
"""
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
      ccall(_sym(:randompack_mvn), Bool,
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

"""
    X = random_mvn!(rng, n, Sigma)
    X = random_mvn!(rng, n, Sigma; mu)

Draw `n` multivariate normal variates with covariance `Sigma` and optional mean `mu`. If
`mu` is absent, a zero mean is used. Returns an `n×d` matrix of Float64 draws, where `d =
size(Sigma,1)`. `Sigma` must be a positive semidefinite square matrix, and `mu` (if
provided) must have length `d`.


# Examples
```julia
rng = rng_create()
Sigma = [1.0 0.3; 0.3 2.0]
X = random_mvn(rng, 100, Sigma)
Y = random_mvn(rng, 50, Sigma; mu=[1,2])
```

# See also
- random_mvn!
"""
function random_mvn(rng::RNG, n::Integer, Sigma::AbstractMatrix{<:Real};
  mu::Union{Nothing,AbstractVector{<:Real}}=nothing)
  X = Array{Float64}(undef, n, size(Sigma, 1))
  random_mvn!(rng, X, Sigma; mu=mu)
  return X
end

