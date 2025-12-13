project('randompack', 'c', 'fortran', 'cpp',
  version : '1.0',
  default_options : [
    'c_std=c11',
    'cpp_std=c++11',
    'warning_level=3',
    'buildtype=debug',
  ]
)

add_project_arguments('-Wno-do-subscript', language : 'fortran')

buildtype = get_option('buildtype')
host_system = host_machine.system()

# Get user-specified BLAS order or use platform defaults
blas_order_option = get_option('blas_order')

if blas_order_option == 'auto'
  if host_system == 'windows'
    blas_search_order = ['mkl', 'openblas']
  elif host_system == 'darwin'
    blas_search_order = ['accelerate', 'openblas', 'mkl']
  else  # Linux and other Unix-like
    blas_search_order = ['openblas', 'mkl']
  endif
else
  blas_search_order = blas_order_option.split(',')
  message('Using user-specified BLAS search order: ' + ', '.join(blas_search_order))
endif

if buildtype.startswith('debug')
  add_project_arguments('-DDEBUG_PRINT', language : 'c')
endif

# Common detection function for all platforms
cc = meson.get_compiler('c')
blas_dep = disabler()
blas_found = 'none'
foreach blas_name : blas_search_order
  if blas_name == 'mkl'
    mkl_names = ['mkl-dynamic-lp64-iomp', 'mkl-static-lp64-iomp', 'mkl']
    foreach mkl_variant : mkl_names
      blas_dep = dependency(mkl_variant, required: false)
      if blas_dep.found()
        break
      endif
    endforeach
    if not blas_dep.found()  # Try finding MKL directly
      blas_dep = cc.find_library('mkl_rt', required: false)
    endif
    if blas_dep.found()
      blas_found = 'MKL'
    endif
    
  elif blas_name == 'accelerate'
    if host_system == 'darwin'
      blas_dep = dependency('appleframeworks', modules: 'Accelerate', required: true)
      blas_found = 'Accelerate'
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

if blas_dep.found()
  message('Using BLAS library: ' + blas_found)
else
  error('No optimized BLAS found, reference implementation not yet available')
endif

# if blas_option == 'none'
#   blas_dep = declare_dependency()  # no BLAS, for symbol inspection

# Your own headers live in src/ for now
inc = include_directories('src')

subdir('src')
subdir('tests')
subdir('examples')
