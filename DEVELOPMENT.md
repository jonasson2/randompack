
Table of contents
-----------------
- [C-library](#c-library)
- [Julia](#julia)
- [Python](#python)
- [R-library](#r-library)
- [Build wheels](#build-wheels)
- [Normal testing](#normal-testing)
- [Benchmarking](#benchmarking)
- [Extra Testing](#extra-testing)
- [TestU01](#testu01)
- [PractRand](#practrand)


C-library
=========
MESON SETUP:
scripts/meson-setup.sh release  # optimized
scripts/meson-setup.sh debug    # debug-enabled
= these set library "prefix" to ./install

BUILD:
ninja -C release
ninja -C debug

INSTALL:
ninja -C release install    # used for Julia and Fortran
                            # (copies library to ./install)
release/examples/RunRandom  # simple example program

All subprojects
===============
scripts/set_version.sh      # sets version number across all the language interfaces
build-all.sh                # rebuilds all language interfaces

Julia
=====
FIRST TIME SETUP:
cd Randompack.jl           # the Julia package folder
export JULIA_PROJECT=.     # use environment defined by ./Project.toml
julia                      # run julia
julia> ] instantiate       # install project dependencies (once per Julia version) (1)

AFTER CHANGE TO C LIBRARY:
Make sure to run "ninja -C release install" and restart Julia

USE:
export JULIA_PROJECT=.     # needed in a new shell (2)
julia examples/TimeDist.jl # run timing program
julia                      # run julia
julia> using Randompack    # load into current session

TO UPDATE JULIA
juliaup add 1.13           # install Julia version 1.13
juliaup default 1.13       # and make it default (3)

Notes:
(1) ] enters pkg> prompt; <backspace> to go back to julia> prompt
(2) alternatively, set JULIA_PROJECT in .bashrc/.zshrc with absolute path
(3) or run Julia with julia +1.13

Python
======
FIRST TIME SETUP:
cd <project-root>                            # enter project root
scripts/syncpy.sh                            # copy C sources from src to python/src
python -m venv .venv                         # create a local virtual environment (once)
source .venv/bin/activate                    # activate it
pip install meson meson-python ninja cython  # add needed build tools
pip install numpy pytest                     # add packages
cd python                                    # enter python library folder
pip install -e . --no-build-isolation        # install Randompack in editable mode

AFTER CHANGE TO C LIBRARY
scripts/syncpy.sh                            # copy C sources from src to python/src
restart python

USE:
(with Python running in the same environment)
>>> import randompack
will automatically rebuild the module after changes to C/Cython sources.

WITH CONDA:
cd <project-root>
conda create -n randompack-dev python
conda activate randompack-dev
conda install meson meson-python ninja cython pytest numpy
cd python
pip install -e . --no-build-isolation
= optionally specify Python version: "conda create -n randompack-dev python=3.12"

R-library
=========
BUILD:
cd <project-root>                  # enter project root
scripts/syncR.sh                   # copy C sources from src to r-package/src
R CMD install r-package            # create R library             
= do this both first-time and 
  after changes to C library; 
  then restart R

USE:
R                                  # start R
> library(randompack)              # load into session

Normal testing
==============
meson test -C release              # run meson defined tests for C and Fortran
release/tests/RunTests -v          # run C tests in verbose mode
release/tests/RunTests -h          # show help for test runner
release/tests/RunFortranTests      # Run only the Fortran tests

cd python
pytest -q                          # quiet testing of the Python Randompack
pytest                             # verbose testing

cd <project-root>
R CMD build r-package              # build library in randompack_<version>.tar.gz 
R CMD check randompack_*.tar.gz    # comprehensive CRAN-style full package check
.                                  # replace * with actual version if > 1 .tar.gz file
cd r-package                       # enter the package folder
R                                  # start R
> install.packages("testthat")     # first time only
> testthat::test_local()           # run quick testthat tests

cd Randompack.jl                   # enter Julia folder
export JULIA_PROJECT=.             # unless set in .zshrc/.bashrc
julia test/runtests.jl             # run the Julia tests

Benchmarking
============
cd <projectroot>/release/examples  # enter build folder
TimeDistC                          # benchmark distributions with C randompack
TimeDistFortran                    # = distributions with Fortran randompack
TimeEngines                        # = engines with C bitstream samples
TimeIntegers                       # = integer sampling with C

cd <project-root>
export JULIA_PROJECT=Randompack.jl          # if not set in .zshrc/.bashrc
Rscript r-package/inst/examples/TimeDist.R  # compare R-randompack with base-R and Dqrng
julia Randompack.jl/examples/TimeDist.jl    # compare Julia randompack with the built-in
python python/examples/TimeDist.py          # compare Python-randompack with numpy.random

release/examples/TimeDistC -h               # display short help
.                                           # the other benchmark programs also accept -h

Extra Testing
=============
scripts/run-test-variants.sh  # Run built-in BLAS, no128, and nosimd variants
scripts/test-all.sh           # Run tests for all language interfaces

TestU01:
In some folder:
wget http://simul.iro.umontreal.ca/testu01/TestU01.zip  # Download official TestU01
unzip TestU01.zip                                       # = unzip it
cd TestU01-1.2.3                                        # = and install it 
configure --prefix=<prefix>                             # E.g. $HOME/lib
make -j                                                 #
make install                                            #
meson setup -C release -Dbuildtype=release \            #
    -DTestU01=<prefix>                                  # or edit meson_config.txt
cd release/examples                                     #
TestU01Driver -h                                        # help
TestU01Driver -c                                        # Crush (minutes)
TestU01Driver -b                                        # BigCrush (hours)
TestU01PIT -h                                           # help
TestU01PIT -c                                           # PIT test normal with Crush

PractRand:
[This only works on x86_64 Linux]
cd to <project-root>/misc
Download from https://sourceforge.net/projects/pracrand/files
unzip and enter PractRand folder
g++ -O3 -std=c++11 -pthread -Iinclude src/*.cpp src/RNGs/*.cpp src/RNGs/other/*.cpp 
  tools/RNG_test.cpp -o ../../release/examples/RNG_test
cd <project-root>/release/examples

RawStream | Rng_test stdin64           # default engine, runs "forever"
RawStream -e x128+ | RNG_test stdin64  # x128+ fails fast
RawStream -h                           # help
Rng_test -h                            # help

Julia release
=============
Yggdrasil (Julia):
scripts/make-julia-tarball.sh         # create .tar.gz file with C library in archives/
cd archives                           # clone of github.com/jonasson2/randompack-src
– update packageng/build_tarballs.jl  # change version number (2x) and sha (from script)
git commit -am "Release x.y.z"        #
git push                              # 
git tag vx.y.z                        # (can also use local tarball from build_tarball.jl
git push origin vx.y.z                # push new tag
julia build_tarballs.jl --debug                  # 
  --verbose --deploy=local                       # Check all platforms
  x86_64-linux-gnu-libgfortran5-cxx11            # locally
  [repeat for all platforms]                     #
git clone git@github.com:jonasson2/Yggdrasil.git #
update build_tarballs.jl in R/Randompack         # - in the clone
git commit -m "Add Randompack -vx.y.z            #
git push                                         # push to Yggdrasil
gh pr create --repo JuliaPackaging/Yggdrasil \   # open pull request for a merge
     --title "Add Randompack vX.Y.Z" \           # 
     --body "New version of Randompack"          #

For Julia General registry:
Wait for the Yggdrasil merge
in julia:
  ] activate Randompack.jl
  ] add Randompack_jll
  ] update
Commit and push randompack (including Randompack.jl)
Tag it vX.Y.Z
Comment on the new commit on github: @JuliaRegistrator register

Python PiPY
===========
Create wheels:
Bump version with set_version.sh
commit, push, and tag randompack
scripts/wheel-build.sh             # trigger github Actions
gh run list                        # monitor runs
gh run view <RUN-ID> --log-failed  # view failed log
gh run delete <RUN-ID>             # delete run
.                                  # wait for the actions to complete successfully
cd python/
python -m build -s                 # build sdist in dist/
scripts/wheel-download.sh          # download wheels from GH Actions into dist/

Upload to TestPyPI:
python -m twine upload --repository testpypi dist/*     # verify on TestPyPI
python -m venv /tmp/rp-test                             # - using a fresh venv
source /tmp/rp-test/bin/activate                        # 
pip install \                                           # try out the new TestPyPI version 
  --index-url https://test.pypi.org/simple \            #
  --extra-index-url https://pypi.org/simple \           #
  randompack                                            #
python -c "import randompack; print(randompack.Rng())"  #
deactivate                                              # exit the venv

Upload to PyPI:
python -m twine upload dist/*   # real upload
pip install randompack          # try it out

More wheel commands:
gh workflow list
gh workflow run xxx.yml  # trigger wheel building
gh run watch
gh run list --json databaseId --jq '.[].databaseId'  # just the id-s, use in foreach...

CRAN submission
===============
scripts/syncR.sh

From r-package:
R -e "devtools::document()"            # Creates man docs
R -e "devtools::build_readme()"        # Creates README.md from README.Rmd
R -e "devtools::check_win_devel()"     # offline-check of Windows
R -e "devtools::check_win_release()"   # do.

From project root:
R CMD build r-package
R CMD check randompack_*.gz
R CMD check --as-cran randompack_*.gz  # complete checking
                                       # edit cran-comments.md with results
