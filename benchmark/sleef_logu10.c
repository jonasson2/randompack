// -*- C -*-
// Benchmark-local SLEEF AVX2 log(1 ulp) implementation.

#if !defined(__x86_64__) && !defined(_M_X64)
// No AVX2 on this architecture.
#else

#include <immintrin.h>
#include <math.h>

#if defined(_MSC_VER)
#define STATINLINE static __forceinline
#define CONST
#define SLEEF_INF HUGE_VAL
#define SLEEF_NAN NAN
#elif defined(__GNUC__) || defined(__clang__)
#define STATINLINE static inline __attribute__((always_inline))
#define CONST __attribute__((const))
#define SLEEF_INF __builtin_inf()
#define SLEEF_NAN __builtin_nan("")
#else
#define STATINLINE static inline
#define CONST
#define SLEEF_INF HUGE_VAL
#define SLEEF_NAN NAN
#endif

typedef __m256i vmask;
typedef __m256i vopmask;
typedef __m256d vdouble;
typedef __m128i vint;
typedef struct {
  vdouble x, y;
} vdouble2;

STATINLINE vdouble vcast_vd_d(double d) { return _mm256_set1_pd(d); }
STATINLINE vmask vreinterpret_vm_vd(vdouble vd) { return _mm256_castpd_si256(vd); }
STATINLINE vdouble vreinterpret_vd_vm(vmask vm) { return _mm256_castsi256_pd(vm); }
STATINLINE vint vcast_vi_i(int i) { return _mm_set1_epi32(i); }
STATINLINE vint vcastu_vi_vm(vmask vi) {
  return _mm_or_si128(_mm_castps_si128(_mm_shuffle_ps(
      _mm_castsi128_ps(_mm256_castsi256_si128(vi)), _mm_set1_ps(0), 0x0d)),
      _mm_castps_si128(_mm_shuffle_ps(_mm_set1_ps(0),
      _mm_castsi128_ps(_mm256_extractf128_si256(vi, 1)), 0xd0)));
}
STATINLINE vmask vcastu_vm_vi(vint vi) {
  return _mm256_slli_epi64(_mm256_cvtepi32_epi64(vi), 32);
}
STATINLINE vdouble vcast_vd_vi(vint vi) { return _mm256_cvtepi32_pd(vi); }
STATINLINE vint vadd_vi_vi_vi(vint x, vint y) { return _mm_add_epi32(x, y); }
STATINLINE vint vsub_vi_vi_vi(vint x, vint y) { return _mm_sub_epi32(x, y); }
STATINLINE vint vneg_vi_vi(vint e) { return vsub_vi_vi_vi(vcast_vi_i(0), e); }
STATINLINE vint vand_vi_vi_vi(vint x, vint y) { return _mm_and_si128(x, y); }
STATINLINE vint vsll_vi_vi_i(vint x, int c) { return _mm_slli_epi32(x, c); }
STATINLINE vint vsrl_vi_vi_i(vint x, int c) { return _mm_srli_epi32(x, c); }
STATINLINE vmask vadd64_vm_vm_vm(vmask x, vmask y) { return _mm256_add_epi64(x, y); }
STATINLINE vdouble vmul_vd_vd_vd(vdouble x, vdouble y) { return _mm256_mul_pd(x, y); }
STATINLINE vdouble vadd_vd_vd_vd(vdouble x, vdouble y) { return _mm256_add_pd(x, y); }
STATINLINE vdouble vsub_vd_vd_vd(vdouble x, vdouble y) { return _mm256_sub_pd(x, y); }
STATINLINE vdouble vdiv_vd_vd_vd(vdouble x, vdouble y) { return _mm256_div_pd(x, y); }
STATINLINE vdouble vrec_vd_vd(vdouble x) { return _mm256_div_pd(_mm256_set1_pd(1), x); }
STATINLINE vdouble vmla_vd_vd_vd_vd(vdouble x, vdouble y, vdouble z) {
  return _mm256_fmadd_pd(x, y, z);
}
STATINLINE vdouble vfma_vd_vd_vd_vd(vdouble x, vdouble y, vdouble z) {
  return _mm256_fmadd_pd(x, y, z);
}
STATINLINE vdouble vfmapn_vd_vd_vd_vd(vdouble x, vdouble y, vdouble z) {
  return _mm256_fmsub_pd(x, y, z);
}
STATINLINE vdouble vfmanp_vd_vd_vd_vd(vdouble x, vdouble y, vdouble z) {
  return _mm256_fnmadd_pd(x, y, z);
}
STATINLINE vopmask vlt_vo_vd_vd(vdouble x, vdouble y) {
  return vreinterpret_vm_vd(_mm256_cmp_pd(x, y, 0x11));
}
STATINLINE vopmask veq_vo_vd_vd(vdouble x, vdouble y) {
  return vreinterpret_vm_vd(_mm256_cmp_pd(x, y, 0x00));
}
STATINLINE vopmask visnan_vo_vd(vdouble d) {
  return vreinterpret_vm_vd(_mm256_cmp_pd(d, d, 0x04));
}
STATINLINE vopmask vispinf_vo_vd(vdouble d) {
  return vreinterpret_vm_vd(_mm256_cmp_pd(d, _mm256_set1_pd(SLEEF_INF),
      0x00));
}
STATINLINE vopmask vor_vo_vo_vo(vopmask x, vopmask y) {
  return vreinterpret_vm_vd(
      _mm256_or_pd(vreinterpret_vd_vm(x), vreinterpret_vd_vm(y)));
}
STATINLINE vopmask vcast_vo32_vo64(vopmask o) {
  return _mm256_permutevar8x32_epi32(o, _mm256_set_epi32(0, 0, 0, 0,
      6, 4, 2, 0));
}
STATINLINE vdouble vsel_vd_vo_vd_vd(vopmask o, vdouble x, vdouble y) {
  return _mm256_blendv_pd(y, x, _mm256_castsi256_pd(o));
}
STATINLINE vint vsel_vi_vo_vi_vi(vopmask m, vint x, vint y) {
  return _mm_blendv_epi8(y, x, _mm256_castsi256_si128(m));
}
STATINLINE CONST vdouble vd2getx_vd_vd2(vdouble2 v) { return v.x; }
STATINLINE CONST vdouble vd2gety_vd_vd2(vdouble2 v) { return v.y; }
STATINLINE CONST vdouble2 vd2setxy_vd2_vd_vd(vdouble x, vdouble y) {
  vdouble2 v;
  v.x = x;
  v.y = y;
  return v;
}
STATINLINE CONST vdouble2 vcast_vd2_d_d(double h, double l) {
  return vd2setxy_vd2_vd_vd(vcast_vd_d(h), vcast_vd_d(l));
}
STATINLINE CONST vdouble vadd_vd_3vd(vdouble v0, vdouble v1, vdouble v2) {
  return vadd_vd_vd_vd(vadd_vd_vd_vd(v0, v1), v2);
}
STATINLINE CONST vdouble vadd_vd_4vd(vdouble v0, vdouble v1, vdouble v2,
    vdouble v3) {
  return vadd_vd_3vd(vadd_vd_vd_vd(v0, v1), v2, v3);
}
STATINLINE CONST vdouble2 ddscale_vd2_vd2_vd(vdouble2 d, vdouble s) {
  return vd2setxy_vd2_vd_vd(vmul_vd_vd_vd(vd2getx_vd_vd2(d), s),
      vmul_vd_vd_vd(vd2gety_vd_vd2(d), s));
}
STATINLINE CONST vdouble2 ddadd2_vd2_vd_vd(vdouble x, vdouble y) {
  vdouble s = vadd_vd_vd_vd(x, y);
  vdouble v = vsub_vd_vd_vd(s, x);
  return vd2setxy_vd2_vd_vd(s, vadd_vd_vd_vd(vsub_vd_vd_vd(x,
      vsub_vd_vd_vd(s, v)), vsub_vd_vd_vd(y, v)));
}
STATINLINE CONST vdouble2 ddadd_vd2_vd2_vd(vdouble2 x, vdouble y) {
  vdouble s = vadd_vd_vd_vd(vd2getx_vd_vd2(x), y);
  return vd2setxy_vd2_vd_vd(s, vadd_vd_3vd(vsub_vd_vd_vd(vd2getx_vd_vd2(x),
      s), y, vd2gety_vd_vd2(x)));
}
STATINLINE CONST vdouble2 ddadd_vd2_vd2_vd2(vdouble2 x, vdouble2 y) {
  vdouble s = vadd_vd_vd_vd(vd2getx_vd_vd2(x), vd2getx_vd_vd2(y));
  return vd2setxy_vd2_vd_vd(s, vadd_vd_4vd(vsub_vd_vd_vd(vd2getx_vd_vd2(x),
      s), vd2getx_vd_vd2(y), vd2gety_vd_vd2(x), vd2gety_vd_vd2(y)));
}
STATINLINE CONST vdouble2 dddiv_vd2_vd2_vd2(vdouble2 n, vdouble2 d) {
  vdouble t = vrec_vd_vd(vd2getx_vd_vd2(d));
  vdouble s = vmul_vd_vd_vd(vd2getx_vd_vd2(n), t);
  vdouble u = vfmapn_vd_vd_vd_vd(t, vd2getx_vd_vd2(n), s);
  vdouble v = vfmanp_vd_vd_vd_vd(vd2gety_vd_vd2(d), t,
      vfmanp_vd_vd_vd_vd(vd2getx_vd_vd2(d), t, vcast_vd_d(1)));
  return vd2setxy_vd2_vd_vd(s, vfma_vd_vd_vd_vd(s, v,
      vfma_vd_vd_vd_vd(vd2gety_vd_vd2(n), t, u)));
}
STATINLINE CONST vint vilogb2k_vi_vd(vdouble d) {
  vint q = vcastu_vi_vm(vreinterpret_vm_vd(d));
  q = vsrl_vi_vi_i(q, 20);
  q = vand_vi_vi_vi(q, vcast_vi_i(0x7ff));
  q = vsub_vi_vi_vi(q, vcast_vi_i(0x3ff));
  return q;
}
STATINLINE CONST vdouble vldexp3_vd_vd_vi(vdouble d, vint q) {
  return vreinterpret_vd_vm(vadd64_vm_vm_vm(vreinterpret_vm_vd(d),
      vcastu_vm_vi(vsll_vi_vi_i(q, 20))));
}
STATINLINE CONST vdouble2 ddmul_vd2_vd2_vd(vdouble2 x, vdouble y) {
  vdouble s = vmul_vd_vd_vd(vd2getx_vd_vd2(x), y);
  return vd2setxy_vd2_vd_vd(s, vfma_vd_vd_vd_vd(vd2gety_vd_vd2(x), y,
      vfmapn_vd_vd_vd_vd(vd2getx_vd_vd2(x), y, s)));
}

CONST vdouble Sleef_logd4_u10avx2(vdouble d) {
  vdouble2 x;
  vdouble t, m, x2;
  vopmask o = vlt_vo_vd_vd(d, vcast_vd_d(0x1p-1022));
  d = vsel_vd_vo_vd_vd(o, vmul_vd_vd_vd(d,
      vcast_vd_d(4294967296.0 * 4294967296.0)), d);
  vint e = vilogb2k_vi_vd(vmul_vd_vd_vd(d, vcast_vd_d(1.0/0.75)));
  m = vldexp3_vd_vd_vi(d, vneg_vi_vi(e));
  e = vsel_vi_vo_vi_vi(vcast_vo32_vo64(o), vsub_vi_vi_vi(e, vcast_vi_i(64)),
      e);
  x = dddiv_vd2_vd2_vd2(ddadd2_vd2_vd_vd(vcast_vd_d(-1), m),
      ddadd2_vd2_vd_vd(vcast_vd_d(1), m));
  x2 = vmul_vd_vd_vd(vd2getx_vd_vd2(x), vd2getx_vd_vd2(x));
  vdouble x4 = vmul_vd_vd_vd(x2, x2);
  vdouble x8 = vmul_vd_vd_vd(x4, x4);
  t = vmla_vd_vd_vd_vd((x8), (vmla_vd_vd_vd_vd((x4),
      (vcast_vd_d(0.1532076988502701353)), (vmla_vd_vd_vd_vd((x2),
      (vcast_vd_d(0.1525629051003428716)), (vcast_vd_d(0.1818605932937785996)))))),
      (vmla_vd_vd_vd_vd((x4), (vmla_vd_vd_vd_vd((x2),
      (vcast_vd_d(0.2222214519839380009)), (vcast_vd_d(0.2857142932794299317)))),
      (vmla_vd_vd_vd_vd((x2), (vcast_vd_d(0.3999999999635251990)),
      (vcast_vd_d(0.6666666666667333541)))))));
  vdouble2 s = ddmul_vd2_vd2_vd(vcast_vd2_d_d(0.693147180559945286226764,
      2.319046813846299558417771e-17), vcast_vd_vi(e));
  s = ddadd_vd2_vd2_vd2(s, ddscale_vd2_vd2_vd(x, vcast_vd_d(2)));
  s = ddadd_vd2_vd2_vd(s, vmul_vd_vd_vd(vmul_vd_vd_vd(x2, vd2getx_vd_vd2(x)),
      t));
  vdouble r = vadd_vd_vd_vd(vd2getx_vd_vd2(s), vd2gety_vd_vd2(s));
  r = vsel_vd_vo_vd_vd(vispinf_vo_vd(d), vcast_vd_d(SLEEF_INF), r);
  r = vsel_vd_vo_vd_vd(vor_vo_vo_vo(vlt_vo_vd_vd(d, vcast_vd_d(0)),
      visnan_vo_vd(d)), vcast_vd_d(SLEEF_NAN), r);
  r = vsel_vd_vo_vd_vd(veq_vo_vd_vd(d, vcast_vd_d(0)),
      vcast_vd_d(-SLEEF_INF), r);
  return r;
}

#endif
