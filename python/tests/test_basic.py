import numpy as np
import pytest

import randompack as rp


def pick_engine():
    engs = rp.engines()
    assert isinstance(engs, dict)
    assert len(engs) > 0
    if "pcg64" in engs:
        return "pcg64"
    if "x256++simd" in engs:
        return "x256++simd"
    return next(iter(engs.keys()))


def test_engines_nonempty():
    engs = rp.engines()
    assert isinstance(engs, dict)
    assert len(engs) > 0


def test_create_default():
    rng = rp.Rng()
    x = rng.unif(3)
    assert isinstance(x, np.ndarray)
    assert x.shape == (3,)


def test_create_explicit_engine():
    rng = rp.Rng(pick_engine())
    x = rng.unif(3)
    assert isinstance(x, np.ndarray)
    assert x.shape == (3,)


def test_create_unknown_engine():
    with pytest.raises(Exception) as err:
        rp.Rng("no_such_engine")
    assert "spelling error" in str(err.value)


def test_rng_capsule():
    rng = rp.Rng()
    capsule = rng.__randompack_capsule__()
    assert type(capsule).__name__ == "PyCapsule"
    assert rp.RNG_CAPSULE_NAME == "randompack.randompack_rng"
