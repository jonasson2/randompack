import numpy as np
cimport numpy as np

from libc.stddef cimport size_t
from libc.limits cimport INT_MIN, INT_MAX, LLONG_MIN, LLONG_MAX
from libc.stdint cimport uint8_t, uint32_t, uint64_t

from cpython.bytearray cimport PyByteArray_AsString
from cpython.bytes cimport PyBytes_AS_STRING, PyBytes_GET_SIZE

U32_MAX = (1 << 32) - 1
U64_MAX = (1 << 64) - 1
U128_MAX = (1 << 128) - 1

from ._core cimport (
    randompack_rng, randompack_create,
    randompack_duplicate, randompack_free,
    randompack_last_error, randompack_seed,
    randompack_randomize, randompack_engines,
    randompack_int, randompack_long_long,
    randompack_uint32, randompack_uint64,
    randompack_perm, randompack_sample,
    randompack_raw,
    randompack_unif,
    randompack_normal, randompack_lognormal,
    randompack_exp, randompack_gamma,
    randompack_beta, randompack_chi2,
    randompack_t, randompack_f,
    randompack_gumbel, randompack_pareto,
    randompack_weibull, randompack_skew_normal,
    randompack_mvn,
    randompack_uniff, randompack_normalf,
    randompack_lognormalf, randompack_expf,
    randompack_gammaf, randompack_betaf,
    randompack_chi2f, randompack_tf,
    randompack_ff, randompack_gumbelf,
    randompack_paretof, randompack_weibullf,
    randompack_skew_normalf, randompack_serialize,
    randompack_deserialize, randompack_philox_set_ctr,
    randompack_philox_set_key, randompack_sfc64_set_abc,
    randompack_squares_set_ctr, randompack_squares_set_key,
    randompack_pcg64_set_inc,
    randompack_set_state, )

np.import_array()

cdef inline void _raise_last_error(randompack_rng *rng):
    cdef char *msg = randompack_last_error(rng)
    if msg != NULL and msg[0] != 0:
        raise RuntimeError((<bytes>msg).decode('utf-8', 'replace'))
    raise RuntimeError("randompack error")

cdef inline void _out_check(randompack_rng *rng, object out, object dtype, object size):
    if rng == NULL:
        raise RuntimeError("RNG pointer is NULL")
    if out is not None and dtype is not None:
        raise ValueError("cannot pass both out and dtype")
    if out is not None and size is not None:
        raise ValueError("cannot pass both out and size")
    if out is None:
        return
    if not isinstance(out, np.ndarray):
        raise TypeError("out must be a numpy array")
    if not out.flags["C_CONTIGUOUS"]:
        raise ValueError("out must be contiguous")
    if not out.flags["WRITEABLE"]:
        raise ValueError("out must be writable")

cdef inline np.ndarray _prep_out_float(randompack_rng *rng, object out, object dtype,
                                       object size, void **ptr, size_t *n_elem,
                                       bint *is_f64):
    cdef np.ndarray arr
    cdef object dt
    _out_check(rng, out, dtype, size)
    if out is None:
        dt = np.dtype(np.float64) if dtype is None else np.dtype(dtype)
        if dt != np.dtype(np.float32) and dt != np.dtype(np.float64):
            raise TypeError("dtype must be float32 or float64")
        if size is None:
            arr = np.empty(1, dtype=dt)
        else:
            arr = np.empty(size, dtype=dt)
    else:
        arr = out
        if arr.dtype != np.dtype(np.float32) and arr.dtype != np.dtype(np.float64):
            raise TypeError("out must be float32 or float64")
    ptr[0] = <void *>np.PyArray_DATA(arr)
    n_elem[0] = <size_t>np.PyArray_SIZE(arr)
    is_f64[0] = arr.dtype == np.dtype(np.float64)
    return arr

cdef inline np.ndarray _prep_out_int(randompack_rng *rng, object out, object dtype,
                                     object size, void **ptr, size_t *n_elem,
                                     bint *is_i64):
    cdef np.ndarray arr
    cdef object dt
    _out_check(rng, out, dtype, size)
    if out is None:
        dt = np.dtype(np.int64) if dtype is None else np.dtype(dtype)
        if dt != np.dtype(np.int32) and dt != np.dtype(np.int64):
            raise TypeError("dtype must be int32 or int64")
        if size is None:
            arr = np.empty(1, dtype=dt)
        else:
            arr = np.empty(size, dtype=dt)
    else:
        arr = out
        if arr.dtype != np.dtype(np.int32) and arr.dtype != np.dtype(np.int64):
            raise TypeError("out must be int32 or int64")
    ptr[0] = <void *>np.PyArray_DATA(arr)
    n_elem[0] = <size_t>np.PyArray_SIZE(arr)
    is_i64[0] = arr.dtype == np.dtype(np.int64)
    return arr

cdef inline object _return_scalar(np.ndarray arr, object out, object size):
    if out is None and size is None:
        return arr[0]
    return arr

cdef class Rng:
    """
    Random number generator.

    Parameters
    ----------
    engine : str, optional
        Name of the random number generator engine to use. If omitted,
        a default engine is selected. Available engines can be listed
        using `engines()`.
    bitexact : bool, optional
        If True, use bitexact log/exp for distributions that rely on them.

    Notes
    -----
    A newly created generator is randomized (initialized) using system entropy
    unless explicitly seeded with the `seed` method.

    Examples
    --------
    >>> import randompack
    >>> rng = randompack.Rng()
    >>> rng.unif(3)

    >>> rng2 = randompack.Rng(engine="philox")
    >>> rng2.seed(123)
    >>> rng2.normal(3)
    >>> rng3 = randompack.Rng(engine="pcg64", bitexact=True)  # make samples bit-identical across platforms (x==y true)
    """

    cdef randompack_rng *ptr
    
    def __cinit__(self, engine=None, bitexact=False):
        self.ptr = NULL
        if engine is None:
            self.ptr = randompack_create(NULL)
        else:
            if not isinstance(engine, str):
                raise TypeError("engine must be a string")
            self.ptr = randompack_create(engine.encode())
        if self.ptr == NULL:
            raise MemoryError("randompack_create failed")
        msg = randompack_last_error(self.ptr)
        if msg != NULL and msg[0] != 0:
            randompack_free(self.ptr)
            self.ptr = NULL
            raise ValueError((<bytes>msg).decode())
        if bitexact:
            if not randompack_bitexact(self.ptr, True):
                _raise_last_error(self.ptr)

    def __dealloc__(self):
        if self.ptr != NULL:
            randompack_free(self.ptr)
            self.ptr = NULL

    def seed(self, seed, spawn_key=None):
        """
        Seed the random number generator deterministically.

        Parameters
        ----------
        seed : int
            Integer seed in the range [-2^31, 2^31-1].
        spawn_key : sequence of int, optional
            Optional sequence of integers (e.g., list or tuple) used together with `seed`
            to derive an independent deterministic substream. Each entry must lie in [0,
            2^32-1].
        Examples
        --------
        >>> import numpy as np, randompack, threading
        >>> rng = randompack.Rng()
        >>> rng.seed(123)
        >>> rng.unif(2)
        array([0.31641448, 0.0484203 ])

        >>> results = [None, None]
        >>> def worker(i):
        ...     rng = randompack.Rng()
        ...     rng.seed(123, spawn_key=[i])
        ...     results[i] = rng.normal(1000)
        >>> t0 = threading.Thread(target=worker, args=(0,))
        >>> t1 = threading.Thread(target=worker, args=(1,))
        >>> t0.start()
        >>> t1.start()
        >>> t0.join()
        >>> t1.join()
        >>> X = np.array(results)
        >>> X.shape
        (2, 1000)

        Returns
        -------
        None

        See Also
        --------
        randomize
        set_state
        """
        cdef np.ndarray[np.uint32_t, ndim=1, mode="c"] arr
        if self.ptr == NULL:
            raise RuntimeError("RNG pointer is NULL")
        s = int(seed)
        if s < INT_MIN or s > INT_MAX:
            raise ValueError("seed out of range for int32")
        if spawn_key is None:
            if not randompack_seed(s, <uint32_t *>NULL, 0, self.ptr):
                _raise_last_error(self.ptr)
            return
        vals = [int(v) for v in spawn_key]
        for v in vals:
            if v < 0 or v > U32_MAX:
                raise ValueError("spawn_key entries must be in [0, 2^32-1]")
        arr = np.asarray(vals, dtype=np.uint32)
        if not randompack_seed(s, <uint32_t *>arr.data, arr.size, self.ptr):
            _raise_last_error(self.ptr)

    def randomize(self):
        """
        Randomize the random number generator using system entropy.

        Returns
        -------
        None

        Examples
        --------
        >>> import randompack
        >>> rng = randompack.Rng()  # randomized by default
        >>> rng.seed(123)           # deterministic
        >>> rng.normal(3)
        >>> rng.randomize()         # discard deterministic state
        >>> rng.normal(3)        

        See Also
        --------
        seed
        set_state
        """

        if self.ptr == NULL:
            raise RuntimeError("RNG pointer is NULL")
        if not randompack_randomize(self.ptr):
            _raise_last_error(self.ptr)

    def full_mantissa(self, enable=True):
        """
        Toggle full mantissa generation for Float64 and Float32 draws.

        When enabled, Float64 draws use 53 bits of precision and Float32 draws use 24
        bits; otherwise 52 and 23 bits are used. The factory default is disabled. Enabling
        full mantissas slightly slows down the generator.

        Parameters
        ----------
        enable : bool
            True to enable full mantissas, False to disable.

        Returns
        -------
        None

        Examples
        --------
        >>> import randompack
        >>> rng = randompack.Rng()
        >>> rng.full_mantissa()       # turn full mantissa on
        >>> rng.full_mantissa(False)  # turn it off again
        """

        cdef bint ok
        if self.ptr == NULL:
            raise RuntimeError("RNG pointer is NULL")
        ok = randompack_full_mantissa(self.ptr, bool(enable))
        if not ok:
            _raise_last_error(self.ptr)

    def jump(self, p):
        """
        Jump an xor-family engine ahead by \\eqn{2^p} steps. The `x128+` and
        `xoro128++` engines support `p = 32, 64, 96`, while `x256++`, `x256**`,
        and `x256++simd` also support `p = 128` and `p = 192`.

        Parameters
        ----------
        p : int
            Jump exponent.

        Returns
        -------
        None
        """

        cdef bint ok
        if self.ptr == NULL:
            raise RuntimeError("RNG pointer is NULL")
        p = int(p)
        ok = randompack_jump(p, self.ptr)
        if not ok:
            _raise_last_error(self.ptr)

    def duplicate(self):
        """
        Duplicate the random number generator, preserving its state.

        Returns
        -------
        Rng
            A new random number generator object with identical state.

        Examples
        --------
        >>> import randompack
        >>> rng1 = randompack.Rng()
        >>> rng2 = rng1.duplicate()
        >>> x1 = rng1.normal(3)
        >>> x2 = rng2.normal(3)
        >>> (x1 == x2).all()
        True
        """

        cdef randompack_rng *dup
        cdef Rng obj
        if self.ptr == NULL:
            raise RuntimeError("RNG pointer is NULL")
        dup = randompack_duplicate(self.ptr)
        if dup == NULL:
            _raise_last_error(self.ptr)
        obj = Rng.__new__(Rng)
        obj.ptr = dup
        return obj

    def serialize(self):
        """
        Serialize the current random number generator state.

        Returns
        -------
        bytes
            Serialized state suitable for later restoration with `deserialize`.
        """

        cdef int n = 0
        cdef bytearray buf
        cdef uint8_t *ptr
        if self.ptr == NULL:
            raise RuntimeError("RNG pointer is NULL")
        if not randompack_serialize(<uint8_t *>NULL, &n, self.ptr):
            _raise_last_error(self.ptr)
        if n <= 0:
            _raise_last_error(self.ptr)
        buf = bytearray(n)
        ptr = <uint8_t *>PyByteArray_AsString(buf)
        if not randompack_serialize(ptr, &n, self.ptr):
            _raise_last_error(self.ptr)
        return bytes(buf[:n])

    def deserialize(self, state):
        """
        Restore the random number generator state from serialized bytes.

        Parameters
        ----------
        state : bytes
            Serialized state previously returned by `serialize()`.

        Returns
        -------
        None
        """

        cdef const uint8_t *ptr
        cdef Py_ssize_t n
        if self.ptr == NULL:
            raise RuntimeError("RNG pointer is NULL")
        if not isinstance(state, (bytes,)):
            raise TypeError("state must be bytes")
        n = PyBytes_GET_SIZE(state)
        ptr = <const uint8_t *>PyBytes_AS_STRING(state)
        if not randompack_deserialize(ptr, <int>n, self.ptr):
            _raise_last_error(self.ptr)

    def squares_set_ctr(self, ctr):
        """
        Set the counter of a random number generator created with
        engine="squares".

        Parameters
        ----------
        ctr : int
            64-bit counter value in [0, 2^64-1].
        Returns
        -------
        None
        """
        cdef uint64_t ctr_v
        cdef object ctr_i
        if self.ptr == NULL:
            raise RuntimeError("RNG pointer is NULL")
        ctr_i = int(ctr)
        if ctr_i < 0 or ctr_i > U64_MAX:
            raise ValueError("ctr must be in [0, 2^64-1]")
        ctr_v = <uint64_t>ctr_i
        if not randompack_squares_set_ctr(ctr_v, self.ptr):
            _raise_last_error(self.ptr)

    def squares_set_key(self, key):
        """
        Set the key of a random number generator created with engine="squares".

        Parameters
        ----------
        key : int
            64-bit key value in [0, 2^64-1].

        Returns
        -------
        None

        Examples
        --------
        >>> import randompack
        """
        cdef uint64_t key_v
        cdef object key_i
        if self.ptr == NULL:
            raise RuntimeError("RNG pointer is NULL")
        key_i = int(key)
        if key_i < 0 or key_i > U64_MAX:
            raise ValueError("key must be in [0, 2^64-1]")
        key_v = <uint64_t>key_i
        if not randompack_squares_set_key(key_v, self.ptr):
            _raise_last_error(self.ptr)

    def philox_set_ctr(self, ctr):
        """
        Set the counter of an RNG instance using the "philox" engine.
        """
        cdef uint64_t c[4]
        cdef int i
        cdef object val
        if self.ptr == NULL:
            raise RuntimeError("RNG pointer is NULL")
        vals_ctr = [int(v) for v in ctr]
        if len(vals_ctr) != 4:
            raise ValueError("ctr must have length 4")
        for i in range(4):
            val = vals_ctr[i]
            if val < 0 or val > U64_MAX:
                raise ValueError("ctr entries must be in [0, 2^64-1]")
            c[i] = <uint64_t>val
        if not randompack_philox_set_ctr(c, self.ptr):
            _raise_last_error(self.ptr)

    def philox_set_key(self, key):
        """
        Set the key of an RNG instance using the "philox" engine.
        """
        cdef uint64_t k[2]
        cdef int i
        cdef object val
        if self.ptr == NULL:
            raise RuntimeError("RNG pointer is NULL")
        vals_key = [int(v) for v in key]
        if len(vals_key) != 2:
            raise ValueError("key must have length 2")
        for i in range(2):
            val = vals_key[i]
            if val < 0 or val > U64_MAX:
                raise ValueError("key entries must be in [0, 2^64-1]")
            k[i] = <uint64_t>val
        if not randompack_philox_set_key(k, self.ptr):
            _raise_last_error(self.ptr)

    def pcg64_set_inc(self, inc):
        """
        Set the 128-bit increment of a random number generator created with
        engine="pcg64".

        Parameters
        ----------
        inc : sequence of int
            Two 64-bit words `[low, high]` in [0, 2^64-1]. The low word must
            be odd.

        Returns
        -------
        None

        See Also
        --------
        squares_set_ctr
        philox_set_ctr
        set_state
        """
        cdef uint64_t c_inc[2]
        cdef object vals
        cdef object val
        cdef int i
        if self.ptr == NULL:
            raise RuntimeError("RNG pointer is NULL")
        vals = list(inc)
        if len(vals) != 2:
            raise ValueError("inc must have length 2")
        for i in range(2):
            val = int(vals[i])
            if val < 0 or val > U64_MAX:
                raise ValueError("inc entries must be in [0, 2^64-1]")
            c_inc[i] = <uint64_t>val
        if not randompack_pcg64_set_inc(c_inc, self.ptr):
            _raise_last_error(self.ptr)

    def sfc64_set_abc(self, abc):
        """
        Set the sfc64 a, b, c state words directly.

        Parameters
        ----------
        abc : sequence of int
            Three 64-bit state words `[a, b, c]` in [0, 2^64-1].

        Returns
        -------
        None

        See Also
        --------
        set_state
        pcg64_set_inc
        """
        cdef uint64_t c_abc[3]
        cdef object vals
        cdef object val
        cdef int i
        if self.ptr == NULL:
            raise RuntimeError("RNG pointer is NULL")
        vals = list(abc)
        if len(vals) != 3:
            raise ValueError("abc must have length 3")
        for i in range(3):
            val = int(vals[i])
            if val < 0 or val > U64_MAX:
                raise ValueError("abc entries must be in [0, 2^64-1]")
            c_abc[i] = <uint64_t>val
        if not randompack_sfc64_set_abc(c_abc, self.ptr):
            _raise_last_error(self.ptr)

    def set_state(self, state):
        """
        Set the internal engine state directly.

        Parameters
        ----------
        state : sequence of int
            Sequence of integers with entries in [0, 2^64-1]. Length is engine-specific.

        Returns
        -------
        None

        See Also
        --------
        seed
        randomize
        squares_set_ctr
        squares_set_key
        philox_set_ctr
        philox_set_key
        pcg64_set_inc
        """
        cdef list vals
        cdef np.ndarray arr
        cdef int nstate
        cdef int i
        cdef object v
        if self.ptr == NULL:
            raise RuntimeError("RNG pointer is NULL")
        vals = [int(v) for v in state]
        for i in range(len(vals)):
            v = vals[i]
            if v < 0 or v > U64_MAX:
                raise ValueError("state entries must be in [0, 2^64-1]")
        arr = np.asarray(vals, dtype=np.uint64)
        if arr.ndim != 1:
            raise ValueError("state must be a 1D array")
        if arr.size > INT_MAX:
            raise ValueError("state too long")
        nstate = <int>arr.size
        if not randompack_set_state(<uint64_t *>np.PyArray_DATA(arr), nstate,
                                    self.ptr):
            _raise_last_error(self.ptr)

    def unif(self, size=None, *, a=0.0, b=1.0, out=None, dtype=None):
        """
        Draw samples from a uniform distribution.

        Samples are drawn from the uniform distribution on the interval [a,b]. The
        default interval is [0,1]. The returned data type is float64 unless float32
        is requested.

        Parameters
        ----------
        size : int or tuple of int, optional
            Output shape. Cannot be given together with `out`. If `size` is None, a
            a scalar is returned.
        a : float, default 0.0
            Lower bound of the sampling interval.
        b : float, default 1.0
            Upper bound of the sampling interval. Must satisfy b > a.
        out : numpy.ndarray, optional
            Output array. Must be contiguous, writeable, and of dtype float32 or float64.
        dtype : numpy.dtype or str, optional
            float32 or float64. Cannot be given together with `out`.

        Returns
        -------
        numpy.ndarray
            Samples drawn from the uniform distribution.

        Examples
        --------
        >>> import numpy as np, randompack
        >>> rng = randompack.Rng()
        >>> rng.seed(42)
        >>> rng.unif(2)
        array([0.59733558, 0.53618867])
        >>> x = np.zeros(2)
        >>> rng.seed(42)
        >>> rng.unif(out=x)
        >>> x
        array([0.59733558, 0.53618867])
        >>> y = rng.unif(2, a=2.0, b=5.0)
        >>> z = rng.unif(size=(3, 3), dtype=np.float32)

        See Also
        --------
        normal
        """
        cdef double a_d = a
        cdef double b_d = b
        cdef size_t n_elem
        cdef void *ptr
        cdef bint ok, is_f64
        arr = _prep_out_float(self.ptr, out, dtype, size, &ptr, &n_elem, &is_f64)
        with nogil:
            if is_f64:
                ok = randompack_unif(<double *>ptr, n_elem, a_d, b_d, self.ptr)
            else:
                ok = randompack_uniff(<float *>ptr, n_elem, <float>a_d,
                                      <float>b_d, self.ptr)
        if not ok:
            _raise_last_error(self.ptr)
        return _return_scalar(arr, out, size)

    def normal(self, size=None, *, mu=0.0, sigma=1.0, out=None, dtype=None):
        """Draw samples from a normal distribution.

        Samples are drawn from N(`mu`,`sigma`). The default parameters are `mu=0.0` and
        `sigma=1.0`, so if neither is provided then standard normal samples are obtained.
        The returned data type is float64 unless float32 is requested.

        Parameters
        ----------
        size : int or tuple of int, optional
            Output shape. Cannot be given together with `out`. If `size` is None, a
            a scalar is returned.
        mu : float, default 0.0
            Mean of the distribution.
        sigma : float, default 1.0
            Standard deviation. Must satisfy sigma > 0.
        out : numpy.ndarray, optional
            Output array. Must be contiguous, writeable, and of dtype float32 or float64.
        dtype : numpy.dtype or str, optional
            float32 or float64. Cannot be given together with `out`.

        Returns
        -------
        numpy.ndarray
            Samples drawn from the normal distribution.

        Notes
        -----
        The normal distribution is generated using the Ziggurat rejection-sampling method
        of Marsaglia and Tsang, with constants matching the NumPy implementation at the
        time of writing (2025).

        Examples
        --------
        >>> import numpy as np, randompack
        >>> rng = randompack.Rng()
        >>> rng.seed(42)
        >>> rng.normal(2, mu=3, sigma=2)
        array([6.64112356, 2.36966204])
        >>> x = np.zeros(2)
        >>> rng.seed(42)
        >>> rng.normal(mu=3.0, sigma=2.0, out=x)
        >>> x
        array([6.64112356, 2.36966204])
        >>> y = rng.normal(size=(3, 3), dtype=np.float32)

        See Also
        --------
        unif
        exp
        lognormal
        gamma

        """
        cdef double mu_d = mu
        cdef double sigma_d = sigma
        cdef size_t n_elem
        cdef void *ptr
        cdef bint ok, is_f64
        arr = _prep_out_float(self.ptr, out, dtype, size, &ptr, &n_elem, &is_f64)
        with nogil:
            if is_f64:
                ok = randompack_normal(<double *>ptr, n_elem, mu_d, sigma_d,
                                       self.ptr)
            else:
                ok = randompack_normalf(<float *>ptr, n_elem, <float>mu_d,
                                        <float>sigma_d, self.ptr)
        if not ok:
            _raise_last_error(self.ptr)
        return _return_scalar(arr, out, size)

    def exp(self, size=None, *, scale=1.0, out=None, dtype=None):
        """Draw samples from an exponential distribution.

        Samples are drawn with scale parameter `scale` (mean = scale). The default scale
        is 1.0, giving the standard exponential distribution. The returned data type is
        float64 unless float32 is requested.

        Parameters
        ----------
        size : int or tuple of int, optional
            Output shape. Cannot be given together with `out`. If `size` is None, a
            a scalar is returned.
        scale : float, default 1.0
            Scale parameter. Must satisfy scale > 0.
        out : numpy.ndarray, optional
            Output array. Must be contiguous, writeable, and of dtype float32 or float64.
        dtype : numpy.dtype or str, optional
            float32 or float64. Cannot be given together with `out`.

        Returns
        -------
        numpy.ndarray
            Samples drawn from the exponential distribution.

        Notes
        -----
        The exponential distribution is generated using the Ziggurat rejection-sampling
        method of Marsaglia and Tsang, with constants matching the NumPy implementation at
        the time of writing (2025).

        Examples
        --------
        >>> import numpy as np, randompack
        >>> rng = randompack.Rng()
        >>> rng.exp(5)
        >>> x = rng.exp(5, scale=2.0)
        >>> y = rng.exp(size=(3,3), dtype=np.float32)

        See Also
        --------
        unif
        normal
        lognormal
        gamma

        """
        cdef double scale_d = scale
        cdef size_t n_elem
        cdef void *ptr
        cdef bint ok, is_f64
        arr = _prep_out_float(self.ptr, out, dtype, size, &ptr, &n_elem, &is_f64)
        with nogil:
            if is_f64:
                ok = randompack_exp(<double *>ptr, n_elem, scale_d, self.ptr)
            else:
                ok = randompack_expf(<float *>ptr, n_elem, <float>scale_d, self.ptr)
        if not ok:
            _raise_last_error(self.ptr)
        return _return_scalar(arr, out, size)

    def lognormal(self, size=None, *, mu=0.0, sigma=1.0, out=None, dtype=None):
        """
        Draw samples from a lognormal distribution.

        Samples are drawn from a lognormal distribution derived from an underlying
        normal distribution with mean `mu` and standard deviation `sigma`. The
        default parameters are `mu=0.0` and `sigma=1.0`. The returned data type is
        float64 unless float32 is requested.

        Parameters
        ----------
        size : int or tuple of int, optional
            Output shape. Cannot be given together with `out`. If `size` is None, a
            a scalar is returned.
        mu : float, default 0.0
            Mean of the underlying normal distribution.
        sigma : float, default 1.0
            Standard deviation of the underlying normal distribution. Must satisfy
            sigma > 0.
        out : numpy.ndarray, optional
            Output array. Must be contiguous, writeable, and of dtype float32 or float64.
        dtype : numpy.dtype or str, optional
            float32 or float64. Cannot be given together with `out`.

        Returns
        -------
        numpy.ndarray
            Samples drawn from the lognormal distribution.

        Examples
        --------
        >>> import numpy as np, randompack
        >>> rng = randompack.Rng()
        >>> rng.lognormal(5)
        >>> x = rng.lognormal(5, mu=1.0, sigma=0.5)
        >>> y = rng.lognormal(size=(3,3), dtype=np.float32)

        See Also
        --------
        normal
        exp
        gamma
        """
        cdef double mu_d = mu
        cdef double sigma_d = sigma
        cdef size_t n_elem
        cdef void *ptr
        cdef bint ok, is_f64
        arr = _prep_out_float(self.ptr, out, dtype, size, &ptr, &n_elem, &is_f64)
        with nogil:
            if is_f64:
                ok = randompack_lognormal(<double *>ptr, n_elem, mu_d, sigma_d,
                                          self.ptr)
            else:
                ok = randompack_lognormalf(<float *>ptr, n_elem, <float>mu_d,
                                           <float>sigma_d, self.ptr)
        if not ok:
            _raise_last_error(self.ptr)
        return _return_scalar(arr, out, size)

    def gamma(self, size=None, *, shape, scale=1.0, out=None, dtype=None):
        """
        Draw samples from a gamma distribution.

        Samples are drawn with shape parameter `shape` and scale parameter `scale`.
        The default scale is 1.0. The returned data type is float64 unless float32
        is requested.

        Parameters
        ----------
        size : int or tuple of int, optional
            Output shape. Cannot be given together with `out`. If `size` is None, a
            a scalar is returned.
        shape : float
            Shape parameter. Must satisfy shape > 0.
        scale : float, default 1.0
            Scale parameter. Must satisfy scale > 0.
        out : numpy.ndarray, optional
            Output array. Must be contiguous, writeable, and of dtype float32 or float64.
        dtype : numpy.dtype or str, optional
            float32 or float64. Cannot be given together with `out`.

        Returns
        -------
        numpy.ndarray
            Samples drawn from the gamma distribution.

        Notes
        -----
        Gamma variates are generated using the Marsaglia–Tsang method.

        Examples
        --------
        >>> import numpy as np, randompack
        >>> rng = randompack.Rng()
        >>> rng.gamma(5, shape=2.0)
        >>> x = rng.gamma(5, shape=2.0, scale=3.0)
        >>> y = rng.gamma(size=(3,3), shape=2.0, dtype=np.float32)

        See Also
        --------
        exp
        lognormal
        chi2
        """
        cdef double shape_d = shape
        cdef double scale_d = scale
        cdef size_t n_elem
        cdef void *ptr
        cdef bint ok, is_f64
        arr = _prep_out_float(self.ptr, out, dtype, size, &ptr, &n_elem, &is_f64)
        with nogil:
            if is_f64:
                ok = randompack_gamma(<double *>ptr, n_elem, shape_d, scale_d,
                                      self.ptr)
            else:
                ok = randompack_gammaf(<float *>ptr, n_elem, <float>shape_d,
                                       <float>scale_d, self.ptr)
        if not ok:
            _raise_last_error(self.ptr)
        return _return_scalar(arr, out, size)

    def beta(self, size=None, *, a, b, out=None, dtype=None):
        """
        Draw samples from a beta distribution.

        Samples are drawn from the beta distribution with parameters `a` and `b`. The
        returned data type is float64 unless float32 is requested.

        Parameters
        ----------
        size : int or tuple of int, optional
            Output shape. Cannot be given together with `out`. If `size` is None, a
            a scalar is returned.
        a : float
            First shape parameter. Must satisfy a > 0.
        b : float
            Second shape parameter. Must satisfy b > 0.
        out : numpy.ndarray, optional
            Output array. Must be contiguous, writeable, and of dtype float32 or float64.
        dtype : numpy.dtype or str, optional
            float32 or float64. Cannot be given together with `out`.

        Returns
        -------
        numpy.ndarray
            Samples drawn from the beta distribution.

        Notes
        -----
        Beta(a,b) variates are generated as Ga/(Ga+Gb) with independent Ga ~ Gamma(a,1)
        and Gb ~ Gamma(b,1).

        Examples
        --------
        >>> import numpy as np
        >>> import randompack
        >>> rng = randompack.Rng()
        >>> x = rng.beta(5, a=2.0, b=5.0)
        >>> y = rng.beta(size=(3,3), a=2, b=5, dtype=np.float32)

        See Also
        --------
        unif
        """
        cdef double a_d = a
        cdef double b_d = b
        cdef size_t n_elem
        cdef void *ptr
        cdef bint ok, is_f64
        arr = _prep_out_float(self.ptr, out, dtype, size, &ptr, &n_elem, &is_f64)
        with nogil:
            if is_f64:
                ok = randompack_beta(<double *>ptr, n_elem, a_d, b_d, self.ptr)
            else:
                ok = randompack_betaf(<float *>ptr, n_elem, <float>a_d,
                                      <float>b_d, self.ptr)
        if not ok:
            _raise_last_error(self.ptr)
        return _return_scalar(arr, out, size)

    def chi2(self, size=None, *, nu, out=None, dtype=None):
        """
        Draw samples from a chi-square distribution.

        Samples are drawn with `nu` degrees of freedom. The returned data type is
        float64 unless float32 is requested.

        Parameters
        ----------
        size : int or tuple of int, optional
            Output shape. Cannot be given together with `out`. If `size` is None, a
            a scalar is returned.
        nu : float
            Degrees of freedom. Must satisfy nu > 0.
        out : numpy.ndarray, optional
            Output array. Must be contiguous, writeable, and of dtype float32 or float64.
        dtype : numpy.dtype or str, optional
            float32 or float64. Cannot be given together with `out`.

        Returns
        -------
        numpy.ndarray
            Samples drawn from the chi-square distribution.

        Notes
        -----
        Chi-square variates are generated as Gamma(nu/2,2).

        Examples
        --------
        >>> import numpy as np, randompack
        >>> rng = randompack.Rng()
        >>> rng.chi2(5, nu=3)
        >>> x = rng.chi2(5, nu=3, dtype=np.float32)

        See Also
        --------
        gamma
        t
        f
        """
        cdef double nu_d = nu
        cdef size_t n_elem
        cdef void *ptr
        cdef bint ok, is_f64
        arr = _prep_out_float(self.ptr, out, dtype, size, &ptr, &n_elem, &is_f64)
        with nogil:
            if is_f64:
                ok = randompack_chi2(<double *>ptr, n_elem, nu_d, self.ptr)
            else:
                ok = randompack_chi2f(<float *>ptr, n_elem, <float>nu_d, self.ptr)
        if not ok:
            _raise_last_error(self.ptr)
        return _return_scalar(arr, out, size)

    def t(self, size=None, *, nu, out=None, dtype=None):
        """
        Draw samples from a Student's t distribution.

        Samples are drawn with `nu` degrees of freedom. The returned data type is
        float64 unless float32 is requested.

        Parameters
        ----------
        size : int or tuple of int, optional
            Output shape. Cannot be given together with `out`. If `size` is None, a
            a scalar is returned.
        nu : float
            Degrees of freedom. Must satisfy nu > 0.
        out : numpy.ndarray, optional
            Output array. Must be contiguous, writeable, and of dtype float32 or float64.
        dtype : numpy.dtype or str, optional
            float32 or float64. Cannot be given together with `out`.

        Returns
        -------
        numpy.ndarray
            Samples drawn from the t distribution.

        Notes
        -----
        Student’s t variates are generated as Z / sqrt(U/nu) with Z ~ N(0,1) and
        U ~ Gamma(nu/2,2) independent.

        Examples
        --------
        >>> import numpy as np, randompack
        >>> rng = randompack.Rng()
        >>> rng.t(5, nu=10)
        >>> x = rng.t(5, nu=10, dtype=np.float32)

        See Also
        --------
        normal
        chi2
        f
        """
        cdef double nu_d = nu
        cdef size_t n_elem
        cdef void *ptr
        cdef bint ok, is_f64
        arr = _prep_out_float(self.ptr, out, dtype, size, &ptr, &n_elem, &is_f64)
        with nogil:
            if is_f64:
                ok = randompack_t(<double *>ptr, n_elem, nu_d, self.ptr)
            else:
                ok = randompack_tf(<float *>ptr, n_elem, <float>nu_d, self.ptr)
        if not ok:
            _raise_last_error(self.ptr)
        return _return_scalar(arr, out, size)

    def f(self, size=None, *, nu1, nu2, out=None, dtype=None):
        """
        Draw samples from an F distribution.

        Samples are drawn with `nu1` and `nu2` degrees of freedom. The returned data
        type is float64 unless float32 is requested.

        Parameters
        ----------
        size : int or tuple of int, optional
            Output shape. Cannot be given together with `out`. If `size` is None, a
            a scalar is returned.
        nu1 : float
            Degrees of freedom for the numerator. Must satisfy nu1 > 0.
        nu2 : float
            Degrees of freedom for the denominator. Must satisfy nu2 > 0.
        out : numpy.ndarray, optional
            Output array. Must be contiguous, writeable, and of dtype float32 or float64.
        dtype : numpy.dtype or str, optional
            float32 or float64. Cannot be given together with `out`.

        Returns
        -------
        numpy.ndarray
            Samples drawn from the F distribution.

        Notes
        -----
        F(nu1,nu2) variates are generated as (X1nu2)/(X2nu1) with X1 ~ Gamma(nu1/2,1)
        and X2 ~ Gamma(nu2/2,1) independent.

        Examples
        --------
        >>> import numpy as np, randompack
        >>> rng = randompack.Rng()
        >>> rng.f(5, nu1=5, nu2=7)
        >>> x = rng.f(5, nu1=5, nu2=7, dtype=np.float32)

        See Also
        --------
        chi2
        t
        """
        cdef double nu1_d = nu1
        cdef double nu2_d = nu2
        cdef size_t n_elem
        cdef void *ptr
        cdef bint ok, is_f64
        arr = _prep_out_float(self.ptr, out, dtype, size, &ptr, &n_elem, &is_f64)
        with nogil:
            if is_f64:
                ok = randompack_f(<double *>ptr, n_elem, nu1_d, nu2_d, self.ptr)
            else:
                ok = randompack_ff(<float *>ptr, n_elem, <float>nu1_d,
                                   <float>nu2_d, self.ptr)
        if not ok:
            _raise_last_error(self.ptr)
        return _return_scalar(arr, out, size)

    def gumbel(self, size=None, *, mu=0.0, beta=1.0, out=None, dtype=None):
        """
        Draw samples from a Gumbel distribution.

        Samples are drawn with location `mu` and scale `beta`. The default parameters
        are `mu=0.0` and `beta=1.0`. The returned data type is float64 unless float32
        is requested.

        Parameters
        ----------
        size : int or tuple of int, optional
            Output shape. Cannot be given together with `out`. If `size` is None, a
            a scalar is returned.
        mu : float, default 0.0
            Location parameter.
        beta : float, default 1.0
            Scale parameter. Must satisfy beta > 0.
        out : numpy.ndarray, optional
            Output array. Must be contiguous, writeable, and of dtype float32 or float64.
        dtype : numpy.dtype or str, optional
            float32 or float64. Cannot be given together with `out`.

        Returns
        -------
        numpy.ndarray
            Samples drawn from the Gumbel distribution.

        Notes
        -----
        Gumbel variates are generated as mu - beta * log(-log U) with U ~ Uniform(0,1).

        Examples
        --------
        >>> import numpy as np, randompack
        >>> rng = randompack.Rng()
        >>> rng.gumbel(5)
        >>> x = rng.gumbel(5, mu=0.0, beta=2.0)
        >>> y = rng.gumbel(size=(3,3), dtype=np.float32)

        See Also
        --------
        normal
        pareto
        """
        cdef double mu_d = mu
        cdef double beta_d = beta
        cdef size_t n_elem
        cdef void *ptr
        cdef bint ok, is_f64
        arr = _prep_out_float(self.ptr, out, dtype, size, &ptr, &n_elem, &is_f64)
        with nogil:
            if is_f64:
                ok = randompack_gumbel(<double *>ptr, n_elem, mu_d, beta_d,
                                       self.ptr)
            else:
                ok = randompack_gumbelf(<float *>ptr, n_elem, <float>mu_d,
                                        <float>beta_d, self.ptr)
        if not ok:
            _raise_last_error(self.ptr)
        return _return_scalar(arr, out, size)

    def pareto(self, size=None, *, xm, alpha, out=None, dtype=None):
        """
        Draw samples from a Pareto distribution.

        Samples are drawn with minimum value `xm` and shape parameter `alpha`. The
        returned data type is float64 unless float32 is requested.

        Parameters
        ----------
        size : int or tuple of int, optional
            Output shape. Cannot be given together with `out`. If `size` is None, a
            a scalar is returned.
        xm : float
            Minimum value. Must satisfy xm > 0.
        alpha : float
            Shape parameter. Must satisfy alpha > 0.
        out : numpy.ndarray, optional
            Output array. Must be contiguous, writeable, and of dtype float32 or float64.
        dtype : numpy.dtype or str, optional
            float32 or float64. Cannot be given together with `out`.

        Returns
        -------
        numpy.ndarray
            Samples drawn from the Pareto distribution.

        Notes
        -----
        Pareto(xm,alpha) variates are generated as xm * exp(E/alpha) with E ~ Exp(1).

        Examples
        --------
        >>> import numpy as np, randompack
        >>> rng = randompack.Rng()
        >>> rng.pareto(5, xm=1.0, alpha=2.0)
        >>> y = rng.pareto(size=(3,3), xm=1, alpha=2, dtype=np.float32)

        See Also
        --------
        gumbel
        weibull
        """
        cdef double xm_d = xm
        cdef double alpha_d = alpha
        cdef size_t n_elem
        cdef void *ptr
        cdef bint ok, is_f64
        arr = _prep_out_float(self.ptr, out, dtype, size, &ptr, &n_elem, &is_f64)
        with nogil:
            if is_f64:
                ok = randompack_pareto(<double *>ptr, n_elem, xm_d, alpha_d,
                                       self.ptr)
            else:
                ok = randompack_paretof(<float *>ptr, n_elem, <float>xm_d,
                                        <float>alpha_d, self.ptr)
        if not ok:
            _raise_last_error(self.ptr)
        return _return_scalar(arr, out, size)

    def weibull(self, size=None, *, shape, scale=1.0, out=None, dtype=None):
        """
        Draw samples from a Weibull distribution.

        Samples are drawn with shape parameter `shape` and scale parameter `scale`.
        The default scale is 1.0. The returned data type is float64 unless float32
        is requested.

        Parameters
        ----------
        size : int or tuple of int, optional
            Output shape. Cannot be given together with `out`. If `size` is None, a
            a scalar is returned.
        shape : float
            Shape parameter. Must satisfy shape > 0.
        scale : float, default 1.0
            Scale parameter. Must satisfy scale > 0.
        out : numpy.ndarray, optional
            Output array. Must be contiguous, writeable, and of dtype float32 or float64.
        dtype : numpy.dtype or str, optional
            float32 or float64. Cannot be given together with `out`.

        Returns
        -------
        numpy.ndarray
            Samples drawn from the Weibull distribution.

        Notes
        -----
        Weibull variates are generated as scale * E^(1/shape) with E ~ Exp(1).

        Examples
        --------
        >>> import numpy as np, randompack
        >>> rng = randompack.Rng()
        >>> rng.weibull(5, shape=2.0)
        >>> y = rng.weibull(size=(3,3), shape=2, dtype=np.float32)

        See Also
        --------
        pareto
        """
        cdef double shape_d = shape
        cdef double scale_d = scale
        cdef size_t n_elem
        cdef void *ptr
        cdef bint ok, is_f64
        arr = _prep_out_float(self.ptr, out, dtype, size, &ptr, &n_elem, &is_f64)
        with nogil:
            if is_f64:
                ok = randompack_weibull(<double *>ptr, n_elem, shape_d, scale_d,
                                        self.ptr)
            else:
                ok = randompack_weibullf(<float *>ptr, n_elem, <float>shape_d,
                                         <float>scale_d, self.ptr)
        if not ok:
            _raise_last_error(self.ptr)
        return _return_scalar(arr, out, size)

    def skew_normal(self, size=None, *, mu=0.0, sigma=1.0, alpha, out=None, dtype=None):
        """
        Draw samples from a skew-normal distribution.

        Samples are drawn with location `mu`, scale `sigma`, and shape `alpha`. The
        default parameters are `mu=0.0` and `sigma=1.0`. The returned data type is
        float64 unless float32 is requested.

        Parameters
        ----------
        size : int or tuple of int, optional
            Output shape. Cannot be given together with `out`. If `size` is None, a
            a scalar is returned.
        mu : float, default 0.0
            Location parameter.
        sigma : float, default 1.0
            Scale parameter. Must satisfy sigma > 0.
        alpha : float
            Shape parameter.
        out : numpy.ndarray, optional
            Output array. Must be contiguous, writeable, and of dtype float32 or float64.
        dtype : numpy.dtype or str, optional
            float32 or float64. Cannot be given together with `out`.

        Returns
        -------
        numpy.ndarray
            Samples drawn from the skew-normal distribution.

        Notes
        -----
        Skew-normal variates use the representation delta*|U| + sqrt(1-delta^2)*Z
        with U,Z ~ N(0,1) independent, where delta = alpha / sqrt(1 + alpha^2).

        Examples
        --------
        >>> import numpy as np, randompack
        >>> rng = randompack.Rng()
        >>> rng.skew_normal(5, mu=1, sigma=2, alpha=3)
        >>> y = rng.skew_normal(size=(3,3), alpha=2.0, dtype=np.float32)

        See Also
        --------
        normal
        """
        cdef double mu_d = mu
        cdef double sigma_d = sigma
        cdef double alpha_d = alpha
        cdef size_t n_elem
        cdef void *ptr
        cdef bint ok, is_f64
        arr = _prep_out_float(self.ptr, out, dtype, size, &ptr, &n_elem, &is_f64)
        with nogil:
            if is_f64:
                ok = randompack_skew_normal(<double *>ptr, n_elem, mu_d, sigma_d,
                                            alpha_d, self.ptr)
            else:
                ok = randompack_skew_normalf(<float *>ptr, n_elem, <float>mu_d,
                                             <float>sigma_d, <float>alpha_d,
                                             self.ptr)
        if not ok:
            _raise_last_error(self.ptr)
        return _return_scalar(arr, out, size)

    def mvn(self, size=None, Sigma=None, mu=None, out=None):
        """Draw samples from a multivariate normal distribution.

        `Sigma` must be a symmetric positive semidefinite d by d covariance matrix given
        as a 2D NumPy array. If `mu` is provided, it must have length d; otherwise a zero
        mean is used. If `out` is provided, it must be a contiguous, writeable float64
        2D array with d columns, and its number of rows determines the number of samples to
        generate; otherwise `size` samples are generated. If `out` is provided `size` must
        be None. The output is a 2D float64 NumPy array with the generated samples stored in
        its rows.

        Parameters
        ----------
        size : int, optional
            Number of draws. Cannot be given together with `out`. If `size` is None,
            a single-row array is returned.
        Sigma : array_like
            Covariance matrix. Must be square.
        mu : array_like, optional
            Mean vector of length `d`.
        out : numpy.ndarray, optional
            Output array. Must be contiguous, writeable, float64, and of shape `(n, d)`.

        Returns
        -------
        numpy.ndarray
            Samples drawn from the multivariate normal distribution.

        Notes
        -----
        Multivariate normal samples are generated using a Cholesky factorization of the
        covariance matrix. Positive semidefinite, rank-deficient covariance matrices are
        supported via pivoted Cholesky factorization.

        Examples
        --------
        >>> import numpy as np, randompack
        >>> rng = randompack.Rng()
        >>> Sigma = np.array([[1.0, 0.3], [0.3, 2.0]])
        >>> rng.mvn(5, Sigma)
        >>> x = rng.mvn(100, Sigma, mu=[1.0, 2.0])

        See Also
        --------
        normal

        """
        cdef np.ndarray arr
        cdef np.ndarray Sig_arr
        cdef np.ndarray mu_arr
        cdef int d, ldX
        cdef size_t n
        cdef double *mu_ptr
        cdef double *Sig_ptr
        cdef double *X_ptr
        cdef char transp = b'T'[0]
        cdef bint ok
        _out_check(self.ptr, out, None, size)
        if out is not None and size is not None:
            raise ValueError("size must be None when out is provided")
        if Sigma is None:
            raise TypeError("Sigma must be provided")
        Sig_arr = np.asarray(Sigma, dtype=np.float64)
        if Sig_arr.ndim != 2 or Sig_arr.shape[0] != Sig_arr.shape[1]:
            raise ValueError("Sigma must be a square 2D array")
        if not Sig_arr.flags["C_CONTIGUOUS"]:
            Sig_arr = np.ascontiguousarray(Sig_arr)
        d = <int>Sig_arr.shape[0]
        if mu is None:
            mu_ptr = <double *>NULL
        else:
            mu_arr = np.asarray(mu, dtype=np.float64)
            if mu_arr.ndim != 1 or mu_arr.shape[0] != d:
                raise ValueError("mu must have length d")
            if not mu_arr.flags["C_CONTIGUOUS"]:
                mu_arr = np.ascontiguousarray(mu_arr)
            mu_ptr = <double *>np.PyArray_DATA(mu_arr)
        if out is None:
            if size is None:
                n = 1
            else:
                if not isinstance(size, (int, np.integer)):
                    raise TypeError("size must be an integer")
                if size < 0:
                    raise ValueError("size must be nonnegative")
                n = <size_t>size
            arr = np.empty((<int>n, d), dtype=np.float64)
        else:
            _out_check(self.ptr, out, None, size)
            arr = out
            if arr.dtype != np.dtype(np.float64):
                raise TypeError("out must be float64")
            if arr.ndim != 2:
                raise ValueError("out must be a 2D array")
            n = <size_t>arr.shape[0]
            if arr.shape[1] != d:
                raise ValueError("out shape does not match Sigma")
        ldX = d
        Sig_ptr = <double *>np.PyArray_DATA(Sig_arr)
        X_ptr = <double *>np.PyArray_DATA(arr)
        with nogil:
            ok = randompack_mvn(&transp, mu_ptr, Sig_ptr, d, n, X_ptr, ldX,
                                <double *>NULL, self.ptr)
        if not ok:
            _raise_last_error(self.ptr)
        return arr

    def int(self, a=None, b=None, *, size=None, out=None, dtype=None):
        """
        Draw uniform integers.

        If `a` and `b` are provided, samples are drawn from the inclusive range
        `[a, b]`. If `a` and `b` are omitted, raw integers are returned over the full
        range of the chosen dtype (int32 or int64).

        Parameters
        ----------
        a : int, optional
            Lower bound (inclusive). Must be given together with `b`.
        b : int, optional
            Upper bound (inclusive). Must be given together with `a`.
        size : int or tuple of int, optional
            Output shape. Cannot be given together with `out`. If `size` is None, a
            a scalar is returned.
        out : numpy.ndarray, optional
            Output array. Must be contiguous, writeable, and of dtype int32 or int64.
        dtype : numpy.dtype or str, optional
            int32 or int64. Cannot be given together with `out`.

        Returns
        -------
        numpy.ndarray
            Integer samples drawn uniformly from the specified range.

        Notes
        -----
        Integer samples from a bounded range use Lemire’s method for efficient unbiased
        bounded integer generation.

        Examples
        --------
        >>> import numpy as np, randompack
        >>> rng = randompack.Rng()
        >>> rng.int(5, a=-2, b=3)
        array([ 1, -1,  3,  0,  2])
        >>> x = rng.int(size=(3,3), a=1, b=10, dtype=np.int32)

        See Also
        --------
        perm
        sample
        raw
        """
        
        cdef np.ndarray arr
        cdef size_t n_elem
        cdef void *ptr
        cdef bint ok, is_i64
        cdef int a32, b32
        cdef long long a64, b64
        cdef object a_i, b_i
        arr = _prep_out_int(self.ptr, out, dtype, size, &ptr, &n_elem, &is_i64)
        if (a is None) != (b is None):
            raise ValueError("a and b must be given together")
        if a is None:
            with nogil:
                if is_i64:
                    ok = randompack_uint64(<uint64_t *>ptr, n_elem, 0, self.ptr)
                else:
                    ok = randompack_uint32(<uint32_t *>ptr, n_elem, 0, self.ptr)
        else:
            a_i = int(a)
            b_i = int(b)
            if b_i < a_i:
                raise ValueError("require a <= b")
            if not is_i64:
                if a_i < INT_MIN or a_i > INT_MAX:
                    raise ValueError("a out of range for int32")
                if b_i < INT_MIN or b_i > INT_MAX:
                    raise ValueError("b out of range for int32")
                a32 = a_i
                b32 = b_i
            else:
                if a_i < LLONG_MIN or a_i > LLONG_MAX:
                    raise ValueError("a out of range for int64")
                if b_i < LLONG_MIN or b_i > LLONG_MAX:
                    raise ValueError("b out of range for int64")
                a64 = a_i
                b64 = b_i
            with nogil:
                if is_i64:
                    ok = randompack_long_long(<long long *>ptr, n_elem, a64, b64,
                                              self.ptr)
                else:
                    ok = randompack_int(<int *>ptr, n_elem, a32, b32, self.ptr)
        if not ok:
            _raise_last_error(self.ptr)
        return _return_scalar(arr, out, size)

    def perm(self, n, *, out=None):
        """
        Draw a random permutation of 1..n.

        Parameters
        ----------
        n : int
            Size of the permutation.
        out : numpy.ndarray, optional
            Output array. Must be contiguous, writeable, and of dtype int32.

        Returns
        -------
        numpy.ndarray
            A permutation of 1..n as int32 values.

        Notes
        -----
        Permutations are generated using the Fisher–Yates shuffle algorithm.

        Examples
        --------
        >>> import randompack, numpy as np
        >>> rng = randompack.Rng()
        >>> rng.seed(123)
        >>> rng.perm(5)
        array([1, 5, 2, 3, 4], dtype=int32)
        >>> x = np.empty(5, dtype=np.int32)
        >>> rng.seed(123)
        >>> rng.perm(5, out=x)
        >>> x
        array([1, 5, 2, 3, 4], dtype=int32)

        See Also
        --------
        int
        sample
        raw
        """

        cdef np.ndarray arr
        cdef int n_i
        cdef bint ok
        if self.ptr == NULL:
            raise RuntimeError("RNG pointer is NULL")
        n_i = int(n)
        if n_i < 0:
            raise ValueError("n must be nonnegative")
        if out is None:
            arr = np.empty(n_i, dtype=np.int32)
        else:
            _out_check(self.ptr, out, None, None)
            arr = out
            if arr.dtype != np.dtype(np.int32):
                raise TypeError("out must be int32")
            if arr.ndim != 1:
                raise ValueError("out must be 1D")
            if arr.shape[0] != n_i:
                raise ValueError("out has wrong length")
        with nogil:
            ok = randompack_perm(<int *>np.PyArray_DATA(arr), n_i, self.ptr)
        if not ok:
            _raise_last_error(self.ptr)
        arr += 1
        return arr

    def sample(self, n, k, *, out=None):
        """
        Sample without replacement from 1..n.

        Parameters
        ----------
        n : int
            Upper bound of the sampling range (inclusive).
        k : int
            Number of samples to draw.
        out : numpy.ndarray, optional
            Output array. Must be contiguous, writeable, and of dtype int32.

        Returns
        -------
        numpy.ndarray
            Length-`k` sample from 1..n as int32 values.

        Notes
        -----
        Floyd’s algorithm is used for smaller samples (k <= n/2) and reservoir sampling
        otherwise.

        Examples
        --------
        >>> import randompack
        >>> rng = randompack.Rng()
        >>> rng.sample(10, 3)

        See Also
        --------
        int
        perm
        raw
        """
        cdef np.ndarray arr
        cdef int n_i, k_i
        cdef bint ok
        if self.ptr == NULL:
            raise RuntimeError("RNG pointer is NULL")
        n_i = int(n)
        k_i = int(k)
        if n_i < 0 or k_i < 0:
            raise ValueError("n and k must be nonnegative")
        if k_i > n_i:
            raise ValueError("require k <= n")
        if out is None:
            arr = np.empty(k_i, dtype=np.int32)
        else:
            _out_check(self.ptr, out, None, None)
            arr = out
            if arr.dtype != np.dtype(np.int32):
                raise TypeError("out must be int32")
            if arr.ndim != 1:
                raise ValueError("out must be 1D")
            if arr.shape[0] != k_i:
                raise ValueError("out has wrong length")
        with nogil:
            ok = randompack_sample(<int *>np.PyArray_DATA(arr), n_i, k_i,
                                   self.ptr)
        if not ok:
            _raise_last_error(self.ptr)
        arr += 1
        return arr

    def raw(self, n):
        """
        Generate random bytes.

        Parameters
        ----------
        n : int
            Number of bytes to generate.

        Returns
        -------
        bytes
            Random bytes.

        Examples
        --------
        >>> import randompack
        >>> rng = randompack.Rng()
        >>> rng.raw(4)

        See Also
        --------
        int
        perm
        sample
        """
        cdef bytearray buf
        cdef uint8_t *ptr
        cdef size_t nbytes
        cdef bint ok
        if self.ptr == NULL:
            raise RuntimeError("RNG pointer is NULL")
        n_i = int(n)
        if n_i < 0:
            raise ValueError("n must be nonnegative")
        nbytes = <size_t>n_i
        buf = bytearray(nbytes)
        ptr = <uint8_t *>PyByteArray_AsString(buf)
        with nogil:
            ok = randompack_raw(ptr, nbytes, self.ptr)
        if not ok:
            _raise_last_error(self.ptr)
        return bytes(buf)

class _EngineInfo(dict):
    def __repr__(self):
        if not self:
            return "{}"
        width = max(len(name) for name in self)
        lines = []
        for name, desc in self.items():   # preserves insertion order
            lines.append(f"{name:<{width}}  {desc}")
        return "\n".join(lines)

def engines():
    """
    List available random number generator engines.

    Returns
    -------
    dict
        A dictionary-like object mapping engine names to short descriptions. When
        printed, it is displayed as a simple two-column table.
    
    Examples
    --------
    >>> import randompack
    >>> randompack.engines()
    x256++simd  xorshift256++, with SIMD accelaration (4x64)
    x256++      xoshiro256++, Vigna & Blackman, 2019 (4x64)
    x256**      xoshiro256**, Vigna & Blackman, 2019 (4x64)
    xoro++      xoroshiro128++, Vigna & Blackman, 2016 (2x64)
    x128+       xorshift128+, Vigna, 2014 (2x64)
    pcg64       PCG64-DXSM, O'Neill, 2014 (4x64)
    sfc64       sfc64, Chris Doty-Humphrey, 2013 (4x64)
    cwg128      cwg128-64, Działa, 2022 (5x64)
    philox      Philox-4x64, Salmon & Moraes, 2011 (6x64)
    squares     squares64, Widynski, 2021 (2x64)
    chacha20    ChaCha20, Bernstein, 2008 (6x64)
    """
    
    cdef int n = 0
    cdef int eng_max = 0
    cdef int desc_max = 0
    cdef bytearray eng_buf
    cdef bytearray desc_buf
    cdef char *eng_ptr
    cdef char *desc_ptr
    cdef int i
    if not randompack_engines(NULL, NULL, &n, &eng_max, &desc_max):
        raise RuntimeError("randompack_engines failed")
    if n < 0 or eng_max <= 0 or desc_max <= 0:
        raise RuntimeError("randompack_engines returned invalid sizes")
    eng_buf = bytearray(n * eng_max)
    desc_buf = bytearray(n * desc_max)
    eng_ptr = <char *>PyByteArray_AsString(eng_buf)
    desc_ptr = <char *>PyByteArray_AsString(desc_buf)
    if not randompack_engines(eng_ptr, desc_ptr, &n, &eng_max, &desc_max):
        raise RuntimeError("randompack_engines failed")
    out = _EngineInfo()
    for i in range(n):
        e = bytes(eng_buf[i*eng_max:(i + 1)*eng_max]).split(b"\0", 1)[0]
        d = bytes(desc_buf[i*desc_max:(i + 1)*desc_max]).split(b"\0", 1)[0]
        out[e.decode("utf-8", "replace")] = d.decode("utf-8", "replace")
    return out
