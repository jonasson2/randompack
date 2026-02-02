add_languages('cpp', required: true)

test_sources = files(
  'RunTests.c',
  'TestCreate.c',
  'TestInt.c',
  'TestPerm.c',
  'TestSample.c',
  'TestNorm.c',
  'TestUnif.c',
  'TestContinuous.c',
  'TestU01.c',
  'TestMvn.c',
  'TestUint8.c',
  'TestUint16.c',
  'TestUint32.c',
  'TestUint64.c',
  'TestReference.c',
  'TestSeed.c',
  'TestSetState.c',
  'TestHelpers.c',
  'TestBuffer.c',
)

extra_util = files(
  'xCheck.c',
  'TestUtil.c',
  'incbet.c',
  'igam.c',
)

run_tests = executable(
  'RunTests',
  test_sources + extra_util,
  dependencies : [randompack_dep],
)

test('RunTests', run_tests, args : ['-v'])

if get_option('build_test_variants')
  test_no128 = executable(
    'RunTestNo128',
    [
      'RunTestNo128.c',
      'TestCreate.c',
      'TestHelpers.c',
    ] + extra_util,
    dependencies : [randompack_no128_dep],
    c_args : ['-DRANDOMPACK_PRETEND_NO128'],
  )

  test('RunTestNo128', test_no128, args : ['-v'])

  test_pretend_windows = executable(
    'RunTestPretendWindows',
    [
      'RunTestNo128.c',
      'TestCreate.c',
      'TestHelpers.c',
    ] + extra_util,
    dependencies : [randompack_pretend_windows_dep],
    c_args : ['-DRANDOMPACK_PRETEND_WINDOWS'],
  )

  test('RunTestPretendWindows', test_pretend_windows, args : ['-v'])
endif

seedseq_test = executable(
  'TestSeedSeq',
  ['TestSeedSeq.cpp'],
  include_directories : [inc],
)

test('TestSeedSeq', seedseq_test)
