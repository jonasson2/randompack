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


def test_squares_set_state():
    engs = rp.engines()
    if "squares" not in engs:
        return
    rng1 = rp.Rng("squares")
    rng2 = rp.Rng("squares")
    rng1.squares_set_state(3, 4)
    rng2.squares_set_state(3, 4)
    x = rng1.unif(100)
    y = rng2.unif(100)
    assert np.array_equal(x, y)


def test_philox_set_state():
    engs = rp.engines()
    if "philox" not in engs:
        return
    rng1 = rp.Rng("philox")
    rng2 = rp.Rng("philox")
    rng1.philox_set_state([1, 2, 3, 4], [5, 6])
    rng2.philox_set_state([1, 2, 3, 4], [5, 6])
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
