import numpy as np

import randompack as rp


def pick_engine():
    engs = rp.engines()
    if "pcg64" in engs:
        return "pcg64"
    if "x256++simd" in engs:
        return "x256++simd"
    return next(iter(engs.keys()))


def test_int_vector_matrix():
    rng = rp.Rng(pick_engine())
    rng.seed(123)
    x = rng.int(-2, 3, size=50)
    assert np.all((-2 <= x) & (x <= 3))
    m = np.empty((6, 9), dtype=np.int64)
    rng.int(out=m, a=-4, b=-1)
    assert np.all((-4 <= m) & (m <= -1))


def test_perm():
    rng = rp.Rng(pick_engine())
    rng.seed(123)
    p = rng.perm(10)
    assert p.shape == (10,)
    assert np.array_equal(np.sort(p), np.arange(1, 11, dtype=np.int32))
    q = np.empty(7, dtype=np.int32)
    rng.perm(7, out=q)
    assert np.array_equal(np.sort(q), np.arange(1, 8, dtype=np.int32))


def test_sample():
    rng = rp.Rng(pick_engine())
    rng.seed(123)
    s = rng.sample(10, 5)
    assert s.shape == (5,)
    assert np.all((1 <= s) & (s <= 10))
    assert np.unique(s).size == 5
    t = np.empty(6, dtype=np.int32)
    rng.sample(12, 6, out=t)
    assert np.all((1 <= t) & (t <= 12))
    assert np.unique(t).size == 6


def test_raw():
    rng = rp.Rng(pick_engine())
    rng.seed(123)
    x = rng.raw(16)
    assert isinstance(x, (bytes,))
    assert len(x) == 16
