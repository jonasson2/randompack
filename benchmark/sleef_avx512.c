// -*- C -*-
// Benchmark-local SLEEF AVX-512 double exp/log implementation.

#if !defined(__x86_64__) && !defined(_M_X64)
// No AVX-512 on this architecture.
#else

#include <immintrin.h>

#define STATINLINE static inline __attribute__((always_inline))
#define CONST __attribute__((const))

typedef __m512i vmask;
typedef __mmask8 vopmask;
typedef __m512d vdouble;
typedef __m256i vint;

STATINLINE vdouble vcast_vd_d(double d) { return _mm512_set1_pd(d); }
STATINLINE vmask vreinterpret_vm_vd(vdouble vd) { return _mm512_castpd_si512(vd); }
STATINLINE vdouble vreinterpret_vd_vm(vmask vm) { return _mm512_castsi512_pd(vm); }
STATINLINE vint vcast_vi_i(int i) { return _mm256_set1_epi32(i); }
STATINLINE vmask vcastu_vm_vi(vint vi) {
  return _mm512_slli_epi64(_mm512_cvtepi32_epi64(vi), 32);
}
STATINLINE vint vcastu_vi_vm(vmask vm) {
  return _mm512_cvtepi64_epi32(_mm512_srli_epi64(vm, 32));
}
STATINLINE vdouble vcast_vd_vi(vint vi) { return _mm512_cvtepi32_pd(vi); }
STATINLINE vint vrint_vi_vd(vdouble vd) {
  return _mm512_cvt_roundpd_epi32(vd,
      _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
}
STATINLINE vdouble vrint_vd_vd(vdouble vd) {
  return _mm512_roundscale_pd(vd,
      _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
}
STATINLINE vint vadd_vi_vi_vi(vint x, vint y) { return _mm256_add_epi32(x, y); }
STATINLINE vint vsub_vi_vi_vi(vint x, vint y) { return _mm256_sub_epi32(x, y); }
STATINLINE vint vneg_vi_vi(vint e) { return vsub_vi_vi_vi(vcast_vi_i(0), e); }
STATINLINE vint vand_vi_vi_vi(vint x, vint y) { return _mm256_and_si256(x, y); }
STATINLINE vint vsll_vi_vi_i(vint x, int c) { return _mm256_slli_epi32(x, c); }
STATINLINE vint vsra_vi_vi_i(vint x, int c) { return _mm256_srai_epi32(x, c); }
STATINLINE vint vsrl_vi_vi_i(vint x, int c) { return _mm256_srli_epi32(x, c); }
STATINLINE vmask vadd64_vm_vm_vm(vmask x, vmask y) { return _mm512_add_epi64(x, y); }
STATINLINE vdouble vadd_vd_vd_vd(vdouble x, vdouble y) { return _mm512_add_pd(x, y); }
STATINLINE vdouble vsub_vd_vd_vd(vdouble x, vdouble y) { return _mm512_sub_pd(x, y); }
STATINLINE vdouble vmul_vd_vd_vd(vdouble x, vdouble y) { return _mm512_mul_pd(x, y); }
STATINLINE vdouble vdiv_vd_vd_vd(vdouble x, vdouble y) { return _mm512_div_pd(x, y); }
STATINLINE vdouble vrec_vd_vd(vdouble x) { return _mm512_div_pd(vcast_vd_d(1), x); }
STATINLINE vdouble vmla_vd_vd_vd_vd(vdouble x, vdouble y, vdouble z) {
  return _mm512_fmadd_pd(x, y, z);
}
STATINLINE vdouble vfma_vd_vd_vd_vd(vdouble x, vdouble y, vdouble z) {
  return _mm512_fmadd_pd(x, y, z);
}
STATINLINE vdouble vfmapn_vd_vd_vd_vd(vdouble x, vdouble y, vdouble z) {
  return _mm512_fmsub_pd(x, y, z);
}
STATINLINE vdouble vfmanp_vd_vd_vd_vd(vdouble x, vdouble y, vdouble z) {
  return _mm512_fnmadd_pd(x, y, z);
}
STATINLINE vopmask vlt_vo_vd_vd(vdouble x, vdouble y) {
  return _mm512_cmp_pd_mask(x, y, _CMP_LT_OQ);
}
STATINLINE vopmask vgt_vo_vd_vd(vdouble x, vdouble y) {
  return _mm512_cmp_pd_mask(x, y, _CMP_GT_OQ);
}
STATINLINE vopmask veq_vo_vd_vd(vdouble x, vdouble y) {
  return _mm512_cmp_pd_mask(x, y, _CMP_EQ_OQ);
}
STATINLINE vopmask visnan_vo_vd(vdouble d) {
  return _mm512_cmp_pd_mask(d, d, _CMP_NEQ_UQ);
}
STATINLINE vopmask vispinf_vo_vd(vdouble d) {
  return _mm512_cmp_pd_mask(d, vcast_vd_d(__builtin_inf()), _CMP_EQ_OQ);
}
STATINLINE vopmask vor_vo_vo_vo(vopmask x, vopmask y) { return x | y; }
STATINLINE vdouble vsel_vd_vo_vd_vd(vopmask o, vdouble x, vdouble y) {
  return _mm512_mask_blend_pd(o, y, x);
}
STATINLINE vint vsel_vi_vo_vi_vi(vopmask m, vint x, vint y) {
  return _mm512_castsi512_si256(_mm512_mask_blend_epi32(m,
      _mm512_castsi256_si512(y), _mm512_castsi256_si512(x)));
}
STATINLINE vmask vandnot_vm_vo64_vm(vopmask o, vmask m) {
  return _mm512_mask_blend_epi64(o, m, _mm512_setzero_si512());
}

STATINLINE CONST vdouble vpow2i_vd_vi(vint q) {
  q = vadd_vi_vi_vi(vcast_vi_i(0x3ff), q);
  return vreinterpret_vd_vm(vcastu_vm_vi(vsll_vi_vi_i(q, 20)));
}

STATINLINE CONST vdouble vldexp2_vd_vd_vi(vdouble d, vint e) {
  vint e2 = vsra_vi_vi_i(e, 1);
  return vmul_vd_vd_vd(vmul_vd_vd_vd(d, vpow2i_vd_vi(e2)),
      vpow2i_vd_vi(vsub_vi_vi_vi(e, e2)));
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

CONST vdouble Sleef_expd8_u10avx512f(vdouble d) {
  vdouble u = vrint_vd_vd(vmul_vd_vd_vd(d, vcast_vd_d(
      1.442695040888963407359924681001892137426645954152985934135449406931)));
  vint q = vrint_vi_vd(u);
  vdouble s = vmla_vd_vd_vd_vd(u, vcast_vd_d(
      -.69314718055966295651160180568695068359375), d);
  s = vmla_vd_vd_vd_vd(u, vcast_vd_d(
      -.28235290563031577122588448175013436025525412068e-12), s);
  vdouble s2 = vmul_vd_vd_vd(s, s);
  vdouble s4 = vmul_vd_vd_vd(s2, s2);
  vdouble s8 = vmul_vd_vd_vd(s4, s4);
  u = vmla_vd_vd_vd_vd(s8, vmla_vd_vd_vd_vd(s,
      vcast_vd_d(+0.2081276378237164457e-8),
      vcast_vd_d(+0.2511210703042288022e-7)),
      vmla_vd_vd_vd_vd(s4, vmla_vd_vd_vd_vd(s2,
      vmla_vd_vd_vd_vd(s, vcast_vd_d(+0.2755762628169491192e-6),
      vcast_vd_d(+0.2755723402025388239e-5)),
      vmla_vd_vd_vd_vd(s, vcast_vd_d(+0.2480158687479686264e-4),
      vcast_vd_d(+0.1984126989855865850e-3))),
      vmla_vd_vd_vd_vd(s2, vmla_vd_vd_vd_vd(s,
      vcast_vd_d(+0.1388888888914497797e-2),
      vcast_vd_d(+0.8333333333314938210e-2)),
      vmla_vd_vd_vd_vd(s, vcast_vd_d(+0.4166666666666602598e-1),
      vcast_vd_d(+0.1666666666666669072)))));
  u = vfma_vd_vd_vd_vd(u, s, vcast_vd_d(+0.5));
  u = vfma_vd_vd_vd_vd(u, s, vcast_vd_d(+1));
  u = vfma_vd_vd_vd_vd(u, s, vcast_vd_d(+1));
  u = vldexp2_vd_vd_vi(u, q);
  vopmask o = vgt_vo_vd_vd(d, vcast_vd_d(0x1.62e42fefa39efp+9));
  u = vsel_vd_vo_vd_vd(o, vcast_vd_d(__builtin_inf()), u);
  u = vreinterpret_vd_vm(vandnot_vm_vo64_vm(vlt_vo_vd_vd(d, vcast_vd_d(-1000)),
      vreinterpret_vm_vd(u)));
  return u;
}

CONST vdouble Sleef_logd8_u35avx512f(vdouble d) {
  vopmask o = vlt_vo_vd_vd(d, vcast_vd_d(0x1p-1022));
  d = vsel_vd_vo_vd_vd(o, vmul_vd_vd_vd(d,
      vcast_vd_d((double)(1L << 32)*(double)(1L << 32))), d);
  vint e = vilogb2k_vi_vd(vmul_vd_vd_vd(d, vcast_vd_d(1.0/0.75)));
  vdouble m = vldexp3_vd_vd_vi(d, vneg_vi_vi(e));
  e = vsel_vi_vo_vi_vi(o, vsub_vi_vi_vi(e, vcast_vi_i(64)), e);
  vdouble x = vdiv_vd_vd_vd(vsub_vd_vd_vd(m, vcast_vd_d(1)),
      vadd_vd_vd_vd(vcast_vd_d(1), m));
  vdouble x2 = vmul_vd_vd_vd(x, x);
  vdouble x4 = vmul_vd_vd_vd(x2, x2);
  vdouble x8 = vmul_vd_vd_vd(x4, x4);
  vdouble x3 = vmul_vd_vd_vd(x, x2);
  vdouble t = vmla_vd_vd_vd_vd(x8, vmla_vd_vd_vd_vd(x4,
      vcast_vd_d(0.153487338491425068243146),
      vmla_vd_vd_vd_vd(x2, vcast_vd_d(0.152519917006351951593857),
      vcast_vd_d(0.181863266251982985677316))),
      vmla_vd_vd_vd_vd(x4, vmla_vd_vd_vd_vd(x2,
      vcast_vd_d(0.222221366518767365905163),
      vcast_vd_d(0.285714294746548025383248)),
      vmla_vd_vd_vd_vd(x2, vcast_vd_d(0.399999999950799600689777),
      vcast_vd_d(0.6666666666667778740063))));
  x = vmla_vd_vd_vd_vd(x, vcast_vd_d(2), vmul_vd_vd_vd(
      vcast_vd_d(0.693147180559945286226764), vcast_vd_vi(e)));
  x = vmla_vd_vd_vd_vd(x3, t, x);
  x = vsel_vd_vo_vd_vd(vispinf_vo_vd(d), vcast_vd_d(__builtin_inf()), x);
  x = vsel_vd_vo_vd_vd(vor_vo_vo_vo(vlt_vo_vd_vd(d, vcast_vd_d(0)),
      visnan_vo_vd(d)), vcast_vd_d(__builtin_nan("")), x);
  x = vsel_vd_vo_vd_vd(veq_vo_vd_vd(d, vcast_vd_d(0)),
      vcast_vd_d(-__builtin_inf()), x);
  return x;
}

void sleef_avx512_exp(double out[], const double in[], int n) {
  int i = 0;
  for (; i + 8 <= n; i += 8) {
    __m512d x = _mm512_loadu_pd(in + i);
    __m512d y = Sleef_expd8_u10avx512f(x);
    _mm512_storeu_pd(out + i, y);
  }
  for (; i < n; i++)
    out[i] = __builtin_exp(in[i]);
}

void sleef_avx512_log(double out[], const double in[], int n) {
  int i = 0;
  for (; i + 8 <= n; i += 8) {
    __m512d x = _mm512_loadu_pd(in + i);
    __m512d y = Sleef_logd8_u35avx512f(x);
    _mm512_storeu_pd(out + i, y);
  }
  for (; i < n; i++)
    out[i] = __builtin_log(in[i]);
}

#endif
