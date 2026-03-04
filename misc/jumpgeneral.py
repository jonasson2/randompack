#!/usr/bin/env python3
"""
Compute jump polynomial for xoshiro256++ for arbitrary N steps.
Uses square-and-multiply in GF(2)[x] / char_poly(x).

We don't need char_poly explicitly — we work directly with the
transition matrix T and compute T^N via matrix square-and-multiply,
then extract the jump polynomial from the resulting matrix.
"""

def rotl64(x, k):
    return ((x << k) | (x >> (64 - k))) & 0xFFFFFFFFFFFFFFFF

def xoshiro256pp_next(s):
    s = list(s)
    t = (s[1] << 17) & 0xFFFFFFFFFFFFFFFF
    s[2] ^= s[0]; s[3] ^= s[1]
    s[1] ^= s[2]; s[0] ^= s[3]
    s[2] ^= t
    s[3] = rotl64(s[3], 45)
    return s

def int_to_state256(v):
    m = (1 << 64) - 1
    return [v & m, (v >> 64) & m, (v >> 128) & m, (v >> 192) & m]

def state_to_int256(s):
    return s[0] | (s[1] << 64) | (s[2] << 128) | (s[3] << 192)

def int_to_state128(v):
    m = (1 << 64) - 1
    return [v & m, (v >> 64) & m]

def state_to_int128(s):
    return s[0] | (s[1] << 64)

def mat_mul(A, B, nbits):
    C = []
    for j in range(nbits):
        col = 0; b = B[j]
        while b:
            lsb = b & (-b)
            col ^= A[lsb.bit_length() - 1]
            b ^= lsb
        C.append(col)
    return C

def mat_pow(T, N, nbits):
    """Compute T^N over GF(2) using square-and-multiply."""
    # Identity matrix
    result = [1 << i for i in range(nbits)]
    base = list(T)
    while N:
        if N & 1:
            result = mat_mul(result, base, nbits)
        base = mat_mul(base, base, nbits)
        N >>= 1
    return result

def apply_matrix(M, v, nbits):
    result = 0; i = 0
    while v:
        if v & 1: result ^= M[i]
        v >>= 1; i += 1
    return result

def extract_jump_poly(M, T, nbits):
    """
    Extract jump polynomial r (as nbits-bit int) from matrix M = T^N.
    Solves: for each bit i, sum_{k: r_k=1} bit_i(T^k * e_0) = bit_i(M * e_0)
    via Gaussian elimination over GF(2).
    """
    # Precompute T^k * e_0 for k = 0..nbits-1
    T_pow_e0 = []
    cur = 1
    for k in range(nbits):
        T_pow_e0.append(cur)
        cur = apply_matrix(T, cur, nbits)

    target = M[0]  # M * e_0 = column 0 of M

    # Build row matrix: row_mat[i] bit k = bit i of T^k*e_0
    row_mat = [0] * nbits
    for k, val in enumerate(T_pow_e0):
        v = val; i = 0
        while v:
            if v & 1: row_mat[i] |= 1 << k
            v >>= 1; i += 1

    rhs = [(target >> i) & 1 for i in range(nbits)]

    # Gaussian elimination over GF(2)
    for col in range(nbits):
        pivot = next((r for r in range(col, nbits) if (row_mat[r] >> col) & 1), None)
        if pivot is None:
            raise ValueError(f"Singular at column {col}")
        row_mat[col], row_mat[pivot] = row_mat[pivot], row_mat[col]
        rhs[col], rhs[pivot] = rhs[pivot], rhs[col]
        for r in range(nbits):
            if r != col and (row_mat[r] >> col) & 1:
                row_mat[r] ^= row_mat[col]
                rhs[r] ^= rhs[col]

    return sum(rhs[i] << i for i in range(nbits))

def apply_jump(r_int, s_list, step_func, nbits):
    s = list(s_list)
    result = [0] * len(s)
    for i in range(nbits):
        if r_int & (1 << i):
            for k in range(len(s)):
                result[k] ^= s[k]
        s = step_func(s)
    return result

def compute_jump(T, N, label, nbits):
    print(f"Computing jump for N = {label} ({N.bit_length()} bits)...")
    M = mat_pow(T, N, nbits)
    r = extract_jump_poly(M, T, nbits)
    mask64 = (1 << 64) - 1
    words = [(r >> (64*j)) & mask64 for j in range(nbits // 64)]
    if len(words) == 2:
        print(f"  0x{words[0]:016x}, 0x{words[1]:016x}")
    else:
        print(f"  0x{words[0]:016x}, 0x{words[1]:016x},")
        print(f"  0x{words[2]:016x}, 0x{words[3]:016x}")
    return r

# Build T for xoshiro256
print("Building transition matrix T (xoshiro256++)...")
T = []
for i in range(256):
    s = int_to_state256(1 << i)
    T.append(state_to_int256(xoshiro256pp_next(s)))

JUMP_128 = [0x180ec6d33cfd0aba, 0xd5a61266f0c9392c,
            0xa9582618e03fc9aa, 0x39abdc4529b1661c]
JUMP_192 = [0x76e15d3efefdcbbf, 0xc5004e441c522fb3,
            0x77710069854ee241, 0x39109bb02acbe635]

vigna_128 = sum(w << (64*j) for j, w in enumerate(JUMP_128))
vigna_192 = sum(w << (64*j) for j, w in enumerate(JUMP_192))

test = [0x123456789ABCDEF0, 0xFEDCBA9876543210,
        0x0F1E2D3C4B5A6978, 0x8796A5B4C3D2E1F0]

print()
r32  = compute_jump(T, 2**32,  "2^32", 256)
r64  = compute_jump(T, 2**64,  "2^64", 256)
r96  = compute_jump(T, 2**96,  "2^96", 256)
r128 = compute_jump(T, 2**128, "2^128", 256)
r192 = compute_jump(T, 2**192, "2^192", 256)
r254 = compute_jump(T, 2**254, "2^254", 256)

print()
print(f"2^128 matches Vigna: {apply_jump(r128, test, xoshiro256pp_next, 256) == apply_jump(vigna_128, test, xoshiro256pp_next, 256)}")
print(f"2^192 matches Vigna: {apply_jump(r192, test, xoshiro256pp_next, 256) == apply_jump(vigna_192, test, xoshiro256pp_next, 256)}")

# --- Extra verification for 2^64 ---
import random
random.seed(0)
print("\nVerifying 2^64 polynomial against T^(2^64) matrix on 10 random states...")
M64 = mat_pow(T, 2**64, 256)
all_ok = True
for trial in range(10):
    s = [random.getrandbits(64) for _ in range(4)]
    # Ensure non-zero
    while s == [0,0,0,0]:
        s = [random.getrandbits(64) for _ in range(4)]
    matrix_result  = int_to_state256(apply_matrix(M64, state_to_int256(s), 256))
    poly_result    = apply_jump(r64, s, xoshiro256pp_next, 256)
    ok = matrix_result == poly_result
    print(f"  trial {trial+1}: {'OK' if ok else 'FAIL'}")
    if not ok:
        all_ok = False
print(f"All passed: {all_ok}")

def xoroshiro128pp_next(s):
    s = list(s)
    s0 = s[0]
    s1 = s[1]
    s1 ^= s0
    s[0] = rotl64(s0, 49) ^ s1 ^ ((s1 << 21) & 0xFFFFFFFFFFFFFFFF)
    s[1] = rotl64(s1, 28)
    return s

def xorshift128p_next(s):
    s = list(s)
    t = (s[1] ^ (s[1] << 23)) & 0xFFFFFFFFFFFFFFFF
    s[1] = s[0]
    s[0] = t ^ s[0] ^ (t >> 18) ^ (s[0] >> 5)
    return s

print("\nBuilding transition matrix T (xoroshiro128++)...")
T128 = []
for i in range(128):
    s = int_to_state128(1 << i)
    T128.append(state_to_int128(xoroshiro128pp_next(s)))

print()
compute_jump(T128, 2**32, "2^32", 128)
compute_jump(T128, 2**64, "2^64", 128)
compute_jump(T128, 2**96, "2^96", 128)

print("\nBuilding transition matrix T (xorshift128+)...")
T128 = []
for i in range(128):
    s = int_to_state128(1 << i)
    T128.append(state_to_int128(xorshift128p_next(s)))

print()
compute_jump(T128, 2**32, "2^32", 128)
compute_jump(T128, 2**64, "2^64", 128)
compute_jump(T128, 2**96, "2^96", 128)

# what was the test checking?

# 12:00 PM
# The test in our Python program was checking that apply_jump(r, s) — which applies the polynomial by stepping the generator 256 times and XORing in the state when the corresponding polynomial bit is set — gives the same result as apply_matrix(M, s) — which directly multiplies the explicitly computed matrix T^N by the state vector.

# These are two completely independent computations of the same thing, so agreement on 10 random states gives high confidence that extract_jump_poly correctly extracted the polynomial coefficients from the matrix.
