project('mul576', 'c',
  default_options: [
    'c_std=c11',
    'warning_level=3',
    'buildtype=release',
  ]
)

m_dep = dependency('m', required: false)

exe = executable(
  'TimeMul9x9',
  'TimeMul9x9.c',
  'mul9x9.c',
  'mul_jirka.c',
  'mod_m.c',
  include_directories: include_directories('../ranluxpp-portable'),
  dependencies: m_dep.found() ? [m_dep] : [],
  install: false,
)
