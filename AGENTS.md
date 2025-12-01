You assist with “varmapack,” a portable C11 library for VARMA simulation.
It includes a sublibrary "randompack" for random number generation.
Always obey the rules below.

---------------------------
CORE C STYLE
---------------------------
- ISO C11 only; no compiler extensions except GNU's "eat the comma".  
- Indent 2 spaces; Stroustrup braces; // comments.  
- Pointer form: int *x. Prefer i++ in loops.  
- Max line length 90; no spaces around *.  
- Use bool/true/false; prefer 0 over NULL.  
- Public API uses snake_case with prefix varmapack_ or randompack_.  
- Internal helpers are static/static inline without prefixes.  
- No macros except include guards. Use xAssert (not assert) for checks.  
- Avoid min/max macros; use fmin/fmax or inline helpers.

---------------------------
ARCHITECTURE
---------------------------
- Public headers: varmapack.h (and randompack.h for RNG facade).  
- Sources under src/: core C files plus subdirs (e.g., src/r for R gateways).  
- Tests live in tests/, examples in examples/.  
- Meson/Ninja drive builds; no visibility attributes.  

---------------------------
STATUS / EXPECTATIONS
---------------------------
- Only varmapack_* and randompack_* APIs are public; internal helpers must stay private.

---------------------------
OUTPUT
---------------------------
- Keep responses concise, technical, and style-compliant.  
- Do not restate these instructions.
- Provide concrete answers or code following the rules above.
- Stroustrup braces also on function signature lines
- Use 0 instead of NULL
