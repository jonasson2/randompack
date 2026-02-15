randompack_inc = [inc] + fortran_inc

randompack_src = files(
  'randompack.c',
  'printX.c',
)
randompack_src += fortran_src

blas_objs = []
if builtin_blas
  fc = meson.get_compiler('fortran')
  blas_f_args = fc.get_supported_arguments([
    '-Wno-compare-reals',
    '-Wno-unused-dummy-argument',
    '-Wno-obsolescent',
    '-Wno-maybe-uninitialized',
    '-Wno-surprising',
  ])
  blas_lib = static_library(
    'randompack_blas',
    files('blas.f'),
    fortran_args: blas_f_args,
    pic: true,
    install: false,
  )
  blas_objs = [blas_lib.extract_all_objects()]
endif

u01_hires = get_option('u01_hires')
u01_hires_args = u01_hires ? ['-DU01_HIRES'] : []

build_avx2_define = build_avx2 ? ['-DBUILD_AVX2'] : []
base_c_args = u01_hires_args + accel_bug_args + build_avx2_define

avx2_objs = []
if build_avx2
  avx2_lib = static_library(
    'randompack_avx2',
    files('avx2.c'),
    include_directories: randompack_inc,
    c_args: avx2_c_args + build_avx2_define,
    pic: true,
    install: false,
  )
  avx2_objs = [avx2_lib.extract_all_objects()]
endif

extra_objs = avx2_objs + blas_objs

if accel_bug_workaround
  randompack_src += files(
    'lapack_dpstf2.f',
    'lapack_dpstrf.f')
endif

librandompack_static = static_library(
  'randompack',
  randompack_src,
  objects: extra_objs,
  include_directories: randompack_inc,
  dependencies: [blas_dep] + math_deps,
  c_args: base_c_args,
  pic: true,
  install: true,
)

randompack_lto = fortran_interface ? ['b_lto=false'] : ['b_lto=true']

librandompack = shared_library(
  'randompack',
  randompack_src,
  objects: extra_objs,
  include_directories: randompack_inc,
  dependencies: [blas_dep] + math_deps,
  c_args: base_c_args,
  override_options: randompack_lto,
  install: true,
)

install_headers(
  'randompack.h',
  subdir: 'randompack',
)

randompack_dep = declare_dependency(
  link_with: librandompack,
  include_directories: randompack_inc,
  dependencies: [blas_dep] + math_deps,
  compile_args: base_c_args,
)

if get_option('build_test_variants')
  librandompack_no128 = static_library(
    # Pretend HAVE128 is disabled
    'randompack_no128',
    randompack_src,
    objects: extra_objs,
    include_directories: randompack_inc,
    dependencies: [blas_dep] + math_deps,
    c_args: ['-DRANDOMPACK_PRETEND_NO128'] + base_c_args,
    pic: true,
    install: false,
  )

  randompack_no128_dep = declare_dependency(
    link_with: librandompack_no128,
    include_directories: randompack_inc,
    dependencies: [blas_dep] + math_deps,
    compile_args: base_c_args,
  )

  librandompack_nosimd = static_library(
    # Pretend SIMD is disabled
    'randompack_nosimd',
    randompack_src,
    objects: blas_objs,
    include_directories: randompack_inc,
    dependencies: [blas_dep] + math_deps,
    c_args: ['-DPRETEND_NOSIMD'] + u01_hires_args + accel_bug_args,
    pic: true,
    install: false,
  )

  randompack_nosimd_dep = declare_dependency(
    link_with: librandompack_nosimd,
    include_directories: randompack_inc,
    dependencies: [blas_dep] + math_deps,
    compile_args: ['-DPRETEND_NOSIMD'] + u01_hires_args + accel_bug_args,
  )
endif
