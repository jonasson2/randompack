project('randompack', 'c',
  meson_version: '>=1.3.0',
  version: '0.1.4',
  default_options: [
    'c_std=c11',
    'cpp_std=c++17',
    'warning_level=3',
    'buildtype=debug',
    'default_library=shared',
  ]
)

pkg = import('pkgconfig')

buildtype = get_option('buildtype')
host_system = host_machine.system()

if host_machine.system() != 'windows'  # Ensure <time.h> exposes clock_gettime/CLOCK_*
  add_project_arguments('-D_POSIX_C_SOURCE=200809L', language: 'c')
  if host_machine.system() == 'darwin'
    add_project_arguments('-D_DARWIN_C_SOURCE', language: 'c')
  endif
endif

install_data('LICENSE', install_dir: join_paths(get_option('datadir'), 'licenses', 'Randompack'))

# -------------------------------------------------
# BLAS selection
# -------------------------------------------------

blas_option = get_option('blas')

if buildtype.startswith('debug')
  add_project_arguments('-DDEBUG_PRINT', language: 'c')
endif

cc = meson.get_compiler('c')
fp_args = []

if cc.get_id() == 'msvc'
  fp_args += cc.get_supported_arguments(['/fp:precise'])
elif cc.get_id() == 'intel-cl'
  fp_args += cc.get_supported_arguments(['/fp:precise'])
elif cc.get_id() in ['clang', 'appleclang']
  fp_args += cc.get_supported_arguments(['-ffp-model=precise'])
  fp_args += cc.get_supported_arguments(['-ffp-contract=off'])
elif cc.get_id() == 'intel-llvm'
  fp_args += cc.get_supported_arguments(['-fp-model=strict'])
else
  fp_args += cc.get_supported_arguments(['-ffp-contract=off'])
endif

if fp_args.length() > 0
  add_project_arguments(fp_args, language: 'c')
endif

if cc.get_argument_syntax() == 'msvc'
  add_project_arguments(
    cc.get_supported_arguments([
      '/D_CRT_SECURE_NO_WARNINGS',
      '/wd4701',
      '/wd4702',
      '/wd4723',
      '/wd4756',
    ]),
    language: 'c'
  )
endif

if cc.get_id() == 'nvidia_hpc'
  add_project_arguments('-Minform=warn', language: 'c')
endif

force_no128 = get_option('force_no128')
force_nosimd = get_option('force_nosimd')

if force_nosimd
  add_project_arguments('-DPRETEND_NOSIMD', language: 'c')
endif

if force_no128
  add_project_arguments('-DRANDOMPACK_PRETEND_NO128', language: 'c')
endif

build_avx2 = false
avx2_c_args = []
build_avx512 = false
avx512_c_args = []

if host_machine.cpu_family() in ['x86', 'x86_64']
  if cc.get_id() in ['gcc', 'clang', 'appleclang', 'msvc', 'intel-llvm']
    build_avx2 = true
    build_avx512 = cc.get_id() != 'msvc'
  endif
endif

build_avx2 = build_avx2 and not force_nosimd
build_avx512 = build_avx512 and build_avx2

if build_avx2
  if cc.get_id() == 'msvc'
    avx2_c_args = ['/arch:AVX2']
  else
    avx2_c_args = cc.get_supported_arguments(['-mavx2', '-mfma'])
  endif
endif

if build_avx512
  avx512_c_args = cc.get_supported_arguments(['-mavx512f', '-mavx512dq'])
  build_avx512 = avx512_c_args.length() == 2
endif

c_deps = []

math_dep = cc.find_library('m', required: false)
c_deps += math_dep.found() ? [math_dep] : []

rt_dep = disabler()
if host_system == 'linux'
  rt_dep = cc.find_library('rt', required: false)
  c_deps += rt_dep.found() ? [rt_dep] : []
endif

if host_system == 'windows'
  advapi32_dep = cc.find_library('advapi32', required: false)
  c_deps += advapi32_dep.found() ? [advapi32_dep] : []
endif

blas_dep = disabler()
blas_found = 'none'
using_builtin_blas = false
blas_backends = []
blas_explicit = (blas_option != 'builtin' and blas_option != 'auto')

if blas_option == 'builtin'
  using_builtin_blas = true
elif blas_option == 'auto' or blas_option == 'system'
  blas_backends = ['accelerate', 'openblas', 'mkl']
else
  blas_backends = [blas_option]
endif

foreach blas_name: blas_backends
  if blas_name == 'accelerate'
    if host_system == 'darwin'
      blas_dep = dependency('appleframeworks', modules: 'Accelerate', required: false)
      if blas_dep.found()
        blas_found = 'Accelerate'
      endif
    endif
  elif blas_name == 'mkl'
    foreach mkl_variant: ['mkl-dynamic-lp64-iomp', 'mkl-static-lp64-iomp', 'mkl']
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
      blas_dep = cc.find_library('openblas',
        dirs: [join_paths(get_option('prefix'), 'lib')],
        required: false)
    endif
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

if (blas_explicit or blas_option == 'system') and not blas_dep.found()
  error('Requested system BLAS not found: ' + blas_option)
endif

if not blas_dep.found() and not using_builtin_blas and blas_option != 'system'
  using_builtin_blas = true
endif

c_deps += blas_dep.found() ? [blas_dep] : []

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
local_dpstrf_args = local_dpstrf ? ['-DLOCAL_DPSTRF']: []
accel_vv = using_accelerate and (not using_builtin_blas)
accel_vv_args = accel_vv ? ['-DUSE_ACCEL_VV']: []

# -------------------------------------------------
# Optional Fortran support
# -------------------------------------------------

build_fortran_requested = get_option('build_fortran_interface')
need_fortran = local_dpstrf or build_fortran_requested
has_fortran = add_languages('fortran', required: need_fortran, native: false)
build_fortran = has_fortran
build_fortran_interface = build_fortran and build_fortran_requested
build_cpp = add_languages('cpp', required: false, native: false)

if build_cpp
  cpp = meson.get_compiler('cpp')
  if cpp.get_argument_syntax() == 'msvc'
    add_project_arguments(
      cpp.get_supported_arguments([
        '/D_CRT_SECURE_NO_WARNINGS',
      ]),
      language: 'cpp'
    )
  endif
  if cpp.get_id() == 'nvidia_hpc'
    add_project_arguments('-Minform=warn', language: 'cpp')
  endif
endif

if has_fortran
  fc = meson.get_compiler('fortran')
  if fc.get_id() == 'nvidia_hpc'
    add_project_arguments('-Minform=warn', language: 'fortran')
  endif
  if fc.get_id() == 'gcc'
    add_project_arguments('-fno-backtrace', language: 'fortran')
  endif
  add_project_arguments(
    fc.get_supported_arguments([
      '-Wno-do-subscript',
      '-Wno-function-elimination',
    ]),
    language: 'fortran'
  )
  if fc.get_id() in ['intel', 'intel-cl', 'intel-llvm', 'intel-llvm-cl']
    add_project_arguments(
      fc.get_supported_arguments(['/Qdiag-disable:5472']),
      language: 'fortran'
    )
    add_project_link_arguments(
      fc.get_supported_link_arguments(['/Qdiag-disable:10047']),
      language: 'fortran'
    )
  endif
endif

if build_fortran and not build_fortran_interface
  message('Fortran interface disabled by build_fortran_interface option')
endif
fortran_interface_src = files('fortran/src/randompack.f90')
fortran_src = []
fortran_inc = []
if build_fortran_interface
  fortran_src = fortran_interface_src
endif

install_data(
  fortran_interface_src,
  install_dir: join_paths(get_option('includedir'), 'randompack'),
)

# -------------------------------------------------
# Includes / subdirectories
# -------------------------------------------------

inc = include_directories('src')

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

if get_option('build_benchmarks')
  subdir('benchmark')
endif

if get_option('build_tests')
  subdir('tests')
endif
