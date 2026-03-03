project('mul576', 'c',
  default_options: [
    'c_std=c11',
    'warning_level=3',
    'buildtype=release',
  ]
)

cc = meson.get_compiler('c')
extra_args = cc.get_supported_arguments([])
if extra_args.length() > 0
  add_project_arguments(extra_args, language: 'c')
endif

m_dep = dependency('m', required: false)

exe = executable(
  'TimeMul9x9',
  'TimeMul9x9.c',
  'mul9x9.c',
  'mul_jirka.c',
  'mod_m.c',
  include_directories: include_directories('../../third-party/ranluxpp-portable'),
  dependencies: m_dep.found() ? [m_dep] : [],
  install: false,
)
