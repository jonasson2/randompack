@testset "rng_create" begin
  # Default engine
  rng = rng_create()
  @test rng isa Randompack.RNG
  @test rng.ptr != C_NULL

  # Explicit engine (same default)
  rng2 = rng_create("x256++simd")
  @test rng2 isa Randompack.RNG
  @test rng2.ptr != C_NULL

  # Unknown engine should error with spelling message
  err = @test_throws ErrorException rng_create("this_engine_does_not_exist")
  exc = err isa Test.Pass ? err.value : err
  msg = sprint(showerror, exc)
  @test occursin("unknown engine name", msg) || occursin("unknown RNG engine", msg)
  @test occursin("spelling error", msg)
end

@testset "rng_seed!" begin
  rng1 = rng_create("x256++simd")
  rng_seed!(rng1, 42)
  x1 = random_unif(rng1)
  rng2 = rng_create("x256++simd")
  rng_seed!(rng2, 42)
  x2 = random_unif(rng2)
  @test x1 == x2
  rng3 = rng_create("x256++simd")
  rng_seed!(rng3, 43)
  x3 = random_unif(rng3)
  @test x1 != x3
  rng4 = rng_create("sfc64")
  rng_seed!(rng4, 42)
  x4 = random_unif(rng4)
  @test x1 != x4
  rng5 = rng_create("x256++simd")
  rng_seed!(rng5, 42; spawn_key=[42, 7])
  x5 = random_unif(rng5)
  rng6 = rng_create("x256++simd")
  rng_seed!(rng6, 42; spawn_key=[42, 13])
  x6 = random_unif(rng6)
  @test x5 != x6
  has_pcg = true
  try
    rng_create("pcg64")
  catch
    has_pcg = false
  end
  if has_pcg
    rnga = rng_create("pcg64")
    rng_seed!(rnga, 42)
    xa = random_unif(rnga)
    rngb = rng_create("pcg64")
    rng_seed!(rngb, 42)
    xb = random_unif(rngb)
    @test xa == xb
  else
    @test true
  end
  @test_throws ArgumentError rng_seed!(rng1, 2^33)
  @test_throws ArgumentError rng_seed!(rng1, 1; spawn_key=[big(1) << 33])
  rngnull = Randompack.RNG(C_NULL)
  @test_throws ErrorException rng_seed!(rngnull, 1)
end

@testset "duplicate" begin
  rng = rng_create()
  rng_seed!(rng, 42)
  rng2 = Randompack.duplicate(rng)
  x1 = random_unif(rng)
  x2 = random_unif(rng2)
  @test x1 == x2
end

@testset "randomize!" begin
  rng = rng_create()
  Randompack.randomize!(rng)
  x = random_unif(rng)
  @test isfinite(x)
end

@testset "full_mantissa!" begin
  rng = rng_create()
  Randompack.full_mantissa!(rng, true)
  x = random_unif(rng)
  @test isfinite(x)
  Randompack.full_mantissa!(rng, false)
  y = random_unif(rng)
  @test isfinite(y)
end

@testset "jump!" begin
  rng1 = rng_create()
  rng_seed!(rng1, 123)
  rng2 = rng_create()
  rng_seed!(rng2, 123)
  Randompack.jump!(rng2, 128)
  x1 = random_unif(rng1)
  x2 = random_unif(rng2)
  @test x1 != x2
  @test_throws ErrorException Randompack.jump!(rng1, 129)
  @test_throws ErrorException Randompack.jump!(rng1, 254)
end

@testset "pcg64_advance!" begin
  has_pcg = true
  try
    rng_create("pcg64")
  catch
    has_pcg = false
  end
  if has_pcg
    rng1 = rng_create("pcg64")
    rng2 = rng_create("pcg64")
    state = [0x5bee00f1ac1e7b4d, 0x786df8ae32b3fe64,
             0x26dbcfc7823f9c3b, 0x0d4e48fee886333a]
    Randompack.set_state!(rng1; state=state)
    Randompack.set_state!(rng2; state=state)
    Randompack.pcg64_advance!(rng1; delta=[0, 1 << 16])
    Randompack.jump!(rng2, 80)
    @test random_unif(rng1) == random_unif(rng2)
    @test_throws ArgumentError Randompack.pcg64_advance!(rng1; delta=[1])
    rng3 = rng_create("squares")
    @test_throws ErrorException Randompack.pcg64_advance!(rng3; delta=[1, 0])
  else
    @test true
  end
end

@testset "engines" begin
  out = Randompack.engines()
  @test haskey(out, :engine)
  @test haskey(out, :description)
  @test length(out.engine) == length(out.description)
  @test length(out.engine) > 0
end

@testset "set_state!" begin
  rng1 = rng_create("squares")
  rng2 = rng_create("squares")
  Randompack.set_state!(rng1; state=[1, 2])
  Randompack.set_state!(rng2; state=[1, 2])
  x1 = random_unif(rng1)
  x2 = random_unif(rng2)
  @test x1 == x2
end

@testset "squares_set_key!" begin
  rng1 = rng_create("squares")
  rng2 = rng_create("squares")
  Randompack.set_state!(rng1; state=[3, 0])
  Randompack.squares_set_key!(rng1; key=4)
  Randompack.set_state!(rng2; state=[3, 0])
  Randompack.squares_set_key!(rng2; key=4)
  x1 = random_unif(rng1)
  x2 = random_unif(rng2)
  @test x1 == x2
  @test_throws ArgumentError Randompack.squares_set_key!(rng1; key=big(1) << 65)
end

@testset "philox_set_key!" begin
  has_philox = true
  try
    rng_create("philox")
  catch
    has_philox = false
  end
  if has_philox
    rng1 = rng_create("philox")
    rng2 = rng_create("philox")
    Randompack.set_state!(rng1; state=[1, 2, 3, 4, 0, 0])
    Randompack.philox_set_key!(rng1; key=[5, 6])
    Randompack.set_state!(rng2; state=[1, 2, 3, 4, 0, 0])
    Randompack.philox_set_key!(rng2; key=[5, 6])
    x1 = random_unif(rng1)
    x2 = random_unif(rng2)
    @test x1 == x2
  else
    @test true
  end
end

@testset "pcg64_set_inc!" begin
  has_pcg = true
  try
    rng_create("pcg64")
  catch
    has_pcg = false
  end
  if has_pcg
    rng1 = rng_create("pcg64")
    rng2 = rng_create("pcg64")
    Randompack.set_state!(rng1; state=[1, 0, 1, 0])
    Randompack.set_state!(rng2; state=[1, 0, 1, 0])
    Randompack.pcg64_set_inc!(rng1; inc=[3, 5])
    Randompack.pcg64_set_inc!(rng2; inc=[3, 5])
    x1 = random_unif(rng1)
    x2 = random_unif(rng2)
    @test x1 == x2
    @test_throws ErrorException Randompack.pcg64_set_inc!(rng1; inc=[2, 5])
  else
    @test true
  end
end

@testset "cwg128_set_weyl!" begin
  rng1 = rng_create("cwg128")
  rng2 = rng_create("cwg128")
  Randompack.set_state!(rng1; state=[1, 0, 7, 0, 11, 0, 13, 0])
  Randompack.set_state!(rng2; state=[1, 0, 7, 0, 11, 0, 13, 0])
  Randompack.cwg128_set_weyl!(rng1; weyl=[3, 5])
  Randompack.cwg128_set_weyl!(rng2; weyl=[3, 5])
  x1 = random_unif(rng1)
  x2 = random_unif(rng2)
  @test x1 == x2
  @test_throws ErrorException Randompack.cwg128_set_weyl!(rng1; weyl=[2, 5])
end

@testset "sfc64_set_abc!" begin
  rng1 = rng_create("sfc64")
  rng2 = rng_create("sfc64")
  Randompack.set_state!(rng1; state=[1, 2, 3, 17])
  Randompack.set_state!(rng2; state=[1, 2, 3, 17])
  Randompack.sfc64_set_abc!(rng1; abc=[7, 11, 13])
  Randompack.sfc64_set_abc!(rng2; abc=[7, 11, 13])
  x1 = random_unif(rng1)
  x2 = random_unif(rng2)
  @test x1 == x2
end

@testset "chacha_set_nonce!" begin
  has_chacha = true
  try
    rng_create("chacha20")
  catch
    has_chacha = false
  end
  if has_chacha
    rng1 = rng_create("chacha20")
    rng2 = rng_create("chacha20")
    rng_seed!(rng1, 123)
    rng_seed!(rng2, 123)
    Randompack.chacha_set_nonce!(rng1; nonce=[7, 11, 13])
    Randompack.chacha_set_nonce!(rng2; nonce=[7, 11, 13])
    x1 = random_unif(rng1)
    x2 = random_unif(rng2)
    @test x1 == x2
  else
    @test true
  end
end

@testset "serialize" begin
  rng = rng_create()
  rng_seed!(rng, 42)
  bytes = Randompack.serialize(rng)
  rng2 = rng_create()
  Randompack.deserialize!(rng2, bytes)
  x1 = random_unif(rng)
  x2 = random_unif(rng2)
  @test x1 == x2
end
