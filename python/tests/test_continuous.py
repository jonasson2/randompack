import numpy as np

import randompack as rp


def pick_engine():
    engs = rp.engines()
    if "pcg64" in engs:
        return "pcg64"
    if "x256++simd" in engs:
        return "x256++simd"
    return next(iter(engs.keys()))


def assert_finite(x):
    assert np.all(np.isfinite(x))


def test_unif_normal_float64():
    rng = rp.Rng(pick_engine())
    rng.seed(123)
    x = rng.unif()
    assert np.isscalar(x)
    x = rng.unif(100)
    assert np.all((0 <= x) & (x <= 1))
    assert_finite(x)
    a = np.empty((7, 11), dtype=np.float64)
    rng.unif(out=a)
    assert np.all((0 <= a) & (a <= 1))
    assert_finite(a)

    rng.seed(123)
    x = rng.unif(100, a=-1.0, b=2.0)
    assert np.all((-1 <= x) & (x <= 2))
    assert_finite(x)
    a = np.empty((7, 11), dtype=np.float64)
    rng.unif(out=a, a=-3.0, b=-2.0)
    assert np.all((-3 <= a) & (a <= -2))
    assert_finite(a)

    rng.seed(123)
    x = rng.normal(100, mu=0.0, sigma=1.0)
    assert_finite(x)
    a = np.empty((7, 11), dtype=np.float64)
    rng.normal(out=a, mu=2.0, sigma=3.0)
    assert_finite(a)


def test_unif_normal_float32():
    rng = rp.Rng(pick_engine())
    rng.seed(123)
    x = rng.unif(100, dtype=np.float32)
    assert x.dtype == np.float32
    assert np.all((0 <= x) & (x <= 1))
    assert_finite(x)
    a = np.empty((7, 11), dtype=np.float32)
    rng.unif(out=a)
    assert np.all((0 <= a) & (a <= 1))
    assert_finite(a)

    rng.seed(123)
    x = rng.unif(100, a=-1.0, b=2.0, dtype=np.float32)
    assert x.dtype == np.float32
    assert np.all((-1 <= x) & (x <= 2))
    assert_finite(x)
    a = np.empty((7, 11), dtype=np.float32)
    rng.unif(out=a, a=-3.0, b=-2.0)
    assert np.all((-3 <= a) & (a <= -2))
    assert_finite(a)

    rng.seed(123)
    x = rng.normal(100, mu=0.0, sigma=1.0, dtype=np.float32)
    assert x.dtype == np.float32
    assert_finite(x)
    a = np.empty((7, 11), dtype=np.float32)
    rng.normal(out=a, mu=2.0, sigma=3.0)
    assert_finite(a)


def test_other_continuous_float64():
    rng = rp.Rng(pick_engine())
    rng.seed(123)
    x = rng.exp()
    assert np.isscalar(x)
    x = rng.lognormal(100, mu=0.0, sigma=1.0)
    assert np.all(x >= 0)
    assert_finite(x)
    x = rng.exp(100, scale=1.0)
    assert np.all(x >= 0)
    assert_finite(x)
    x = rng.gamma(100, shape=2.0, scale=1.0)
    assert np.all(x >= 0)
    assert_finite(x)
    x = rng.chi2(100, nu=5.0)
    assert np.all(x >= 0)
    assert_finite(x)
    x = rng.t(100, nu=5.0)
    assert_finite(x)
    x = rng.f(100, nu1=5.0, nu2=7.0)
    assert np.all(x >= 0)
    assert_finite(x)
    x = rng.gumbel(100, mu=0.0, beta=1.0)
    assert_finite(x)
    x = rng.pareto(100, xm=2.0, alpha=1.0)
    assert np.all(x >= 0)
    assert_finite(x)
    x = rng.weibull(100, shape=2.0, scale=1.0)
    assert np.all(x >= 0)
    assert_finite(x)
    x = rng.skew_normal(100, mu=0.0, sigma=1.0, alpha=2.0)
    assert_finite(x)
    x = rng.beta(100, a=2.0, b=3.0)
    assert np.all((0 <= x) & (x <= 1))
    assert_finite(x)


def test_other_continuous_float32():
    rng = rp.Rng(pick_engine())
    rng.seed(123)
    x = rng.lognormal(100, mu=0.0, sigma=1.0, dtype=np.float32)
    assert x.dtype == np.float32
    assert np.all(x >= 0)
    assert_finite(x)
    x = rng.exp(100, scale=1.0, dtype=np.float32)
    assert np.all(x >= 0)
    assert_finite(x)
    x = rng.gamma(100, shape=2.0, scale=1.0, dtype=np.float32)
    assert np.all(x >= 0)
    assert_finite(x)
    x = rng.chi2(100, nu=5.0, dtype=np.float32)
    assert np.all(x >= 0)
    assert_finite(x)
    x = rng.t(100, nu=5.0, dtype=np.float32)
    assert_finite(x)
    x = rng.f(100, nu1=5.0, nu2=7.0, dtype=np.float32)
    assert np.all(x >= 0)
    assert_finite(x)
    x = rng.gumbel(100, mu=0.0, beta=1.0, dtype=np.float32)
    assert_finite(x)
    x = rng.pareto(100, xm=2.0, alpha=1.0, dtype=np.float32)
    assert np.all(x >= 0)
    assert_finite(x)
    x = rng.weibull(100, shape=2.0, scale=1.0, dtype=np.float32)
    assert np.all(x >= 0)
    assert_finite(x)
    x = rng.skew_normal(100, mu=0.0, sigma=1.0, alpha=2.0, dtype=np.float32)
    assert_finite(x)
    x = rng.beta(100, a=2.0, b=3.0, dtype=np.float32)
    assert np.all((0 <= x) & (x <= 1))
    assert_finite(x)


def test_mvn():
    rng = rp.Rng(pick_engine())
    rng.seed(123)
    Sigma = np.array([[1.0, 0.2], [0.2, 2.0]], dtype=np.float64)
    x = rng.mvn(5, Sigma)
    assert x.shape == (5, 2)
    assert_finite(x)
    mu = np.array([1.0, 2.0], dtype=np.float64)
    y = rng.mvn(3, Sigma, mu=mu)
    assert y.shape == (3, 2)
    assert_finite(y)
    z = np.empty((4, 2), dtype=np.float64)
    rng.mvn(Sigma=Sigma, mu=mu, out=z)
    assert z.shape == (4, 2)
    assert_finite(z)
    with np.testing.assert_raises(ValueError):
        rng.mvn(2, np.ones((2, 3)))
    with np.testing.assert_raises(ValueError):
        rng.mvn(2, Sigma, mu=np.array([1.0]))
