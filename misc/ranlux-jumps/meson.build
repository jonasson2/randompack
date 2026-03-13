project('ranlux-jumps', 'c',
  default_options: [
    'c_std=c11',
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

executable(
  'compute_ranlux_jumps',
  'compute_ranlux_jumps.c',
  include_directories : ['../../src'],
)
