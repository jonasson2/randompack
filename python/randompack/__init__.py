"""
Randompack: high-performance random number generation.

Quick Start
-----------
Create an RNG instance with `Rng()` and use its methods to generate
random variates:

    >>> import randompack
    >>> rng = randompack.Rng()               # Default engine
    >>> rng = randompack.Rng("pcg64")        # Specify engine
    >>> x = rng.normal(100, mu=1, sigma=2)   # 100 N(1,2) variates

Available Engines
-----------------
Supported engines include xoshiro256++, xoshiro256**, PCG64 DXSM,
sfc64, Philox-4x64, Squares64, and ChaCha20.

See `Rng` for the main interface and `engines()` for the complete list.
"""

from importlib.metadata import PackageNotFoundError, version as _version
try:
  __version__ = _version("randompack")
except PackageNotFoundError:
  __version__ = "0.1.0"

from ._core import Rng, engines
__all__ = ["Rng", "engines"]
