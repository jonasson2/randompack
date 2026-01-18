EXAMPLE, TEST AND TIMING PROGRAMS FOR RANDOMPACK

- The example programs in here are built automatically by "meson compile..." or "ninja".
- Most example program show help when issued with: <program> -h
- For testing with PractRand, it must be set up first (see misc/practrand.patch)
- For testing with TestU01, see TestU01Driver.c
- The following commands are issued from the package root (or a subdirectory after cd)

    meson setup build                        # for a debug-enabled code
    meson setup release -Dbuildtype=release  # optimized build
    ./meson-setup-perf.sh                    # performance build
    cd release                               
    ninja                                    # compile
    ninja test                               # run the tests
    cd tests
      RunTests -v                            # run verbosely
      RunTests -vv                           # more verbosity
    cd ../examples    
    TimeEngines                              # run timing programs
    TimeDistributions                        #
    TimeInteger                              #
    RunRandom                                # example driver
    RawStream -e x128+ | RNG_test stdin64    # test with PractRand (fails)
    RawStream -e pcg64 | RNG_test stdin64    # test with PractRand (doesn't fail)
    TestU01Driver -e x128+ -s                # run SmallCrush
    TestU01Driver -e x128+ -c                # run Crush (use -b for BigCrush)
    
