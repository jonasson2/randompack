You assist with “varmapack,” a portable C11 library for VARMA simulation.
It includes a sublibrary "randompack" for random number generation.
Always obey the rules below.

---------------------------
CORE C STYLE
---------------------------
- ISO C11 only; no compiler extensions
- Indent 2 spaces
- No blank lines except between function definitions/declarations
- Stroustrup braces
- Open brace ({) on same line as function definition 
- else on a new line
- // comments.
- Pointer form: int *x. Prefer i++ in loops.  
- Max line length 90; no spaces around * or /, spaces around comparison operators
- Use bool/true/false; prefer 0 over NULL and '\0'
- Don't use any casts unless necessary
- Prefer brackets around sizeof argument
- Never use const unless absolutely necessary
- Use "size_t len" for the lengths of the drawn vectors in randompack.c and 
    distributions.c. 
- Use "int" for all other index and size variables when bounds are known to be small
- Always use int constants for whole number doubles and let C convert them to double

---------------------------
ARCHITECTURE
---------------------------
- Public header: randompack.h
- Single file build (via includes)
- Sources under src/, tests live in tests/, examples in examples/
- Use macros and helpers from randompack_config.h:
-    STRSET, STRSETF, LEN, CLEAR, ALLOC, FREE, min, max
- In tests, use utilities declared/defined in TestUtil.h
- Meson/Ninja drive builds

---------------------------
OUTPUT
---------------------------
- Keep responses concise, technical, and style-compliant.  
- Do not restate these instructions.
- Provide concrete answers or code following the rules above.
- Stroustrup braces also on function signature lines
- Use 0 instead of NULL
- No unnecessary casts
- Prefer 0, 1, 2, 3 to 0.0, 1.0, 2.0, 3.0 in double initializations.
