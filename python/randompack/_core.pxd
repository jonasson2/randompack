from libc.stddef cimport size_t
from libc.stdint cimport uint8_t, uint32_t, uint64_t

cdef extern from "randompack.h":
    ctypedef struct randompack_rng

    randompack_rng *randompack_create(const char *engine)
    randompack_rng *randompack_duplicate(randompack_rng *rng)
    void randompack_free(randompack_rng *rng)
    char *randompack_last_error(randompack_rng *rng)
    bint randompack_seed(int seed, uint32_t *spawn_key, int n_key, randompack_rng *rng)
    bint randompack_randomize(randompack_rng *rng)
    bint randompack_full_mantissa(randompack_rng *rng, bint enable)
    bint randompack_bitexact(randompack_rng *rng, bint enable)
    bint randompack_jump(int p, randompack_rng *rng)
    bint randompack_engines(char *engines, char *descriptions, int *nengines,
                            int *eng_maxlen, int *desc_maxlen)

    bint randompack_int(int *x, size_t len, int m, int n,
                        randompack_rng *rng) nogil
    bint randompack_long_long(long long *x, size_t len, long long m, long long n,
                              randompack_rng *rng) nogil
    bint randompack_uint32(uint32_t *x, size_t len, uint32_t bound,
                           randompack_rng *rng) nogil
    bint randompack_uint64(uint64_t *x, size_t len, uint64_t bound,
                           randompack_rng *rng) nogil
    bint randompack_perm(int *x, int n, randompack_rng *rng) nogil
    bint randompack_sample(int *x, int n, int k, randompack_rng *rng) nogil
    bint randompack_raw(void *out, size_t nbytes,
                        randompack_rng *rng) nogil

    bint randompack_unif(double *x, size_t n, double a, double b,
                         randompack_rng *rng) nogil
    bint randompack_normal(double *x, size_t n, double mu, double sigma,
                           randompack_rng *rng) nogil
    bint randompack_lognormal(double *x, size_t n, double mu, double sigma,
                              randompack_rng *rng) nogil
    bint randompack_exp(double *x, size_t n, double scale,
                        randompack_rng *rng) nogil
    bint randompack_gamma(double *x, size_t n, double shape, double scale,
                          randompack_rng *rng) nogil
    bint randompack_beta(double *x, size_t n, double a, double b,
                         randompack_rng *rng) nogil
    bint randompack_chi2(double *x, size_t n, double nu,
                         randompack_rng *rng) nogil
    bint randompack_t(double *x, size_t n, double nu,
                      randompack_rng *rng) nogil
    bint randompack_f(double *x, size_t n, double nu1, double nu2,
                      randompack_rng *rng) nogil
    bint randompack_gumbel(double *x, size_t n, double mu, double beta,
                           randompack_rng *rng) nogil
    bint randompack_pareto(double *x, size_t n, double xm, double alpha,
                           randompack_rng *rng) nogil
    bint randompack_weibull(double *x, size_t n, double shape, double scale,
                            randompack_rng *rng) nogil
    bint randompack_skew_normal(double *x, size_t n, double mu, double sigma,
                                double alpha, randompack_rng *rng) nogil
    bint randompack_mvn(char *transp, double *mu, double *Sig, int d, size_t n,
                        double *X, int ldX, double *L,
                        randompack_rng *rng) nogil

    bint randompack_uniff(float *x, size_t n, float a, float b,
                          randompack_rng *rng) nogil
    bint randompack_normalf(float *x, size_t n, float mu, float sigma,
                            randompack_rng *rng) nogil
    bint randompack_lognormalf(float *x, size_t n, float mu, float sigma,
                               randompack_rng *rng) nogil
    bint randompack_expf(float *x, size_t n, float scale,
                         randompack_rng *rng) nogil
    bint randompack_gammaf(float *x, size_t n, float shape, float scale,
                           randompack_rng *rng) nogil
    bint randompack_betaf(float *x, size_t n, float a, float b,
                          randompack_rng *rng) nogil
    bint randompack_chi2f(float *x, size_t n, float nu,
                          randompack_rng *rng) nogil
    bint randompack_tf(float *x, size_t n, float nu,
                       randompack_rng *rng) nogil
    bint randompack_ff(float *x, size_t n, float nu1, float nu2,
                       randompack_rng *rng) nogil
    bint randompack_gumbelf(float *x, size_t n, float mu, float beta,
                            randompack_rng *rng) nogil
    bint randompack_paretof(float *x, size_t n, float xm, float alpha,
                            randompack_rng *rng) nogil
    bint randompack_weibullf(float *x, size_t n, float shape, float scale,
                             randompack_rng *rng) nogil
    bint randompack_skew_normalf(float *x, size_t n, float mu, float sigma,
                                 float alpha, randompack_rng *rng) nogil

    bint randompack_serialize(uint8_t *buf, int *len, randompack_rng *rng)
    bint randompack_deserialize(const uint8_t *buf, int len,
                                randompack_rng *rng)
    bint randompack_pcg64_set_inc(uint64_t inc[2], randompack_rng *rng)
    bint randompack_philox_set_ctr(uint64_t ctr[4], randompack_rng *rng)
    bint randompack_philox_set_key(uint64_t key[2], randompack_rng *rng)
    bint randompack_sfc64_set_abc(uint64_t abc[3], randompack_rng *rng)
    bint randompack_squares_set_ctr(uint64_t ctr, randompack_rng *rng)
    bint randompack_squares_set_key(uint64_t key, randompack_rng *rng)
    bint randompack_set_state(uint64_t *state, int nstate,
                              randompack_rng *rng)
