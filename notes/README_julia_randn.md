# Julia randn! pipeline (Xoshiro/TaskLocalRNG)

This note summarizes how Julia fills `Array{Float64}` with standard normal values.
Source reference: Julia stdlib `Random/src/normal.jl` and `Random/src/XoshiroSimd.jl`.

Overview
- Two-pass approach: bulk RNG fill + scalar Ziggurat transform.
- SIMD is used only for the RNG bulk fill, not for the Ziggurat loop.

Step 1: Bulk-fill the output array with raw RNG bits
```
GC.@preserve A rand!(rng, UnsafeView{UInt64}(pointer(A), length(A)))
```
Key points:
- `rand!(rng, UnsafeView{UInt64})` calls `xoshiro_bulk_simd` for large buffers, then
  `xoshiro_bulk_nosimd` for the remainder.
- The output `A` memory is filled with raw `UInt64` values (no normal transform yet).

Step 2: Scalar Ziggurat transform pass
```
for i in eachindex(A)
  @inbounds A[i] = _randn(rng, reinterpret(UInt64, A[i]) >>> 12)
end
```
`_randn` fast path (scalar):
- `rabs = Int64(r>>1)`  (sign in LSB)
- `idx = rabs & 0xFF`
- `x = ifelse(r % Bool, -rabs, rabs) * wi[idx+1]`
- If `rabs < ki[idx+1]`: return `x` (about 99.3%).
- Else: call `randn_unlikely(rng, idx, rabs, x)` (tail/triangle).

Slow path (randn_unlikely)
- Uses extra `rand(rng)` draws and log/exp math for the tail/triangle cases.
- Called inline per element; there is no separate replacement pass.

Performance implications vs randompack
- Julia avoids per-element FFI overhead by keeping the Ziggurat loop in JITed Julia.
- RNG bits are generated in large SIMD batches directly into the output array.
- The transform loop is scalar but tight, with very few branches on the fast path.
