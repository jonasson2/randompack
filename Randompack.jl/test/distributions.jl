# Helper: exercise alloc and in-place paths for a distribution.
function test_dist(allocf, fillf; rng, T, ok_pred, kwargs...)
  x = allocf(rng, T; kwargs...)
  @test x isa T
  @test ok_pred(x; kwargs...)
  v = allocf(rng, T, 25; kwargs...)
  @test v isa Vector{T}
  @test length(v) == 25
  @test ok_pred(v; kwargs...)
  m = allocf(rng, T, 3, 4; kwargs...)
  @test m isa Matrix{T}
  @test size(m) == (3, 4)
  @test ok_pred(m; kwargs...)
  t = allocf(rng, T, (2, 3, 2); kwargs...)
  @test t isa Array{T, 3}
  @test size(t) == (2, 3, 2)
  @test ok_pred(t; kwargs...)
  b = Array{T}(undef, 3, 4)
  fillf(rng, b; kwargs...)
  @test ok_pred(b; kwargs...)
  p = Array{T}(undef, 6, 4)
  v = @view p[1:2:5, :]
  fillf(rng, v; kwargs...)
  @test ok_pred(v; kwargs...)
end

# Helper: exercise integer draws with unbounded and bounded paths.
function test_int_dist(allocf, fillf; rng, T, engine)
  n = 100
  x = allocf(rng, T)
  @test x isa T
  v = allocf(rng, T, n)
  @test v isa Vector{T}
  @test length(v) == n
  m = allocf(rng, T, 10, 10)
  @test m isa Matrix{T}
  @test size(m) == (10, 10)
  a = Array{T}(undef, 10, 10)
  fillf(rng, a)
  @test eltype(a) == T
  @test size(a) == (10, 10)
  p = Array{T}(undef, 20, 10)
  w = @view p[1:2:19, :]
  fillf(rng, w)
  @test size(w) == (10, 10)
  @test length(unique(v)) > 1
  if T <: Signed
    @test any(<(zero(T)), v)
    @test any(>(zero(T)), v)
  end
  if T <: Signed
    utype = unsigned(T)
    rng1 = rng_create(engine)
    rng_seed!(rng1, 123)
    rng2 = rng_create(engine)
    rng_seed!(rng2, 123)
    u = allocf(rng1, utype, n)
    x = allocf(rng2, T, n)
    @test reinterpret(utype, x) == u
  end
  if T <: Signed
    r1 = T(-2):T(1)
    v1 = allocf(rng, r1, n)
    @test length(v1) == n
    @test all(in(r1), v1)
    x1 = allocf(rng, r1)
    @test x1 in r1
    a1 = Array{T}(undef, n)
    fillf(rng, a1, r1)
    @test all(in(r1), a1)
    m1 = Array{T}(undef, 10, 10)
    fillf(rng, m1, r1)
    @test all(in(r1), m1)
  end
  r2 = T(5):T(8)
  v2 = allocf(rng, r2, n)
  @test length(v2) == n
  @test all(in(r2), v2)
  x2 = allocf(rng, r2)
  @test x2 in r2
  a2 = Array{T}(undef, n)
  fillf(rng, a2, r2)
  @test all(in(r2), a2)
  m2 = Array{T}(undef, 10, 10)
  fillf(rng, m2, r2)
  @test all(in(r2), m2)
end

# Predicate: values in [0,1).
function unif_ok(x; a::Real=0, b::Real=1)
  if x isa Number
    return a <= x < b
  end
  return all((a .<= x) .& (x .< b))
end

# Predicate: finite values only.
function normal_ok(x; kwargs...)
  if x isa Number
    return isfinite(x)
  end
  return all(isfinite, x)
end

@testset "random_mvn" begin
  rng = rng_create()
  rng_seed!(rng, 42)
  Sigma = [1.0 0.2; 0.2 2.0]
  x = random_mvn(rng, 5, Sigma)
  @test size(x) == (5, 2)
  @test all(isfinite, x)
  mu = [1.0, 2.0]
  y = random_mvn(rng, 3, Sigma; mu=mu)
  @test size(y) == (3, 2)
  @test all(isfinite, y)
  z = Matrix{Float64}(undef, 4, 2)
  random_mvn!(rng, z, Sigma; mu=mu)
  @test size(z) == (4, 2)
  @test all(isfinite, z)
  @test_throws ArgumentError random_mvn(rng, -1, Sigma)
  @test_throws ArgumentError random_mvn(rng, 2, [1.0 0.0 0.0; 0.0 1.0 0.0])
  @test_throws ArgumentError random_mvn(rng, 2, Sigma; mu=[1.0])
end

@testset "random_perm" begin
  rng = rng_create()
  rng_seed!(rng, 42)
  p = random_perm(rng, 5, Int32)
  @test p isa Vector{Int32}
  @test length(p) == 5
  @test sort(p) == Int32[1, 2, 3, 4, 5]
  q = Vector{UInt16}(undef, 6)
  random_perm!(rng, q)
  @test sort(q) == UInt16[1, 2, 3, 4, 5, 6]
  @test_throws ArgumentError random_perm(rng, 300, UInt8)
end

@testset "random_sample" begin
  rng = rng_create()
  rng_seed!(rng, 42)
  s = random_sample(rng, 10, 4, Int16)
  @test s isa Vector{Int16}
  @test length(s) == 4
  @test all(1 .<= s .<= 10)
  @test length(unique(s)) == 4
  t = Vector{UInt8}(undef, 5)
  random_sample!(rng, t, 12)
  @test all(1 .<= t .<= 12)
  @test length(unique(t)) == 5
  @test_throws ArgumentError random_sample(rng, 300, 5, UInt8)
  @test_throws ArgumentError random_sample!(rng, Vector{UInt8}(undef, 5), 300)
end

@testset "random_unif" begin
  rng = rng_create()
  rng_seed!(rng, 42)
  x0 = random_unif(rng)
  @test x0 isa Float64
  @test unif_ok(x0)
  test_dist(random_unif, random_unif!; rng=rng, T=Float32, ok_pred=unif_ok)
  test_dist(random_unif, random_unif!; rng=rng, T=Float64, ok_pred=unif_ok,
            a=2.0, b=3.5)
  @test_throws ArgumentError random_unif!(rng, Array{Float64}(undef, 2, 2); a=1, b=1)
  @test_throws ArgumentError random_unif!(rng, Array{Float64}(undef, 2, 2); a=2, b=1)
  @test_throws ArgumentError random_unif!(rng, Array{Float32}(undef, 4, 4);
                                          a=1e300, b=2e300)
end

@testset "random_normal" begin
  rng = rng_create()
  rng_seed!(rng, 42)
  x0 = random_normal(rng)
  @test x0 isa Float64
  @test normal_ok(x0)
  test_dist(random_normal, random_normal!; rng=rng, T=Float64, ok_pred=normal_ok,
            mu=2.0, sigma=3.0)
  test_dist(random_normal, random_normal!; rng=rng, T=Float32, ok_pred=normal_ok)
  @test_throws ArgumentError random_normal!(rng, Array{Float64}(undef, 2, 2); sigma=0)
  @test_throws ArgumentError random_normal!(rng, Array{Float64}(undef, 2, 2); sigma=-1)
end

@testset "random_skew_normal" begin
  rng = rng_create()
  rng_seed!(rng, 42)
  test_dist(random_skew_normal, random_skew_normal!; rng=rng, T=Float64,
            ok_pred=normal_ok, mu=1.0, sigma=2.0, alpha=-0.5)
  test_dist(random_skew_normal, random_skew_normal!; rng=rng, T=Float32,
            ok_pred=normal_ok, alpha=1.5)
  @test_throws ArgumentError random_skew_normal!(rng, Array{Float64}(undef, 2, 2);
                                                sigma=0, alpha=1)
  @test_throws ArgumentError random_skew_normal!(rng, Array{Float64}(undef, 2, 2);
                                                sigma=-1, alpha=1)
end

@testset "random_int" begin
  engine = "x256++simd"
  types = (Int8, UInt8, Int16, UInt16, Int32, UInt32, Int64, UInt64)
  for T in types
    rng = rng_create(engine)
    rng_seed!(rng, 42)
    test_int_dist(random_int, random_int!; rng=rng, T=T, engine=engine)
  end
end
