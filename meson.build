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

blas_list_option = get_option('blas_list')
blas_search_order = []

if blas_list_option == 'none'
  blas_search_order = []
elif blas_list_option == 'auto'
  if host_system == 'windows'
    blas_search_order = ['mkl', 'openblas']
  elif host_system == 'darwin'
    blas_search_order = ['accelerate', 'openblas', 'mkl']
  else
    blas_search_order = ['openblas', 'mkl']
  endif
else
  blas_search_order = blas_list_option.split(',')
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

blas_deps = blas_dep.found() ? [blas_dep] : []

using_builtin_blas = (blas_list_option == 'none') or (not blas_dep.found())
if using_builtin_blas
  message('Using built-in BLAS from src/blas.f')
endif
if blas_dep.found()
  message('Using BLAS library: ' + blas_found)
elif not using_builtin_blas
  error('No optimized BLAS found, reference implementation not yet available')
endif

# -------------------------------------------------
# Accelerate workaround → requires Fortran
# -------------------------------------------------

accel_bug_workaround = (blas_found == 'Accelerate')
local_dpstrf = accel_bug_workaround or using_builtin_blas
local_dpstrf_args = local_dpstrf ? ['-DLOCAL_DPSTRF'] : []
accel_vv = (host_system == 'darwin') and (blas_found == 'Accelerate') and (not using_builtin_blas)
accel_vv_args = accel_vv ? ['-DUSE_ACCEL_VV'] : []

# -------------------------------------------------
# Optional Fortran support
# -------------------------------------------------

force_fortran = accel_bug_workaround or using_builtin_blas
want_fortran = force_fortran
has_fortran = false

if not force_fortran
  has_fortran = add_languages('fortran', required: false)
  if has_fortran
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
  has_fortran = true
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
