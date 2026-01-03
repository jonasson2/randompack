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
  ['Timing.c'] + common_util,
  dependencies : [randompack_dep],
)

time_engines = executable(
  'TimeEngines',
  ['TimeEngines.c', 'TimeUtil.c'] + common_util,
  dependencies : [randompack_dep],
)

time_distributions = executable(
  'TimeDistributions',
  ['TimeDistributions.c', 'TimeUtil.c'] + common_util,
  dependencies : [randompack_dep],
)

time_float = executable(
  'TimeFloat',
  ['TimeFloat.c', 'TimeUtil.c'] + common_util,
  dependencies : [randompack_dep],
)

time_integers = executable(
  'TimeIntegers',
  ['TimeIntegers.c'] + common_util,
  dependencies : [randompack_dep],
)
