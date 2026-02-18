C-library
–––––––––
MESON SETUP:
scripts/meson-setup.sh release  # optimized
scripts/meson-setup.sh debug    # debug-enabled
– these set library "prefix" to ./install

BUILD:
ninja -C release
ninja -C debug

INSTALL:
ninja -C release install  # used for Julia and Fortran
                          # (copies library to ./install)

Julia
–––––
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
––––––
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
– optionally specify Python version: "conda create -n randompack-dev python=3.12"

R-library
–––––––––
BUILD:
cd <project-root>                  # enter project root
scripts/syncR.sh                   # copy C sources from src to r-package/src
R CMD build r-package              # build library in randompack_<version>.tar.gz
R CMD INSTALL randompack_*.tar.gz  # replace * with actual version if > 1 .tar.gz file
– do this both first-time and after changes to C library; then restart R

USE:
R                                  # start R
> library(randompack)              # load into session

Testing
–––––––
meson test -C release       # run meson defined tests
release/tests/RunTests -v   # run the tests in verbose mode
release/tests/RunTests -h   # show help for test runner

cd 
pytest -q                   $
