// -*- C -*-

#include <immintrin.h>
static inline void u01_from_u64x4_53_avx2(double out[4], const uint64_t in[4]) {
  const __m256i mask32 = _mm256_set1_epi64x(0xffffffffLL);
  const __m256i idx = _mm256_setr_epi32(0, 2, 4, 6, 0, 0, 0, 0);
  const __m256d two32 = _mm256_set1_pd(4294967296.0);        // 2^32
  const __m256d neg_two32 = _mm256_set1_pd(-4294967296.0);   // -2^32
  const __m256d scale = _mm256_set1_pd(0x1.0p-53);           // 2^-53
  __m256i r = _mm256_loadu_si256((const __m256i *)in);
  r = _mm256_srli_epi64(r, 11);
  __m256i lo = _mm256_and_si256(r, mask32);
  __m256i hi = _mm256_srli_epi64(r, 32);
  __m128i lo32 = _mm256_castsi256_si128(_mm256_permutevar8x32_epi32(lo, idx));
  __m128i hi32 = _mm256_castsi256_si128(_mm256_permutevar8x32_epi32(hi, idx));
  __m128i sign = _mm_srai_epi32(lo32, 31);
  __m256d dlo = _mm256_cvtepi32_pd(lo32);
  __m256d dsign = _mm256_cvtepi32_pd(sign);
  dlo = _mm256_add_pd(dlo, _mm256_mul_pd(dsign, neg_two32));
  // add 2^32 where lo32 was "negative"
  __m256d dhi = _mm256_cvtepi32_pd(hi32);
  __m256d dk = _mm256_add_pd(_mm256_mul_pd(dhi, two32), dlo);
  __m256d u = _mm256_mul_pd(dk, scale);
  _mm256_storeu_pd(out, u);
}

ALWAYS_INLINE
void rand_dble(double x[], size_t len, randompack_rng *rng) {

  int w = enter_u64_mode(rng);
  size_t i = 0;
  // bool is_unif = a != 1 || b != 1;
  while (i < len) {
    fill_if_empty(rng, &w);
    size_t remain = len - i;
    size_t avail = BUFSIZE - w;
    size_t take = remain < avail ? remain : avail;
    size_t j;
    size_t takemany = (take/4)*4;
    for (j = 0; j < takemany; j += 4) {
      const uint64_t *p = rng->buf.u64 + w + j;
      u01_from_u64x4_53_avx2(x + i + j, p);
    }
    for (; j < take; j++) {
      uint64_t v = rng->buf.u64[(size_t)w + j];
      x[i + j] = U64_TO_DOUBLE(v);
    }
    w += take;
    i += take;
  }
  exit_u64_mode(rng, w);
}
