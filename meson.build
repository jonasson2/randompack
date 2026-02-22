project('randompack', 'c',
  version : '0.1.0',
  default_options : [
    'c_std=c11',
    'warning_level=3',
    'buildtype=debug',
    'default_library=both',
  ]
)

pkg = import('pkgconfig')

buildtype = get_option('buildtype')
host_system = host_machine.system()

# -------------------------------------------------
# BLAS selection
# -------------------------------------------------

blas_option = get_option('blas')

if buildtype.startswith('debug')
  add_project_arguments('-DDEBUG_PRINT', language : 'c')
endif

cc = meson.get_compiler('c')

perf_opt = get_option('perf')
force_no128 = get_option('force_no128')
force_nosimd = get_option('force_nosimd')
if perf_opt
  add_project_arguments('-DRANDOMPACK_PERF', language : 'c')
  add_project_arguments(
    cc.get_supported_arguments([
      '-fno-math-errno',
      '-fno-trapping-math',
      '-fomit-frame-pointer',
      '-fno-semantic-interposition',
    ]),
    language : 'c'
  )
endif

if force_nosimd
  add_project_arguments('-DPRETEND_NOSIMD', language : 'c')
endif

if force_no128
  add_project_arguments('-DRANDOMPACK_PRETEND_NO128', language : 'c')
endif

build_avx2 = false
avx2_c_args = []

if host_machine.cpu_family() in ['x86', 'x86_64']
  if cc.get_id() in ['gcc', 'clang', 'appleclang', 'msvc']
    build_avx2 = true
  endif
endif

build_avx2 = build_avx2 and not force_nosimd

if build_avx2
  if cc.get_id() == 'msvc'
    avx2_c_args = ['/arch:AVX2']
  else
    avx2_c_args = cc.get_supported_arguments(['-mavx2', '-mfma'])
  endif
endif

math_dep = cc.find_library('m', required: false)
math_deps = math_dep.found() ? [math_dep] : []

blas_dep = disabler()
blas_found = 'none'
using_builtin_blas = false
blas_backends = []
blas_explicit = (blas_option != 'builtin' and blas_option != 'auto')

if blas_option == 'builtin'
  using_builtin_blas = true
elif blas_option == 'auto'
  blas_backends = ['accelerate', 'mkl', 'openblas']
else
  blas_backends = [blas_option]
endif

foreach blas_name : blas_backends
  if blas_name == 'accelerate'
    if host_system == 'darwin'
      blas_dep = dependency('appleframeworks', modules: 'Accelerate', required: false)
      if blas_dep.found()
        blas_found = 'Accelerate'
      endif
    endif
  elif blas_name == 'mkl'
    foreach mkl_variant : ['mkl-dynamic-lp64-iomp', 'mkl-static-lp64-iomp', 'mkl']
      blas_dep = dependency(mkl_variant, required: false)
      if blas_dep.found()
        break
      endif
    endforeach
    if not blas_dep.found()
      blas_dep = cc.find_library('mkl_rt', required: false)
    endif
    if blas_dep.found()
      blas_found = 'MKL'
    endif
  elif blas_name == 'openblas'
    blas_dep = dependency('openblas', required: false)
    if not blas_dep.found()
      blas_dep = cc.find_library('openblas', required: false)
    endif
    if blas_dep.found()
      blas_found = 'OpenBLAS'
    endif
  else
    error('Unknown blas option: ' + blas_name)
  endif

  if blas_dep.found()
    break
  endif
endforeach

if blas_explicit and not blas_dep.found()
  error('Requested system BLAS not found: ' + blas_option)
endif

if not blas_dep.found() and not using_builtin_blas
  using_builtin_blas = true
endif

blas_deps = blas_dep.found() ? [blas_dep] : []
if using_builtin_blas
  message('Using built-in BLAS from src/blas.f')
else
  message('Using BLAS library: ' + blas_found)
endif

# -------------------------------------------------
# Accelerate workaround → requires Fortran
# -------------------------------------------------

using_accelerate = (host_system == 'darwin') and (blas_found == 'Accelerate')
local_dpstrf = using_accelerate or using_builtin_blas
local_dpstrf_args = local_dpstrf ? ['-DLOCAL_DPSTRF'] : []
accel_vv = using_accelerate and (not using_builtin_blas)
accel_vv_args = accel_vv ? ['-DUSE_ACCEL_VV'] : []

# -------------------------------------------------
# Optional Fortran support
# -------------------------------------------------

need_fortran = local_dpstrf
has_fortran = add_languages('fortran', required: need_fortran, native: false)
build_fortran = has_fortran

if has_fortran
  fc = meson.get_compiler('fortran')
  if fc.get_id() == 'gcc'
    add_project_arguments('-fno-backtrace', language: 'fortran')
  endif
  add_project_arguments('-Wno-do-subscript', language : 'fortran')
  add_project_arguments(
    fc.get_supported_arguments(['-Wno-function-elimination']),
    language : 'fortran'
  )
endif

if has_fortran and not need_fortran
  kind_check = fc.run('program rp_kind_check\n' +
                      '  use, intrinsic :: iso_c_binding\n' +
                      '  implicit none\n' +
                      '  if (kind(1.0) /= c_float) stop 1\n' +
                      '  if (kind(1.0d0) /= c_double) stop 2\n' +
                      '  if (kind(1) /= c_int) stop 3\n' +
                      'end program\n',
                      name: 'fortran kind check'
                     )

  build_fortran = kind_check.compiled() and kind_check.returncode() == 0
  if not build_fortran
    warning('Fortran kinds do not match C kinds; disabling Fortran interface')
  endif
endif
fortran_interface = false
fortran_src = []
fortran_inc = []

# -------------------------------------------------
# Includes / subdirectories
# -------------------------------------------------

inc = include_directories('src', 'common')
common_util = files(
  'common/Util.c',
  'common/getopt.c',
)

if build_fortran
  subdir('fortran')
endif

subdir('src')

pkg.generate(
  librandompack,
  name: 'randompack',
  description: 'High-performance, reproducible random number generation library',
  version: meson.project_version(),
  subdirs: 'randompack',
  requires_private: [],
)

if get_option('build_examples')
  subdir('examples')
endif

if get_option('build_tests')
  subdir('tests')
endif
