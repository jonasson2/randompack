// -*- mode: c; -*-
#include <float.h>

static int LastNonzeroColumn(int m, int n, double A[]) {
  // If they are all zero, return 0.
  int k = n-1;
  while (k >= 0) {
    int imx = iamax(m, A+k*m, 1);
    if (A[imx + k*m] > 0) return k + 1;
    k--;
  }
  return k + 2;
}

//#include "zmgd.inc"   // static zmgd() lives here
#include "norm_exp.inc"
#include "rand_dble.inc"

// ALWAYS_INLINE void rand_dble(double x[], size_t len, randompack_rng *rng) {
//   int w = enter_u64_mode(rng);
//   size_t i = 0, remain, avail, take;
//   //  for (int j=0; j<BUFSIZE; j++) rng->buf.u64[j] = 12345678901234ULL;
//   while (i < len) {
//     fill_if_empty(rng, &w);
//     //w = 0;
//     remain = len - i;
//     avail = (size_t)(BUFSIZE - w);
//     take = remain < avail ? remain : avail;
//     for (size_t j = 0; j < take; j++) {
//       uint64_t v = rng->buf.u64[w + j];
//       x[i + j] = U64_TO_DOUBLE(v);
//     }
//     w += (int)take;
//     i += take;
//   }
//   exit_u64_mode(rng, w);
// }

static void rand_uint64(uint64_t x[], size_t n, uint64_t bound, randompack_rng *rng) {
  // Sets x to n random integers in {0, 1,..., bound-1}
  if (bound == 0) {
    draw_u64_copy(x, n, rng);
    return;
  }
  size_t i = 0;
  uint64_t R;
  int w = enter_u32_mode(rng);
  if (bound <= UINT8_MAX) {
    uint8_t r, b = bound, thresh = (1u<<8) % b;
    while (i < n) {
      R = draw_u64(rng, &w);
      for (int j = 0; j < 8; j++) {
        r = R;
        if (r >= thresh) x[i++] = r % b;
        if (i>=n) break;
        R >>= 8;
      }
    }
  }
  else if (bound <= UINT16_MAX) {
    uint16_t r, b = bound, thresh = (1u<<16) % b;
    while (i < n) {
      R = draw_u64(rng, &w);
      for (int j = 0; j < 4; j++) {
        r = R;
        if (r >= thresh) x[i++] = r % b;
        if (i>=n) break;
        R >>= 16;
      }
    }
  }
  else if (bound <= UINT32_MAX) {
    uint32_t r, b = bound, M = 0-b, thresh = M % b;
    while (i < n) {
      R = draw_u64(rng, &w);
      r = R;
      if (r >= thresh) x[i++] = r % b;
      if (i>=n) break;
      R >>= 32;
      r = R;
      if (r >= thresh) x[i++] = r % b;
    }
  }
  else {
    uint64_t M = 0-bound, thresh = M % bound;
    while (i < n) {
      R = draw_u64(rng, &w);
      if (R >= thresh) x[i++] = R % bound;
    }
  }
  exit_u32_mode(rng, w);
}

static void rand_uint32(uint32_t x[], size_t n, uint32_t bound, randompack_rng *rng) {
  // Sets x to n random integers in {0, 1,..., bound-1}
  if (bound == 0) {
    draw_u32_copy(x, n, rng);
    return;
  }
  size_t i = 0;
  int j;
  uint64_t R;
  int w = enter_u32_mode(rng);
  if (bound <= UINT8_MAX) {
    uint8_t r, b = bound, thresh = (1u<<8) % b;
    while (i < n) {
      R = draw_u64(rng, &w);
      for (j = 0; j < 8; j++) {
        r = R;
        if (r >= thresh) x[i++] = r % b;
        if (i>=n) break;
        R >>= 8;
      }
    }
  }
  else if (bound <= UINT16_MAX) {
    uint16_t r, b = bound, thresh = (1u<<16) % b;
    while (i < n) {
      R = draw_u64(rng, &w);
      for (j = 0; j < 4; j++) {
        r = R;
        if (r >= thresh) x[i++] = r % b;
        if (i>=n) break;
        R >>= 16;
      }
    }
  }
  else {
    uint32_t r, b = bound, M = 0-b, thresh = M % b;
    while (i < n) {
      R = draw_u64(rng, &w);
      r = R;
      if (r >= thresh) x[i++] = r % b;
      if (i>=n) break; // process left (high) half of R
      R >>= 32;
      r = R;
      if (r >= thresh) x[i++] = r % b;
    }
  }
  exit_u32_mode(rng, w);
}

static void rand_uint16(uint16_t x[], size_t n, uint16_t bound, randompack_rng *rng) {
  // Sets x to n random integers in {0, 1,..., bound-1}
  if (bound == 0) {
    draw_u16_copy(x, n, rng);
    return;
  }
  size_t i = 0, j;
  uint64_t R;
  int w = enter_u32_mode(rng);
  if (bound <= UINT8_MAX) {
    uint8_t r, b = bound, thresh = (1u<<8) % b;
    while (i < n) {
      R = draw_u64(rng, &w);
      for (j = 0; j < 8; j++) {
        r = R;
        if (r >= thresh) x[i++] = r % b;
        if (i>=n) break;
        R >>= 8;
      }
    }
  }
  else {
    uint16_t r, b = bound, thresh = (1u<<16) % b;
    while (i < n) {
      R = draw_u64(rng, &w);
      for (j = 0; j < 4; j++) {
        r = R;
        if (r >= thresh) x[i++] = r % b;
        if (i>=n) break;
        R >>= 16;
      }
    }
  }
  exit_u32_mode(rng, w);
}

static void rand_uint8(uint8_t x[], size_t n, uint8_t bound, randompack_rng *rng) {
  // Sets x to n random integers in {0, 1,..., bound-1}
  if (bound == 0) {
    for (size_t i = 0; i < n; i++)
      x[i] = draw_byte(rng);
    return;
  }
  size_t i = 0, j;
  uint64_t R;
  uint8_t r, b = bound, thresh = (1u<<8) % b;
  int w = enter_u32_mode(rng);
  while (i < n) {
    R = draw_u64(rng, &w);
    for (j = 0; j < 8; j++) {
      r = R;
      if (r >= thresh) x[i++] = r % b;
      if (i>=n) break;
      R >>= 8;
    }
  }
  exit_u32_mode(rng, w);
}

ALWAYS_INLINE void rand_int(int bound, int x[], size_t n, randompack_rng *rng) {
  rand_uint32((uint32_t*)x, n, bound, rng);
}

static void rand_perm(int p[], int n, randompack_rng *rng) {
  // Fisher-Yates algorithm, stores a random permutation of 0..n-1 in p
  for (int i = 0; i < n; i++) {
    int j;
    rand_int(i+1, &j, 1, rng);
    if (j != i) p[i] = p[j];
    p[j] = i;
  }
}

static inline bool hashtable_has(int *keys, int mask, int v) {
  uint32_t h = mix32((uint32_t)v);
  int i = (int)(h & (uint32_t)mask);
  for (;;) {
    int k = keys[i];
    if (k == -1) return false;
    if (k == v) return true;
    i = (i + 1) & mask;
  }
}

static inline void hashtable_add(int *keys, int mask, int v) {
  uint32_t h = mix32((uint32_t)v);
  int i = (int)(h & (uint32_t)mask);
  for (;;) {
    int k = keys[i];
    if (k == -1 || k == v) { keys[i] = v; return; }
    i = (i + 1) & mask;
  }
}

static void rand_sample_reservoir(int p[], int n, int k, randompack_rng *rng) {
  // Select k values from 0..n-1, c.f. reservoir sampling on Wikipedia
  int r;
  p[0] = 0;
  for (int i = 0; i < k; i++) {
    rand_int(i+1, &r, 1, rng);
    if (r != i) p[i] = p[r];
    p[r] = i;
  }
  for (int i = k; i < n; i++) {
    rand_int(i+1, &r, 1, rng);
    if (r < k) p[r] = i;
  }
}

static void rand_sample_floyd(int p[], int n, int k, randompack_rng *rng) {
  // Floyd: O(k) expected. p[0..k-1] gets k distinct integers from 0..n-1.
  if (k <= 0) return;
  int m = 1;
  while (m < 2*k) m <<= 1;
  int mask = m - 1;
  int *keys;
  if (!ALLOC(keys, m)) {
    rand_sample_reservoir(p, n, k, rng);
    return;
  }
  for (int i = 0; i < m; i++) keys[i] = -1;
  int out = 0;
  for (int j = n - k; j < n; j++) {
    int t;
    rand_int(j + 1, &t, 1, rng);
    int pick = hashtable_has(keys, mask, t) ? j : t;
    hashtable_add(keys, mask, pick);
    p[out++] = pick;
  }
  FREE(keys);
}

static void rand_sample(int p[], int n, int k, randompack_rng *rng) {
  if (k <= n/2) rand_sample_floyd(p, n, k, rng);
  else rand_sample_reservoir(p, n, k, rng);
}

// -*- C -*-
// Gamma RNG (shape, scale): Marsaglia–Tsang for shape>=1, boost trick for shape<1.

#include <math.h>
#include <stdbool.h>
#include <stddef.h>

#include "randompack.h"
#include "randompack_config.h"

ALWAYS_INLINE void fill_gamma_shape1(double *x, size_t len, double shape,
                                            randompack_rng *rng) {
  double d = shape - 1.0/3.0;
  double c = 1.0/sqrt(9.0*d);
  rand_norm(x, len, rng);
  int w = enter_u64_mode(rng);
  //for (size_t i = 0; i < len; i++) x[i] = draw_norm(rng, &w);
  for (size_t i = 0; i < len; i++) {
    for (;;) {
      double z = x[i];
      double v = 1.0 + c*z;
      if (v > 0.0) {
        double v3 = v*v*v;
        double u  = draw_u01(rng, &w);
        double z2 = z*z;
        double z4 = z2*z2;
        if (u < 1.0 - 0.0331*z4 ||
            log(u) < 0.5*z2 + d*(1.0 - v3 + log(v3))) {
          x[i] = d*v3;
          break;
        }
      }
		make_half_full(rng, &w);
      x[i] = draw_norm(rng, &w);
    }
  }
  exit_u64_mode(rng, w);
}

ALWAYS_INLINE void fill_gamma(double *x, size_t len, double shape, double scale,
                                     randompack_rng *rng) {
  if (shape >= 1.0) {
    fill_gamma_shape1(x, len, shape, rng);
  }
  else {
    fill_gamma_shape1(x, len, shape + 1, rng);
	 int w = enter_u64_mode(rng);
    for (size_t i = 0; i < len; i++) {
      double u = draw_u01(rng, &w);
      x[i] *= exp(log(u)/shape);
    }
	 exit_u64_mode(rng, w);
  }
  if (scale != 1) for (size_t i = 0; i < len; i++) x[i] *= scale;
}

ALWAYS_INLINE void fill_beta(double *x, size_t len, double a, double b,
                                    randompack_rng *rng) {
  // X = Ga/(Ga + Gb) ~ Beta(a,b) if Ga ~ Gamma(a,1) and Gb ~ Gamma(b,1)
  fill_gamma(x, len, a, 1.0, rng);
  double gb[64];
  size_t i = 0;
  while (i < len) {
    int n = min(64, len - i);
    fill_gamma(gb, n, b, 1.0, rng);
    double hi = 1.0 - DBL_EPSILON;
    for (int j = 0; j < n; j++) { // force the beta-draws to (0, 1)
      double a = x[i + j];
      double b = gb[j];
      double y = a / (a + b);
      if (y < DBL_MIN) y = DBL_MIN;
      if (y > hi)      y = hi;
      x[i + j] = y;
    }	 
    i += n;
  }
}

ALWAYS_INLINE void fill_t(double *x, size_t len, double nu, randompack_rng *rng) {
  // Z ~ N(0,1), U ~ Gamma(nu/2, 2) independent => X = Z/sqrt(U/nu) ~ t_nu
  double u[64];
  rand_norm(x, len, rng);
  size_t i = 0;
  while (i < len) {
    int n = min(64, len - i);
    fill_gamma(u, n, 0.5*nu, 2.0, rng);
    for (int j = 0; j < n; j++)
      x[i + j] /= sqrt(u[j]/nu);
    i += n;
  }
}

ALWAYS_INLINE void fill_f(double *x, size_t len, double nu1, double nu2,
                                 randompack_rng *rng) {
  // F(nu1,nu2) via gamma ratio:
  //   F = (X1*nu2)/(X2*nu1) where X1 ~ Gamma(nu1/2,1), X2 ~ Gamma(nu2/2,1)
  double x2[64];
  fill_gamma(x, len, nu1/2.0, 1.0, rng);
  size_t i = 0;
  while (i < len) {
    int n = min(len - i, 64);
    fill_gamma(x2, n, nu2/2.0, 1.0, rng);
    for (int j = 0; j < n; j++)
      x[i + j] = (x[i + j]*nu2)/(x2[j]*nu1);
    i += n;
  }
}

ALWAYS_INLINE void fill_chi2(double *x, size_t len, double nu,
  randompack_rng *rng) {
  fill_gamma(x, len, 0.5*nu, 2, rng);
}

ALWAYS_INLINE void fill_gumbel(double *x, size_t len, double mu, double beta,
  randompack_rng *rng) {
  rand_dble(x, len, rng); // x in [0,1)
  for (size_t i = 0; i < len; i++)
    x[i] = mu - beta*log(-log(x[i]));
}

ALWAYS_INLINE void fill_pareto(double *x, size_t len, double xm, double alpha,
  randompack_rng *rng) {
  rand_exp(x, len, rng);
  for (size_t i = 0; i < len; i++)
    x[i] = xm*exp(x[i]/alpha);
}

ALWAYS_INLINE void fill_weibull(double *x, size_t len, double shape,
  double scale, randompack_rng *rng) {
  rand_exp(x, len, rng); // x[i] = E ~ Exp(1) using ziggurat exponential
  double inv_shape = 1/shape;
  for (size_t i = 0; i < len; i++)
    x[i] = scale*pow(x[i], inv_shape);
}

ALWAYS_INLINE void fill_skew_normal(double *x, size_t len, double mu,
  double sigma, double alpha, randompack_rng *rng) {
  double denom = sqrt(1 + alpha*alpha);
  double delta = alpha/denom;
  double scale = 1/denom;
  rand_norm(x, len, rng);
  size_t i = 0;
  double u[64];
  while (i < len) {
    int n = min(64, len - i);
    rand_norm(u, n, rng);
    for (int j = 0; j < n; j++) {
      double v = delta*fabs(u[j]) + scale*x[i + j];
      x[i + j] = mu + sigma*v;
    }
    i += n;
  }
}

// NOTE 1: There are situations when Sig is indefinite but close to being positive
//         definite, for example due to rounding errors. To remedy this a pivoted
//         Cholesky factorization of Sig is employed, which may return an L matrix
//         which is not lower and has some trailing columns 0, but does satisfy
//         L·L' = Sig, and can thus be used to generate vectors with the right
//         distribution.
// NOTE 2: In case the calling program needs the Cholesky factor of Sig this may
//         be returned in L by letting it be a d × d array instead of 0 in the
//         call.
// NOTE 3: When randompack_mvn is to be called multiple times for the same
//         covariance it is possible to save execution time by reusing the
//         Cholesky factorization of Sig on all calls but the first. Specify Sig
//         and return L on the first call, and let Sig be null and specify L on
//         subsequent calls. If both Sig and L are null, the function exits with
//         ok = 0.
static bool rand_mvn(char transp, double mu[], double Sig[], int d, size_t n,
                     double X[], int ldX, double L[], randompack_rng *rng) {
  int info, rank;
  bool Lalloc = false;
  bool TRAN = (transp == 'T');
  double *work = 0, *A = 0, *Wrk = 0;
  int *piv = 0;
  if (Sig) { // Sig needs to be factorized
    if (!L && X == 0) return true;
    if (!L) {
      if (!ALLOC(L, d*d)) goto fail;
      Lalloc = true;
    }
    else {
      laset("Upper", d, d, 0.0, 0.0, L, d);
    }
    if (!ALLOC(work, 2*d)) goto fail;
    if (!ALLOC(piv, d))    goto fail;
    lacpy("Lower", d, d, Sig, d, L, d);
    potrf("Low", d, L, d, &info);
    if (info > 0) {
      lacpy("Lower", d, d, Sig, d, L, d);
      pstrf("Low", d, L, d, piv, &rank, 1e-14, work, &info); // PSD Chol fact.
      if (!ALLOC(A, rank*d)) goto fail;
      copy(rank*d, L, 1, A, 1);
      for (int j = 0; j < d; j++) // Set L to [A(piv, 1:rank) 0]
        copy(rank, A + piv[j], d, L + j, d);
      laset("All", d, d-rank, 0.0, 0.0, L + d*rank, d);
      FREE(A);
    }
    FREE(piv);
    FREE(work);
  }
  rank = LastNonzeroColumn(d, d, L);
  if (n == 0) {
    if (Lalloc) FREE(L);
    return true;
  }
  if (rank == d) {
    if (TRAN) {
      for (size_t i = 0; i < n; i++)
        rand_norm(X + i*ldX, d, rng);
      trmm("Left", "Lower", "NoT", "NotUdia", d, n, 1.0, L, d, X, ldX);
    }
    else {
      for (int i = 0; i < d; i++)
        rand_norm(X + i*ldX, n, rng);
      trmm("Right", "Lower", "T", "NotUdia", n, d, 1.0, L, d, X, ldX);
    }
  }
  else { // Singular Sig, use workspace for the N(0,1) randoms
    if (!ALLOC(Wrk, n*rank)) goto fail;
    rand_norm(Wrk, n*rank, rng);
    if (TRAN)
      gemm("NoT", "NoT", d, n, rank, 1.0, L, d, Wrk, rank, 0.0, X, ldX);
    else
      gemm("NoT", "T", n, d, rank, 1.0, Wrk, n, L, d, 0.0, X, ldX);
    FREE(Wrk);
  }
  if (mu) {
    size_t i;
    if (TRAN) for (i=0; i<n; i++) axpy(d, 1.0, mu, 1, X + i*d, 1); // add mu to each col
    else      for (i=0; i<n; i++) axpy(d, 1.0, mu, 1, X + i, n);   // add mu to each row
  }
  if (Lalloc) FREE(L);
  return true;
 fail:
  FREE(Wrk);
  FREE(A);
  FREE(piv);
  FREE(work);
  if (Lalloc && L) FREE(L);
  return false;
}

// #include "rand_float.inc"
// #include "distrib_float.inc"
