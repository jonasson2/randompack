project('ranlux-work', 'c', 'cpp',
  default_options: [
    'c_std=c11',
    'cpp_std=c++11',
    'warning_level=3',
    'buildtype=release',
  ]
)

cc = meson.get_compiler('c')
if host_machine.system() != 'windows'
  add_project_arguments('-D_POSIX_C_SOURCE=200809L', language: 'c')
  if host_machine.system() == 'darwin'
    add_project_arguments('-D_DARWIN_C_SOURCE', language: 'c')
  endif
endif

randompack_prefix = join_paths(meson.project_source_root(), '../../install')
randompack_dep = declare_dependency(
  compile_args : ['-I' + join_paths(randompack_prefix, 'include/randompack')],
  link_args : [
    join_paths(randompack_prefix, 'lib/librandompack.dylib'),
    '-Wl,-rpath,' + join_paths(randompack_prefix, 'lib'),
  ],
)


executable(
  'compute_ranlux_jumps',
  'compute_ranlux_jumps.c',
  include_directories : ['../../src'],
)

executable(
  'ref_ranlux_portable',
  'ref_ranlux_portable.c',
  '../../third-party/ranluxpp-portable/ranluxpp.c',
  include_directories : ['../../third-party/ranluxpp-portable'],
)

executable(
  'ref_ranlux_hahnmon',
  'ref_ranlux_hahnmon.cpp',
  '../../third-party/ranluxpp-hahnmon/RanluxppEngine.cpp',
  include_directories : ['../../third-party/ranluxpp-hahnmon'],
)

executable(
  'TimeRanluxHahnfeld',
  [
    'TimeRanluxHahnfeld.cpp',
    '../../common/getopt.c',
    '../../third-party/ranluxpp-portable/ranluxpp.c',
    '../../third-party/ranluxpp-hahnmon/RanluxppEngine.cpp',
  ],
  include_directories : [
    '../../common',
    '../../third-party/ranluxpp-portable',
    '../../third-party/ranluxpp-hahnmon',
  ],
  dependencies : [randompack_dep],
)

executable(
  'verify_agreement',
  [
    'verify_agreement.cpp',
    '../../third-party/ranluxpp-portable/ranluxpp.c',
    '../../third-party/ranluxpp-hahnmon/RanluxppEngine.cpp',
  ],
  include_directories : [
    '../../third-party/ranluxpp-portable',
    '../../third-party/ranluxpp-hahnmon',
  ],
)

executable(
  'extract_hahnmon_lcg',
  'extract_hahnmon_lcg.cpp',
  include_directories : [
    '../../third-party/ranluxpp-hahnmon',
  ],
)
