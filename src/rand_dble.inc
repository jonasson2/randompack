// -*- C -*-

#if HAVE_NEON
#include <arm_neon.h>
#elif HAVE_AVX2
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
#endif

ALWAYS_INLINE
void rand_dble(double x[], size_t len, randompack_rng *rng) {

#if HAVE_NEON  
  const float64x2_t scale = vdupq_n_f64(0x1.0p-53);
#elif HAVE_AVX2
  // const __m256i expo = _mm256_set1_epi64x((long long)0x3ff0000000000000ULL);
  // const __m256d one = _mm256_set1_pd(1.0);
#endif
    
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

#if HAVE_NEON
  // 2 SIMD lanes and 2 ILP chains (via unrolling)
    for (j = 0; j < takemany; j += 4) {
      uint64x2_t u0 = vld1q_u64((const uint64_t *)(rng->buf.u64 + w + j));
      uint64x2_t u1 = vld1q_u64((const uint64_t *)(rng->buf.u64 + w + j + 2));
      u0 = vshrq_n_u64(u0, 11);
      u1 = vshrq_n_u64(u1, 11);
      float64x2_t d0 = vmulq_f64(vcvtq_f64_u64(u0), scale);
      float64x2_t d1 = vmulq_f64(vcvtq_f64_u64(u1), scale);
      vst1q_f64(x + i + j, d0);
      vst1q_f64(x + i + j + 2, d1);
		
#elif HAVE_AVX2
    for (j = 0; j < takemany; j += 4) {
      // const uint64_t *p = rng->buf.u64 + w + j;
      // __m256i r = _mm256_loadu_si256((const __m256i *)p);
      // r = _mm256_srli_epi64(r, 12);
      // r = _mm256_or_si256(r, expo);
      // __m256d u = _mm256_castsi256_pd(r);
      // u = _mm256_sub_pd(u, one);
      // _mm256_storeu_pd(x + i + j, u);
      const uint64_t *p = rng->buf.u64 + w + j;
      u01_from_u64x4_53_avx2(x + i + j, p);
#else
    for (j = 0; j < takemany; j += 4) {
      uint64_t v0 = rng->buf.u64[w + j];
      uint64_t v1 = rng->buf.u64[w + j + 1];
      uint64_t v2 = rng->buf.u64[w + j + 2];
      uint64_t v3 = rng->buf.u64[w + j + 3];
      // uint64_t v4 = rng->buf.u64[(size_t)w + j + 4];
      // uint64_t v5 = rng->buf.u64[(size_t)w + j + 5];
      x[i + j] = U64_TO_DOUBLE(v0);
      x[i + j + 1] = U64_TO_DOUBLE(v1);
      x[i + j + 2] = U64_TO_DOUBLE(v2);
      x[i + j + 3] = U64_TO_DOUBLE(v3);
      // x[i + j + 4] = U64_TO_DOUBLE(v4);
      // x[i + j + 5] = U64_TO_DOUBLE(v5);
#endif
    }
    for (; j < take; j++) {
      uint64_t v = rng->buf.u64[(size_t)w + j];
      x[i + j] = U64_TO_DOUBLE(v);
    }
	 // if (is_unif) {
	 // 	for (j = 0; j < take; j++) {
	 // 	  double v = a + w*x[i + j];
	 // 	  x[i + j] = v < b ? v : b;
	 // 	}
	 // }
    w += take;
    i += take;
  }
  exit_u64_mode(rng, w);
}
