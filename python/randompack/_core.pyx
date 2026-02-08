import numpy as np
cimport numpy as np

from libc.stddef cimport size_t
from libc.limits cimport INT_MIN, INT_MAX, LLONG_MIN, LLONG_MAX
from libc.stdint cimport uint8_t, uint32_t, uint64_t

from cpython.bytearray cimport PyByteArray_AsString
from cpython.bytes cimport PyBytes_AS_STRING, PyBytes_GET_SIZE

from ._core cimport (
    randompack_rng, randompack_philox_ctr,
    randompack_philox_key, randompack_create,
    randompack_duplicate, randompack_free,
    randompack_last_error, randompack_seed,
    randompack_randomize, randompack_engines,
    randompack_int, randompack_uint32,
    randompack_uint64, randompack_unif,
    randompack_normal, randompack_lognormal,
    randompack_exp, randompack_gamma,
    randompack_beta, randompack_chi2,
    randompack_t, randompack_f,
    randompack_gumbel, randompack_pareto,
    randompack_weibull, randompack_skew_normal,
    randompack_uniff, randompack_normalf,
    randompack_lognormalf, randompack_expf,
    randompack_gammaf, randompack_betaf,
    randompack_chi2f, randompack_tf,
    randompack_ff, randompack_gumbelf,
    randompack_paretof, randompack_weibullf,
    randompack_skew_normalf, randompack_serialize,
    randompack_deserialize, randompack_philox_set_state,
    randompack_squares_set_state, )

np.import_array()

cdef inline void
_raise_last_error(randompack_rng *rng):
    cdef char *msg = randompack_last_error(rng)
    if msg != NULL and msg[0] != 0:
        raise RuntimeError((<bytes>msg).decode())
    raise RuntimeError("randompack error")

cdef inline void
_out_check(randompack_rng *rng, object out, object dtype, object n):
    if rng == NULL:
        raise RuntimeError("RNG pointer is NULL")
    if out is not None and dtype is not None:
        raise ValueError("cannot pass both out and dtype")
    if out is None:
        if n is None:
            raise ValueError("n is required when out is not provided")
        return
    if not isinstance(out, np.ndarray):
        raise TypeError("out must be a numpy array")
    if not out.flags["C_CONTIGUOUS"]:
        raise ValueError("out must be contiguous")

cdef inline np.ndarray
_prep_out_float(randompack_rng *rng, object out, object dtype, object n,
                void **ptr, size_t *n_elem, bint *is_f64):
    cdef np.ndarray arr
    cdef object dt
    _out_check(rng, out, dtype, n)
    if out is None:
        dt = np.float64 if dtype is None else np.dtype(dtype)
        if dt != np.float32 and dt != np.float64:
            raise TypeError("dtype must be float32 or float64")
        arr = np.empty(n, dtype=dt)
    else:
        arr = out
        if arr.dtype != np.float32 and arr.dtype != np.float64:
            raise TypeError("out must be float32 or float64")
    ptr[0] = <void *>np.PyArray_DATA(arr)
    n_elem[0] = <size_t>np.PyArray_SIZE(arr)
    is_f64[0] = arr.dtype == np.float64
    return arr

cdef inline np.ndarray
_prep_out_int(randompack_rng *rng, object out, object dtype, object n,
              void **ptr, size_t *n_elem, bint *is_i64):
    cdef np.ndarray arr
    cdef object dt
    _out_check(rng, out, dtype, n)
    if out is None:
        dt = np.int64 if dtype is None else np.dtype(dtype)
        if dt != np.int32 and dt != np.int64:
            raise TypeError("dtype must be int32 or int64")
        arr = np.empty(n, dtype=dt)
    else:
        arr = out
        if arr.dtype != np.int32 and arr.dtype != np.int64:
            raise TypeError("out must be int32 or int64")
    ptr[0] = <void *>np.PyArray_DATA(arr)
    n_elem[0] = <size_t>np.PyArray_SIZE(arr)
    is_i64[0] = arr.dtype == np.int64
    return arr

cdef Rng _wrap_ptr(randompack_rng *ptr):
    cdef Rng obj = Rng.__new__(Rng)
    obj.ptr = ptr
    return obj

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
            if v < 0 or v > 4294967295:
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
        if self.ptr == NULL:
            raise RuntimeError("RNG pointer is NULL")
        dup = randompack_duplicate(self.ptr)
        if dup == NULL:
            _raise_last_error(self.ptr)
        return _wrap_ptr(dup)

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
        if ctr_i < 0 or ctr_i > 18446744073709551615:
            raise ValueError("ctr out of range for uint64")
        if key_i < 0 or key_i > 18446744073709551615:
            raise ValueError("key out of range for uint64")
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
            if val < 0 or val > 18446744073709551615:
                raise ValueError("ctr entries out of range for uint64")
            c.v[i] = <uint64_t>val
        for i in range(2):
            val = vals_key[i]
            if val < 0 or val > 18446744073709551615:
                raise ValueError("key entries out of range for uint64")
            k.v[i] = <uint64_t>val
        if not randompack_philox_set_state(c, k, self.ptr):
            _raise_last_error(self.ptr)

    def unif(self, n=None, *, a=0.0, b=1.0, out=None, dtype=None):
        cdef double a_d, b_d
        cdef size_t n_elem
        cdef void *ptr
        cdef bint ok, is_f64
        arr = _prep_out_float(self.ptr, out, dtype, n, &ptr, &n_elem, &is_f64)
        a_d = a
        b_d = b
        if is_f64:
            with nogil:
                ok = randompack_unif(<double *>ptr, n_elem, a_d, b_d, self.ptr)
        else:
            with nogil:
                ok = randompack_uniff(<float *>ptr, n_elem, <float>a_d,
                                      <float>b_d, self.ptr)
        if not ok:
            _raise_last_error(self.ptr)
        return arr

    def normal(self, n=None, *, mu=0.0, sigma=1.0, out=None, dtype=None):
        cdef double mu_d, sigma_d
        cdef size_t n_elem
        cdef void *ptr
        cdef bint ok, is_f64
        arr = _prep_out_float(self.ptr, out, dtype, n, &ptr, &n_elem, &is_f64)
        mu_d = mu
        sigma_d = sigma
        if is_f64:
            with nogil:
                ok = randompack_normal(<double *>ptr, n_elem, mu_d, sigma_d, self.ptr)
        else:
            with nogil:
                ok = randompack_normalf(<float *>ptr, n_elem, <float>mu_d,
                                        <float>sigma_d, self.ptr)
        if not ok:
            _raise_last_error(self.ptr)
        return arr

    def exp(self, n=None, *, scale=1.0, out=None, dtype=None):
        cdef double scaled = scale
        cdef size_t n_elem
        cdef void *ptr
        cdef bint ok, is_f64
        arr = _prep_out_float(self.ptr, out, dtype, n, &ptr, &n_elem, &is_f64)
        if is_f64:
            with nogil:
                ok = randompack_exp(<double *>ptr, n_elem, scaled, self.ptr)
        else:
            with nogil:
                ok = randompack_expf(<float *>ptr, n_elem, <float>scaled, self.ptr)
        if not ok:
            _raise_last_error(self.ptr)
        return arr

    def int(self, a=None, b=None, *, size=None, out=None, dtype=None):
        cdef np.ndarray arr
        cdef size_t n_elem, i
        cdef void *ptr
        cdef bint ok, is_i64
        cdef int a32, b32
        cdef uint64_t bound, offset
        cdef uint64_t *u64
        cdef object a_i, b_i
        arr = _prep_out_int(self.ptr, out, dtype, size, &ptr, &n_elem, &is_i64)
        if (a is None) != (b is None):
            raise ValueError("a and b must be given together")
        if a is None:
            if is_i64:
                with nogil:
                    ok = randompack_uint64(<uint64_t *>ptr, n_elem, 0, self.ptr)
            else:
                with nogil:
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
                with nogil:
                    ok = randompack_int(<int *>ptr, n_elem, a32, b32, self.ptr)
            else:
                if a_i < LLONG_MIN or a_i > LLONG_MAX:
                    raise ValueError("a out of range for int64")
                if b_i < LLONG_MIN or b_i > LLONG_MAX:
                    raise ValueError("b out of range for int64")
                if a_i == LLONG_MIN and b_i == LLONG_MAX:
                    bound = 0
                    offset = 0
                else:
                    bound = b_i - a_i + 1
                    if bound == 0:
                        raise ValueError("range too large for uint64")
                    offset = <uint64_t>a_i
                with nogil:
                    ok = randompack_uint64(<uint64_t *>ptr, n_elem, bound, self.ptr)
                if ok and offset != 0:
                    u64 = <uint64_t *>ptr
                    for i in range(n_elem):
                        u64[i] = u64[i] + offset
        if not ok:
            _raise_last_error(self.ptr)
        return arr

def engines():
    raise NotImplementedError("engines not implemented yet")
