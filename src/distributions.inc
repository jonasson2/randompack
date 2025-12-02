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

#include "zmgd.inc"   // static zmgd() lives here

static void rand_normal(double x[], int n, randompack_rng *rng) {
  for (int i = 0; i < n; i++)
    x[i] = zmgd(rng, rng);
}

static void rand_uint64(uint64_t bound, uint64_t x[], int n, randompack_rng *rng) {
  // Sets x to n random integers in {0, 1,..., bound-1}
  if (bound == 0) return;
  int i = 0, j;
  uint64_t R;
  if (bound <= UINT8_MAX) {
    uint8_t r, b = (uint8_t)bound, thresh = (1u<<8) % b;
    while (i < n) {
      R = rng->next(rng);
      for (j = 0; j<8; j++) {
        r = (uint8_t)R;
        if (r >= thresh) x[i++] = (uint64_t)(r % b);
        if (i>=n) break;
        R >>= 8;
      }
    }
  }
  else if (bound <= (uint64_t)UINT16_MAX) {
    uint16_t r, b = (uint16_t)bound, thresh = (1u<<16) % b;
    while (i < n) {
      R = rng->next(rng);
      for (j = 0; j<4; j++) {
        r = (uint16_t)R;
        if (r >= thresh) x[i++] = (uint64_t)(r % b);
        if (i>=n) break;
        R >>= 16;
      }
    }
  }
  else if (bound <= (uint64_t)UINT32_MAX) {
    uint32_t r, b = (uint32_t)bound, M = (uint32_t)0-b, thresh = M % b;
    while (i < n) {
      R = rng->next(rng);
      r = (uint32_t)R;
      if (r >= thresh) x[i++] = (uint64_t)(r % b);
      if (i>=n) break;
      R >>= 32;
      r = (uint32_t)R;
      if (r >= thresh) x[i++] = (uint64_t)(r % b);
    }
  }
  else {
    uint64_t M = 0-bound, thresh = M % bound;
    while (i < n) {
      R = rng->next(rng);
      if (R >= thresh) x[i++] = R % bound;
    }
  }
}

static void rand_uint32(uint32_t bound, uint32_t x[], int n, randompack_rng *rng) {
  // Sets x to n random integers in {0, 1,..., bound-1}
  if (bound == 0) return;
  int i = 0, j;
  uint64_t R;
  if (bound <= (uint32_t)UINT8_MAX) {
    uint8_t r, b = (uint8_t)bound, thresh = (1u<<8) % b;
    while (i < n) {
      R = rng->next(rng);
      for (j = 0; j<8; j++) {
        r = (uint8_t)R;
        if (r >= thresh) x[i++] = (uint32_t)(r % b);
        if (i>=n) break;
        R >>= 8;
      }
    }
  }
  else if (bound <= (uint32_t)UINT16_MAX) {
    uint16_t r, b = (uint16_t)bound, thresh = (1u<<16) % b;
    while (i < n) {
      R = rng->next(rng);
      for (j = 0; j<4; j++) {
        r = (uint16_t)R;
        if (r >= thresh) x[i++] = (uint32_t)(r % b);
        if (i>=n) break;
        R >>= 16;
      }
    }
  }
  else {
    uint32_t r, b = (uint32_t)bound, M = 0-b, thresh = M % b;
    while (i < n) {
      R = rng->next(rng);
      r = (uint32_t)R;
      if (r >= thresh) x[i++] = (uint32_t)(r % b);
      if (i>=n) break;
      R >>= 32;
      r = (uint32_t)R;
      if (r >= thresh) x[i++] = (uint32_t)(r % b);
    }
  }
}

static void PM_rand_int(int bound, int x[], int n, randompack_rng *rng) {
  if (bound <= 0 || bound >= mersenne8) return;
  int thresh = (mersenne8 - 1) % bound;
  for (int i = 0; i < n; i++) {
    int r;
    do {
      r = PM_rand_bits(rng) - 1; // r in [0, 2^31 - 3]
    } while (r < thresh);
    x[i] = r % bound;
  }
}

static void rand_int(int bound, int x[], int n, randompack_rng *rng) {
  // Sets x to integers in [0, bound), where 1 <= bound <= INT32_MAX.
  if (bound <= 0 || bound > INT32_MAX) return; // also checked by caller
  if (rng->engine == PARKMILLER)
    PM_rand_int(bound, x, n, rng);
  else
    rand_uint32((uint32_t) bound, (uint32_t*) x, n, rng);
}

static void rand_dble(double x[], int n, randompack_rng *rng) {
  int i;
  // Random uniform double
  if (rng->engine == PARKMILLER) {
    const double maxrand = 1.0 + (double)mersenne8;
    for (i = 0; i < n; i++) {
      uint32_t u = PM_rand_bits(rng);
      double r = (double)u;
      x[i] = r / maxrand;
    }
  }
  else {
    for (i = 0; i < n; i++) {
      uint64_t v = rng->next(rng);
      x[i] = (double)(v >> 11) * 0x1.0p-53;
    }
  }
}

static void rand_perm(int p[], int n, randompack_rng *rng) {
  // Fisher-Yates algorithm, stores a random permutation of 0..n-1 in p
  for (int i=0; i<n; i++) {
    int j;
    rand_int(i+1, &j, 1, rng);
    if (j != i) p[i] = p[j];
    p[j] = i;
  }
}

static void rand_sample(int p[], int n, int k, randompack_rng *rng) {
  // Select k values from 0..n-1, c.f. reservoir sampling on Wikipedia
  int r;
  p[0] = 0;
  for (int i=0; i<k; i++) {
    rand_int(i+1, &r, 1, rng);
    if (r != i) p[i] = p[r];
    p[r] = i;
  }
  for (int i=k; i<n; i++) {
    rand_int(i+1, &r, 1, rng);
    if (r < k) p[r] = i;
  }
}

static void rand_normal_polar(double x[], int n, randompack_rng *rng) {
  // Normal random numbers from N(0,1); uses polar method.
  double u=0.0, v=0.0, uv[2], s=0.0, R;
  if (n<=0) return;
  int k = 0;
  if (rng->spare_norm == rng->spare_norm) {
    x[k++] = rng->spare_norm;
    rng->spare_norm = NAN;
    if (k >= n) return;
  }
  while (k + 1 < n) {
    do {
      rand_dble(uv, 2, rng);
      u = uv[0]*2.0 - 1.0;
      v = uv[1]*2.0 - 1.0;
      s = u*u + v*v;
    } while (s <= 0.0 || s >= 1.0);
    R = sqrt(-2.0*log(s)/s);
    x[k++] = u*R;
    x[k++] = v*R;
  }
  if (k < n) {
    do {
      rand_dble(uv, 2, rng);
      u = uv[0]*2.0 - 1.0;
      v = uv[1]*2.0 - 1.0;
      s = u*u + v*v;
    } while (s <= 0.0 || s >= 1.0);
    R = sqrt(-2.0*log(s)/s);
    x[k] = u*R;
    rng->spare_norm = v*R;
  }
}

static bool rand_mvn(char transp, double mu[], double Sig[], int d, int n,
                     double X[], int ldX, double L[], randompack_rng *rng) {
  int i, info, rank;
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
      for (i = 0; i < d; i++) // Set L to [A(piv, 1:rank) 0]
        copy(rank, A + piv[i], d, L + i, d);
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
      for (i = 0; i < n; i++)
        rand_normal(X + i*ldX, d, rng);
      trmm("Left", "Lower", "NoT", "NotUdia", d, n, 1.0, L, d, X, ldX);
    }
    else {
      for (i = 0; i < d; i++)
        rand_normal(X + i*ldX, n, rng);
      trmm("Right", "Lower", "T", "NotUdia", n, d, 1.0, L, d, X, ldX);
    }
  }
  else { // Singular Sig, use workspace for the N(0,1) randoms
    if (!ALLOC(Wrk, n*rank)) goto fail;
    rand_normal(Wrk, n*rank, rng);
    if (TRAN)
      gemm("NoT", "NoT", d, n, rank, 1.0, L, d, Wrk, rank, 0.0, X, ldX);
    else
      gemm("NoT", "T", n, d, rank, 1.0, Wrk, n, L, d, 0.0, X, ldX);
    FREE(Wrk);
  }
  if (mu) {
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
