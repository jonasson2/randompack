## Test environments
- macOS aarch64 (local), R 4.5.2
- Windows Server 2022 x64, R-devel (win-builder)
- Windows Server 2022 x64, R-release (win-builder)

## R CMD check results
There were no ERRORs or WARNINGs.

There were the following NOTEs:
1. `New submission` — this is the first submission of `randompack` to CRAN.
2. win-builder reported the use of non-portable compiler flags `-mavx2` and `-mfma`. These flags are used only for optional SIMD-specific source files selected at configure time when the compiler supports them. The package contains scalar fallback implementations, and runtime dispatch is used so that the generic code path remains available on machines without these instruction sets.

## Submission
This is the first submission of `randompack` to CRAN.
