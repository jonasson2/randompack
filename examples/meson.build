example_util = files(
  'getopt.c',
)

run_varmasim = executable(
  'RunRandom',
  ['RunRandom.c'] + example_util,
  dependencies : [randompack_dep],
)

run_cryptorandom = executable(
  'RunCryptoRandom',
  ['RunCryptoRandom.c'] + example_util,
  dependencies : [randompack_dep],
)

timing = executable(
  'Timing',
  ['Timing.c'],
  dependencies : [randompack_dep],
)
