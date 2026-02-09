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
    randompack_rng, randompack_philox_ctr,
    randompack_philox_key, randompack_create,
    randompack_duplicate, randompack_free,
    randompack_last_error, randompack_seed,
    randompack_randomize, randompack_engines,
    randompack_int, randompack_long_long,
    randompack_uint32, randompack_uint64,
    randompack_perm, randompack_sample,
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
    randompack_deserialize, randompack_philox_set_state,
    randompack_squares_set_state, randompack_pcg64_set_state,
    randompack_set_state, )

np.import_array()

cdef inline void
_raise_last_error(randompack_rng *rng):
    cdef char *msg = randompack_last_error(rng)
    if msg != NULL and msg[0] != 0:
        raise RuntimeError((<bytes>msg).decode('utf-8', 'replace'))
    raise RuntimeError("randompack error")

cdef inline void
_out_check(randompack_rng *rng, object out, object dtype, object size):
    if rng == NULL:
        raise RuntimeError("RNG pointer is NULL")
    if out is not None and dtype is not None:
        raise ValueError("cannot pass both out and dtype")
    if out is None:
        return
    if not isinstance(out, np.ndarray):
        raise TypeError("out must be a numpy array")
    if not out.flags["C_CONTIGUOUS"]:
        raise ValueError("out must be contiguous")
    if not out.flags["WRITEABLE"]:
        raise ValueError("out must be writable")

cdef inline np.ndarray
_prep_out_float(randompack_rng *rng, object out, object dtype, object size,
                void **ptr, size_t *n_elem, bint *is_f64):
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

cdef inline np.ndarray
_prep_out_int(randompack_rng *rng, object out, object dtype, object size,
              void **ptr, size_t *n_elem, bint *is_i64):
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

cdef class Rng:
    cdef randompack_rng *ptr

    def __cinit__(self, engine=None):
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

    def __dealloc__(self):
        if self.ptr != NULL:
            randompack_free(self.ptr)
            self.ptr = NULL

    def seed(self, seed, spawn_key=None):
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
        if self.ptr == NULL:
            raise RuntimeError("RNG pointer is NULL")
        if not randompack_randomize(self.ptr):
            _raise_last_error(self.ptr)

    def duplicate(self):
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

    def squares_set_state(self, ctr, key):
        cdef uint64_t ctr_v
        cdef uint64_t key_v
        cdef object ctr_i
        cdef object key_i
        if self.ptr == NULL:
            raise RuntimeError("RNG pointer is NULL")
        ctr_i = int(ctr)
        key_i = int(key)
        if ctr_i < 0 or ctr_i > U64_MAX:
            raise ValueError("ctr must be in [0, 2^64-1]")
        if key_i < 0 or key_i > U64_MAX:
            raise ValueError("key must be in [0, 2^64-1]")
        ctr_v = <uint64_t>ctr_i
        key_v = <uint64_t>key_i
        if not randompack_squares_set_state(ctr_v, key_v, self.ptr):
            _raise_last_error(self.ptr)

    def philox_set_state(self, ctr, key):
        cdef randompack_philox_ctr c
        cdef randompack_philox_key k
        cdef int i
        cdef object val
        if self.ptr == NULL:
            raise RuntimeError("RNG pointer is NULL")
        vals_ctr = [int(v) for v in ctr]
        vals_key = [int(v) for v in key]
        if len(vals_ctr) != 4:
            raise ValueError("ctr must have length 4")
        if len(vals_key) != 2:
            raise ValueError("key must have length 2")
        for i in range(4):
            val = vals_ctr[i]
            if val < 0 or val > U64_MAX:
                raise ValueError("ctr entries must be in [0, 2^64-1]")
            c.v[i] = <uint64_t>val
        for i in range(2):
            val = vals_key[i]
            if val < 0 or val > U64_MAX:
                raise ValueError("key entries must be in [0, 2^64-1]")
            k.v[i] = <uint64_t>val
        if not randompack_philox_set_state(c, k, self.ptr):
            _raise_last_error(self.ptr)

    def pcg64_set_state(self, state, inc):
        cdef uint64_t st[4]
        cdef object st_i
        cdef object ic_i
        cdef object mask
        cdef object max_u128
        if self.ptr == NULL:
            raise RuntimeError("RNG pointer is NULL")
        st_i = int(state)
        ic_i = int(inc)
        mask = U64_MAX
        max_u128 = U128_MAX
        if st_i < 0 or st_i > max_u128:
            raise ValueError("state must be in [0, 2^128-1]")
        if ic_i < 0 or ic_i > max_u128:
            raise ValueError("inc must be in [0, 2^128-1]")
        st[0] = <uint64_t>(st_i & mask)
        st[1] = <uint64_t>((st_i >> 64) & mask)
        st[2] = <uint64_t>(ic_i & mask)
        st[3] = <uint64_t>((ic_i >> 64) & mask)
        if not randompack_set_state(st, 4, self.ptr):
            _raise_last_error(self.ptr)

    def set_state(self, state):
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
        return arr

    def normal(self, size=None, *, mu=0.0, sigma=1.0, out=None, dtype=None):
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
        return arr

    def exp(self, size=None, *, scale=1.0, out=None, dtype=None):
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
        return arr

    def lognormal(self, size=None, *, mu=0.0, sigma=1.0, out=None, dtype=None):
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
        return arr

    def gamma(self, size=None, *, shape=1.0, scale=1.0, out=None, dtype=None):
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
        return arr

    def beta(self, size=None, *, a=1.0, b=1.0, out=None, dtype=None):
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
        return arr

    def chi2(self, size=None, *, df=1.0, out=None, dtype=None):
        cdef double df_d = df
        cdef size_t n_elem
        cdef void *ptr
        cdef bint ok, is_f64
        arr = _prep_out_float(self.ptr, out, dtype, size, &ptr, &n_elem, &is_f64)
        with nogil:
            if is_f64:
                ok = randompack_chi2(<double *>ptr, n_elem, df_d, self.ptr)
            else:
                ok = randompack_chi2f(<float *>ptr, n_elem, <float>df_d, self.ptr)
        if not ok:
            _raise_last_error(self.ptr)
        return arr

    def t(self, size=None, *, df=1.0, out=None, dtype=None):
        cdef double df_d = df
        cdef size_t n_elem
        cdef void *ptr
        cdef bint ok, is_f64
        arr = _prep_out_float(self.ptr, out, dtype, size, &ptr, &n_elem, &is_f64)
        with nogil:
            if is_f64:
                ok = randompack_t(<double *>ptr, n_elem, df_d, self.ptr)
            else:
                ok = randompack_tf(<float *>ptr, n_elem, <float>df_d, self.ptr)
        if not ok:
            _raise_last_error(self.ptr)
        return arr

    def f(self, size=None, *, dfn=1.0, dfd=1.0, out=None, dtype=None):
        cdef double dfn_d = dfn
        cdef double dfd_d = dfd
        cdef size_t n_elem
        cdef void *ptr
        cdef bint ok, is_f64
        arr = _prep_out_float(self.ptr, out, dtype, size, &ptr, &n_elem, &is_f64)
        with nogil:
            if is_f64:
                ok = randompack_f(<double *>ptr, n_elem, dfn_d, dfd_d, self.ptr)
            else:
                ok = randompack_ff(<float *>ptr, n_elem, <float>dfn_d,
                                   <float>dfd_d, self.ptr)
        if not ok:
            _raise_last_error(self.ptr)
        return arr

    def gumbel(self, size=None, *, mu=0.0, beta=1.0, out=None, dtype=None):
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
        return arr

    def pareto(self, size=None, *, a=1.0, xm=1.0, out=None, dtype=None):
        cdef double a_d = a
        cdef double xm_d = xm
        cdef size_t n_elem
        cdef void *ptr
        cdef bint ok, is_f64
        arr = _prep_out_float(self.ptr, out, dtype, size, &ptr, &n_elem, &is_f64)
        with nogil:
            if is_f64:
                ok = randompack_pareto(<double *>ptr, n_elem, a_d, xm_d, self.ptr)
            else:
                ok = randompack_paretof(<float *>ptr, n_elem, <float>a_d,
                                        <float>xm_d, self.ptr)
        if not ok:
            _raise_last_error(self.ptr)
        return arr

    def weibull(self, size=None, *, k=1.0, lam=1.0, out=None, dtype=None):
        cdef double k_d = k
        cdef double lam_d = lam
        cdef size_t n_elem
        cdef void *ptr
        cdef bint ok, is_f64
        arr = _prep_out_float(self.ptr, out, dtype, size, &ptr, &n_elem, &is_f64)
        with nogil:
            if is_f64:
                ok = randompack_weibull(<double *>ptr, n_elem, k_d, lam_d,
                                        self.ptr)
            else:
                ok = randompack_weibullf(<float *>ptr, n_elem, <float>k_d,
                                         <float>lam_d, self.ptr)
        if not ok:
            _raise_last_error(self.ptr)
        return arr

    def skew_normal(self, size=None, *, mu=0.0, sigma=1.0, alpha=0.0,
                    out=None, dtype=None):
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
        return arr

    def mvn(self, size=None, Sigma=None, mu=None, out=None):
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
        return arr

    def perm(self, n, *, out=None):
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
        return arr

    def sample(self, n, k, *, out=None):
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
        return arr

def engines():
    raise NotImplementedError("engines not implemented yet")
