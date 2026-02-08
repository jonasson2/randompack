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
    x = rng.int(50, -2, 3)
    assert np.all((-2 <= x) & (x <= 3))
    m = np.empty((6, 9), dtype=np.int64)
    rng.int(out=m, m=-4, n=-1)
    assert np.all((-4 <= m) & (m <= -1))
