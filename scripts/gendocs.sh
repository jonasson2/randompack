#!/bin/bash
# Regenerate documentation from roxygen comments
set -eu
[ -f .randompack-root ] || {
  echo "gendocs.sh: run this from the repository root (missing .randompack-root)" 1>&2
  exit 1
}
Rscript -e 'roxygen2::roxygenise("r-package")'
# Generate HTML for documentation files
Rscript -e 'tools::Rd2HTML("r-package/man/randompack-package.Rd", out = "randompack-package.html")'
Rscript -e 'tools::Rd2HTML("r-package/man/randompack_rng.Rd", out = "randompack_rng.html")'
Rscript -e 'tools::Rd2HTML("r-package/man/randompack_engines.Rd", out = "randompack_engines.html")'
# Open the package documentation
open randompack*.html
