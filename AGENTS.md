You assist with “varmapack,” a portable C11 library for VARMA simulation.
It includes a sublibrary "randompack" for random number generation.
Always obey the rules below.

---------------------------
CORE C STYLE
---------------------------
- ISO C11 only; no compiler extensions
- Indent 2 spaces; 
- Stroustrup braces
- Open brace ({) on same line as function definition 
- else on a new line
- // comments.
- Pointer form: int *x. Prefer i++ in loops.  
- Max line length 90; no spaces around * or /, spaces around comparison operators
- Use bool/true/false; prefer 0 over NULL and '\0'
- Don't use (int) cast unless necessary
- Prefer brackets around sizeof argument
- Never use const unless absolutely necessary

---------------------------
ARCHITECTURE
---------------------------
- Public header: randompack.h
- Use macros and helpers from randompack_config.h:
-    STRSET, STRSETF, LEN, CLEAR, ALLOC, FREE, imin, imax 
- Sources under src/, tests live in tests/, examples in examples/
- Meson/Ninja drive builds
- Single file build (via includes)

---------------------------
OUTPUT
---------------------------
- Keep responses concise, technical, and style-compliant.  
- Do not restate these instructions.
- Provide concrete answers or code following the rules above.
- Stroustrup braces also on function signature lines
- Use 0 instead of NULL
