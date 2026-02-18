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

builtin_blas = get_option('builtin_blas')
blas_order_option = get_option('blas_order')
blas_search_order = []

if blas_order_option == 'auto'
  if host_system == 'windows'
    blas_search_order = ['mkl', 'openblas']
  elif host_system == 'darwin'
    blas_search_order = ['accelerate', 'openblas', 'mkl']
  else
    blas_search_order = ['openblas', 'mkl']
  endif
else
  blas_search_order = blas_order_option.split(',')
  message('Using user-specified BLAS search order: ' + ', '.join(blas_search_order))
endif

if buildtype.startswith('debug')
  add_project_arguments('-DDEBUG_PRINT', language : 'c')
endif

cc = meson.get_compiler('c')

perf_opt = get_option('perf')
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

build_avx2 = false
avx2_c_args = []

if host_machine.cpu_family() in ['x86', 'x86_64']
  if cc.get_id() in ['gcc', 'clang', 'appleclang', 'msvc']
    build_avx2 = true
  endif
endif

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

foreach blas_name : blas_search_order
  if blas_name == 'mkl'
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

  elif blas_name == 'accelerate'
    if host_system == 'darwin'
      blas_dep = dependency('appleframeworks', modules: 'Accelerate', required: false)
      if blas_dep.found()
        blas_found = 'Accelerate'
      endif
    endif

  elif blas_name == 'openblas'
    blas_dep = dependency('openblas', required: false)
    if not blas_dep.found()
      blas_dep = cc.find_library('openblas', required: false)
    endif
    if blas_dep.found()
      blas_found = 'OpenBLAS'
    endif
  endif

  if blas_dep.found()
    break
  endif
endforeach

if builtin_blas
  message('Using built-in BLAS from src/blas.f')
endif
if blas_dep.found()
  message('Using BLAS library: ' + blas_found)
else
  error('No optimized BLAS found, reference implementation not yet available')
endif

# -------------------------------------------------
# Accelerate workaround → requires Fortran
# -------------------------------------------------

accel_bug_workaround = (blas_found == 'Accelerate')
accel_bug_args = accel_bug_workaround ? ['-DACCEL_BUG_WORKAROUND'] : []

# -------------------------------------------------
# Optional Fortran support
# -------------------------------------------------

force_fortran = accel_bug_workaround or builtin_blas
want_fortran = force_fortran

if not force_fortran
  add_languages('fortran', required: false)
  if meson.has_language('fortran')
    fc = meson.get_compiler('fortran')
    add_project_arguments('-Wno-do-subscript', language : 'fortran')
    add_project_arguments(
      fc.get_supported_arguments(['-Wno-function-elimination']),
      language : 'fortran'
    )

    kind_check = fc.run('program rp_kind_check\n' +
                        '  use, intrinsic :: iso_c_binding\n' +
                        '  implicit none\n' +
                        '  if (kind(1.0) /= c_float) stop 1\n' +
                        '  if (kind(1.0d0) /= c_double) stop 2\n' +
                        '  if (kind(1) /= c_int) stop 3\n' +
                        'end program\n',
                        name: 'fortran kind check'
                       )

    want_fortran = kind_check.compiled() and kind_check.returncode() == 0
    if not want_fortran
      warning('Fortran kinds do not match C kinds; disabling Fortran interface')
    endif
  else
    want_fortran = false
  endif
endif

if force_fortran
  add_languages('fortran', required: true)
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

build_fortran = want_fortran

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
