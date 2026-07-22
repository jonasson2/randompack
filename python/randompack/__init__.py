"""
Python interface to Randompack. See https://github.com/jonasson2/randompack

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
  __version__ = None

from functools import wraps

from ._core import Rng as _CoreRng, engines

_CYTHON_METHOD = type(_CoreRng.seed)
_RNG_METHODS = frozenset(
  name for name, value in _CoreRng.__dict__.items()
  if isinstance(value, _CYTHON_METHOD)
)


def _unbound_method(name, method):
  @wraps(method)
  def wrapper(*args, **kwargs):
    raise TypeError(
      f"Rng.{name} requires an instance; use randompack.Rng().{name}(...)"
    )
  return wrapper


class _RngType(type):
  def __getattribute__(cls, name):
    value = super().__getattribute__(name)
    if name in _RNG_METHODS:
      return _unbound_method(name, value)
    return value

  def __instancecheck__(cls, value):
    return isinstance(value, _CoreRng)


class Rng(_CoreRng, metaclass=_RngType):
  __doc__ = _CoreRng.__doc__
  __module__ = __name__


__all__ = ["Rng", "engines"]
