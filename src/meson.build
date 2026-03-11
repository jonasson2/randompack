randompack_inc = [inc] + fortran_inc

randompack_src = files(
  'randompack.c',
  'printX.c',
)
randompack_src += fortran_src

blas_objs = []
if using_builtin_blas
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
  blas_objs = [blas_lib.extract_all_objects(recursive: false)]
endif

build_avx2_define = build_avx2 ? ['-DBUILD_AVX2'] : []
test_hooks_define = get_option('build_tests') ? ['-DRANDOMPACK_TEST_HOOKS'] : []
base_c_args = local_dpstrf_args + accel_vv_args + build_avx2_define
test_buf_define = '-DBUFSIZE=' + test_bufsize.to_string()
buf8_c_args = base_c_args + [test_buf_define]

avx2_objs = []
avx2_objs_buf8 = []
if build_avx2
  avx2_lib = static_library(
    'randompack_avx2',
    files(
      'avx2.c',
      'inplace_logexp.c',
      'sleef.c',
    ),
    include_directories: randompack_inc,
    c_args: avx2_c_args + build_avx2_define + test_hooks_define,
    pic: true,
    install: false,
  )
  avx2_objs = [avx2_lib.extract_all_objects(recursive: false)]

  avx2_lib_buf8 = static_library(
    'randompack_avx2_buf8',
    files(
      'avx2.c',
      'inplace_logexp.c',
      'sleef.c',
    ),
    include_directories: randompack_inc,
    c_args: avx2_c_args + build_avx2_define + test_hooks_define + [test_buf_define],
    pic: true,
    install: false,
  )
  avx2_objs_buf8 = [avx2_lib_buf8.extract_all_objects(recursive: false)]
endif

extra_objs = avx2_objs + blas_objs
extra_objs_buf8 = avx2_objs_buf8 + blas_objs

if local_dpstrf
  randompack_src += files('rp_dpstrf.c')
endif

librandompack_static = static_library(
  'randompack',
  randompack_src,
  objects: extra_objs,
  include_directories: randompack_inc,
  dependencies: c_deps,
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
  dependencies: c_deps,
  c_args: base_c_args,
  #override_options: randompack_lto,
  install: true,
)

install_headers(
  'randompack.h',
  subdir: 'randompack',
)

randompack_dep = declare_dependency(
  link_with: librandompack,
  include_directories: randompack_inc,
  dependencies: c_deps,
  compile_args: base_c_args,
)

librandompack_buf8 = static_library(
  'randompack_buf8',
  randompack_src,
  objects: extra_objs_buf8,
  include_directories: randompack_inc,
  dependencies: c_deps,
  c_args: buf8_c_args,
  pic: true,
  install: false,
)

randompack_buf8_dep = declare_dependency(
  link_with: librandompack_buf8,
  include_directories: randompack_inc,
  dependencies: c_deps,
  compile_args: buf8_c_args,
)
