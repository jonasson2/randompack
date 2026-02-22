#!/bin/sh
set -eu
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
REPO_ROOT=$(dirname "$SCRIPT_DIR")
cd "$REPO_ROOT"

# Regenerate documentation from roxygen comments
Rscript -e 'roxygen2::roxygenise("r-package")'
# Generate HTML for documentation files
Rscript -e 'tools::Rd2HTML("r-package/man/randompack-package.Rd", out = "randompack-package.html")'
Rscript -e 'tools::Rd2HTML("r-package/man/randompack_rng.Rd", out = "randompack_rng.html")'
Rscript -e 'tools::Rd2HTML("r-package/man/randompack_engines.Rd", out = "randompack_engines.html")'
# Open the package documentation
open randompack*.html
