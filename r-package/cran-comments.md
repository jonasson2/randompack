## Notes on .inc files

The src/ directory contains .inc files which are C source fragments
included directly into a single translation unit (unity build) for
performance and inlining reasons. This is an intentional build strategy,
not an error.

## New submission
This is the first submission of this package to CRAN.

## Test environments
- macOS aarch64 (local), R 4.x.x
- Linux x86_64 (local), R 4.x.x
- win-builder (release and devel)
- R-hub

## Extra files at top-level
cran-comments.md and README.Rmd are standard devtools/roxygen2 files and 
are listed in .Rbuildignore.
