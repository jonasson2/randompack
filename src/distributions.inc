// -*- mode: c; -*-

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
#include "random_standard_normal.inc"
#include "random_standard_exponential.inc"

static void rand_dble(double x[], size_t n, randompack_rng *rng) {
  size_t i;
  align64(rng);
  i = 0;
  while (i < n) {
    fill_buf_if_empty(rng);
    int take = min(n - i, BUFSIZE - rng->buf_word);
    for (int j = 0; j < take; j++) {
      uint64_t v = rng->buf[rng->buf_word + j];
      x[i + j] = (v >> 11) * 0x1.0p-53;
    }
    rng->buf_word += take;
    i += take;
  }
}

static inline void rand_norm(double *x, size_t n, randompack_rng *rng) {
  for (size_t i = 0; i < n; i++) {
    x[i] = random_standard_normal(rng);
  }
}

static inline void rand_exp(double *x, size_t n, double scale, randompack_rng *rng) {
  if (scale == 1.0) {
    for (size_t i = 0; i < n; i++) x[i] = random_standard_exponential(rng);
  }
  else {
    for (size_t i = 0; i < n; i++) x[i] = random_standard_exponential(rng)*scale;
  }
}

static void rand_uint64(uint64_t bound, uint64_t x[], size_t n, randompack_rng *rng) {
  // Sets x to n random integers in {0, 1,..., bound-1}
  if (bound == 0) {
	 draw_u64_copy(x, n, rng);
	 return;
  }
  size_t i = 0;
  uint64_t R;
  if (bound <= UINT8_MAX) {
    uint8_t r, b = bound, thresh = (1u<<8) % b;
    while (i < n) {
      R = draw_u64(rng);
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
      R = draw_u64(rng);
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
      R = draw_u64(rng);
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
      R = draw_u64(rng);
      if (R >= thresh) x[i++] = R % bound;
    }
  }
}

static void rand_uint32(uint32_t bound, uint32_t x[], size_t n, randompack_rng *rng) {
  // Sets x to n random integers in {0, 1,..., bound-1}
  if (bound == 0) {
    draw_u32_copy(x, n, rng);
    return;
  }
  size_t i = 0;
  int j;
  uint64_t R;
  if (bound <= UINT8_MAX) {
    uint8_t r, b = bound, thresh = (1u<<8) % b;
    while (i < n) {
      R = draw_u64(rng);
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
      R = draw_u64(rng);
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
      R = draw_u64(rng);
      r = R;
      if (r >= thresh) x[i++] = r % b;
      if (i>=n) break; // process left (high) half of R
      R >>= 32;
      r = R;
      if (r >= thresh) x[i++] = r % b;
    }
  }
}

static void rand_uint16(uint16_t bound, uint16_t x[], size_t n, randompack_rng *rng) {
  // Sets x to n random integers in {0, 1,..., bound-1}
  if (bound == 0) {
    draw_u16_copy(x, n, rng);
    return;
  }
  size_t i = 0;
  int j;
  uint64_t R;
  if (bound <= UINT8_MAX) {
    uint8_t r, b = bound, thresh = (1u<<8) % b;
    while (i < n) {
      R = draw_u64(rng);
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
      R = draw_u64(rng);
      for (j = 0; j < 4; j++) {
        r = R;
        if (r >= thresh) x[i++] = r % b;
        if (i>=n) break;
        R >>= 16;
      }
    }
  }
}

static void rand_uint8(uint8_t bound, uint8_t x[], size_t n, randompack_rng *rng) {
  // Sets x to n random integers in {0, 1,..., bound-1}
  if (bound == 0) {
    for (size_t i = 0; i < n; i++)
      x[i] = draw_byte(rng);
    return;
  }
  size_t i = 0;
  int j;
  uint64_t R;
  uint8_t r, b = bound, thresh = (1u<<8) % b;
  while (i < n) {
	 R = draw_u64(rng);
	 for (j = 0; j < 8; j++) {
		r = R;
		if (r >= thresh) x[i++] = r % b;
		if (i>=n) break;
		R >>= 8;
	 }
  }
}

static void rand_int_pm_compatible(int bound, int x[], size_t n, randompack_rng *rng) {
  // Sets x to integers in [0, bound), where 1 <= bound <= INT32_MAX - 2.
  rand_uint32(bound, (uint32_t*)x, n, rng);
}

static void rand_perm(int p[], int n, randompack_rng *rng) {
  // Fisher-Yates algorithm, stores a random permutation of 0..n-1 in p
  for (int i = 0; i < n; i++) {
    int j;
    rand_int_pm_compatible(i+1, &j, 1, rng);
    if (j != i) p[i] = p[j];
    p[j] = i;
  }
}

static void rand_sample(int p[], int n, int k, randompack_rng *rng) {
  // Select k values from 0..n-1, c.f. reservoir sampling on Wikipedia
  int r;
  p[0] = 0;
  for (int i = 0; i < k; i++) {
    rand_int_pm_compatible(i+1, &r, 1, rng);
    if (r != i) p[i] = p[r];
    p[r] = i;
  }
  for (int i = k; i < n; i++) {
    rand_int_pm_compatible(i+1, &r, 1, rng);
    if (r < k) p[r] = i;
  }
}

// -*- C -*-
// Gamma RNG (shape, scale): Marsaglia–Tsang for shape>=1, boost trick for shape<1.

#include <math.h>
#include <stdbool.h>
#include <stddef.h>

#include "randompack.h"
#include "randompack_config.h"

static inline double gamma_shape1(double shape, randompack_rng *rng) {
  double d = shape - 1.0/3.0;
  double c = 1.0/sqrt(9.0*d);
  for (;;) {
    double z = random_standard_normal(rng);
    double v = 1.0 + c*z;
    if (v <= 0.0)
      continue;
    double v3 = v*v*v;
    double u = draw_u01(rng);
    if (u <= 0.0)
      continue;
    double z2 = z*z;
    double z4 = z2*z2;
    if (u < 1.0 - 0.0331*z4)
      return d*v3;
    if (log(u) < 0.5*z2 + d*(1.0 - v3 + log(v3)))
      return d*v3;
  }
}

static inline double gamma_shape(double shape, randompack_rng *rng) {
  if (shape >= 1.0)
    return gamma_shape1(shape, rng);
  double u = draw_u01(rng);
  while (u <= 0.0)
    u = draw_u01(rng);
  double g = gamma_shape1(shape + 1.0, rng);
  return g*exp(log(u)/shape);
}

static void rand_gamma(double x[], size_t len, double shape, double scale, randompack_rng
							  *rng) {
  if (scale == 1)
	 for (size_t i=0; i<len; i++) x[i] = gamma_shape(shape, rng);
  else
	 for (size_t i=0; i<len; i++) x[i] = gamma_shape(shape, rng)*scale;
}

static void rand_beta(double x[], size_t len, double a, double b, randompack_rng *rng) {
  for (size_t i=0; i<len; i++) {
    for (;;) {
      double ga = gamma_shape(a, rng);
      double gb = gamma_shape(b, rng);
      double s = ga + gb;
      if (s > 0) {
        x[i] = ga/s;
        break;
      }
    }
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
