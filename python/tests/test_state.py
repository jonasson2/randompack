import numpy as np
import pytest

import randompack as rp


def pick_engine():
    engs = rp.engines()
    if "pcg64" in engs:
        return "pcg64"
    if "x256++simd" in engs:
        return "x256++simd"
    return next(iter(engs.keys()))


def test_seed_determinism():
    rng = rp.Rng(pick_engine())
    rng.seed(777)
    x = rng.unif(100)
    rng.seed(777)
    y = rng.unif(100)
    assert np.array_equal(x, y)
    rng.seed(778)
    z = rng.unif(100)
    assert np.any(x != z)


def test_duplicate():
    rng = rp.Rng(pick_engine())
    rng.seed(999)
    dup = rng.duplicate()
    x = rng.unif(100)
    y = dup.unif(100)
    assert np.array_equal(x, y)
    rng.unif(100)
    x = rng.unif(100)
    y = dup.unif(100)
    assert np.any(x != y)


def test_full_mantissa_toggle():
    rng = rp.Rng(pick_engine())
    rng.full_mantissa(True)
    rng.full_mantissa(False)


def test_bitexact_create():
    rng = rp.Rng(pick_engine(), bitexact=True)
    rng.unif(10)


def test_serialize_deserialize():
    rng = rp.Rng(pick_engine())
    rng.seed(555)
    rng.unif(100)
    state = rng.serialize()
    y = rng.unif(100)
    rng2 = rp.Rng(pick_engine())
    rng2.deserialize(state)
    z = rng2.unif(100)
    assert np.array_equal(y, z)


def test_squares_set_key():
    engs = rp.engines()
    if "squares" not in engs:
        return
    rng1 = rp.Rng("squares")
    rng2 = rp.Rng("squares")
    rng1.set_state([3, 0])
    rng1.squares_set_key(4)
    rng2.set_state([3, 0])
    rng2.squares_set_key(4)
    x = rng1.unif(100)
    y = rng2.unif(100)
    assert np.array_equal(x, y)


def test_philox_set_key():
    engs = rp.engines()
    if "philox" not in engs:
        return
    rng1 = rp.Rng("philox")
    rng2 = rp.Rng("philox")
    rng1.set_state([1, 2, 3, 4, 0, 0])
    rng1.philox_set_key([5, 6])
    rng2.set_state([1, 2, 3, 4, 0, 0])
    rng2.philox_set_key([5, 6])
    x = rng1.unif(100)
    y = rng2.unif(100)
    assert np.array_equal(x, y)


def test_pcg64_set_inc():
    engs = rp.engines()
    if "pcg64" not in engs:
        return
    rng1 = rp.Rng("pcg64")
    rng2 = rp.Rng("pcg64")
    state = [1, 0, 1, 0]
    rng1.set_state(state)
    rng2.set_state(state)
    rng1.pcg64_set_inc([3, 5])
    rng2.pcg64_set_inc([3, 5])
    x = rng1.unif(100)
    y = rng2.unif(100)
    assert np.array_equal(x, y)
    with pytest.raises(Exception):
        rng1.pcg64_set_inc([2, 5])


def test_cwg128_set_weyl():
    engs = rp.engines()
    if "cwg128" not in engs:
        return
    rng1 = rp.Rng("cwg128")
    rng2 = rp.Rng("cwg128")
    state = [1, 0, 7, 0, 11, 0, 13, 0]
    rng1.set_state(state)
    rng2.set_state(state)
    rng1.cwg128_set_weyl([3, 5])
    rng2.cwg128_set_weyl([3, 5])
    x = rng1.unif(100)
    y = rng2.unif(100)
    assert np.array_equal(x, y)
    with pytest.raises(Exception):
        rng1.cwg128_set_weyl([2, 5])


def test_sfc64_set_abc():
    engs = rp.engines()
    if "sfc64" not in engs:
        return
    rng1 = rp.Rng("sfc64")
    rng2 = rp.Rng("sfc64")
    rng1.set_state([1, 2, 3, 17])
    rng2.set_state([1, 2, 3, 17])
    rng1.sfc64_set_abc([7, 11, 13])
    rng2.sfc64_set_abc([7, 11, 13])
    x = rng1.unif(100)
    y = rng2.unif(100)
    assert np.array_equal(x, y)


def test_chacha_set_nonce():
    engs = rp.engines()
    if "chacha20" not in engs:
        return
    rng1 = rp.Rng("chacha20")
    rng2 = rp.Rng("chacha20")
    rng1.seed(123)
    rng2.seed(123)
    rng1.chacha_set_nonce([7, 11, 13])
    rng2.chacha_set_nonce([7, 11, 13])
    x = rng1.unif(100)
    y = rng2.unif(100)
    assert np.array_equal(x, y)


def test_jump():
    rng1 = rp.Rng()
    rng2 = rp.Rng()
    rng1.seed(123)
    rng2.seed(123)
    a = rng1.unif(5)
    rng2.jump(128)
    b = rng2.unif(5)
    assert not np.array_equal(a, b)
    with pytest.raises(Exception):
        rng1.jump(129)
    with pytest.raises(Exception):
        rng1.jump(254)


def test_pcg64_advance():
    engs = rp.engines()
    if "pcg64" not in engs:
        return
    state = [
        0x5bee00f1ac1e7b4d,
        0x786df8ae32b3fe64,
        0x26dbcfc7823f9c3b,
        0x0d4e48fee886333a,
    ]
    rng1 = rp.Rng("pcg64")
    rng2 = rp.Rng("pcg64")
    rng1.set_state(state)
    rng2.set_state(state)
    rng1.pcg64_advance([0, 1 << 16])
    rng2.jump(80)
    assert rng1.unif() == rng2.unif()
    with pytest.raises(Exception):
        rng1.pcg64_advance([1])
    with pytest.raises(Exception):
        rp.Rng("squares").pcg64_advance([1, 0])
