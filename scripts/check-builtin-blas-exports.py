#!/usr/bin/env python3

import argparse
import importlib
import re
import shutil
import subprocess
import sys
from pathlib import Path


ROUTINE_RE = re.compile(
    r"^\s*(?:recursive\s+)?"
    r"(?:subroutine|(?:double\s+precision|integer|logical)\s+function)"
    r"\s+([a-z][a-z0-9_]*)\s*\(",
    re.IGNORECASE,
)


def routines_in(path):
    routines = set()
    for line in path.read_text(encoding="ascii").splitlines():
        match = ROUTINE_RE.match(line)
        if match:
            routines.add(match.group(1).lower())
    if not routines:
        raise RuntimeError(f"no Fortran routines found in {path}")
    return routines


def run_tool(command):
    result = subprocess.run(command, check=False, text=True, capture_output=True)
    if result.returncode != 0:
        details = result.stderr.strip() or result.stdout.strip()
        raise RuntimeError(f"{' '.join(command)} failed: {details}")
    return result.stdout


def windows_symbol_variants(symbol):
    bases = {symbol, symbol.lower(), symbol.upper()}
    return {
        decorated
        for base in bases
        for decorated in (base, base + "_", base + "__", "_" + base)
    }


def windows_exports(path, candidates):
    dumpbin = shutil.which("dumpbin")
    if dumpbin:
        return run_tool([dumpbin, "/nologo", "/exports", str(path)])
    llvm_readobj = shutil.which("llvm-readobj")
    if llvm_readobj:
        return run_tool([llvm_readobj, "--coff-exports", str(path)])
    import ctypes

    library = ctypes.WinDLL(str(path))
    found = []
    for symbol in candidates:
        for variant in windows_symbol_variants(symbol):
            try:
                getattr(library, variant)
            except AttributeError:
                continue
            found.append(variant)
    return "\n".join(found)


def exported_symbols(path, candidates):
    if sys.platform == "darwin":
        return run_tool(["nm", "-gU", str(path)])
    if sys.platform == "win32":
        return windows_exports(path, candidates)
    return run_tool(["nm", "-D", "--defined-only", str(path)])


def find_routine_exports(output, routines):
    found = []
    for routine in sorted(routines):
        pattern = re.compile(
            rf"(?<![a-z0-9])_?{re.escape(routine)}_{{0,2}}(?:@\d+)?"
            rf"(?![a-z0-9_])",
            re.IGNORECASE,
        )
        if pattern.search(output):
            found.append(routine)
    return found


def require_symbol(output, symbol):
    pattern = re.compile(
        rf"(?<![a-z0-9])_?{re.escape(symbol)}(?:@\d+)?(?![a-z0-9_])",
        re.IGNORECASE,
    )
    if not pattern.search(output):
        raise RuntimeError(f"required symbol is not exported: {symbol}")


def main():
    parser = argparse.ArgumentParser(
        description="Check that Randompack's built-in BLAS is not exported."
    )
    parser.add_argument("blas_source", type=Path)
    parser.add_argument("library", nargs="?", type=Path)
    parser.add_argument("--module")
    parser.add_argument("--require", action="append", default=[])
    args = parser.parse_args()
    if bool(args.library) == bool(args.module):
        parser.error("specify either library or --module")
    if args.module:
        library = Path(importlib.import_module(args.module).__file__)
    else:
        library = args.library
    routines = routines_in(args.blas_source)
    output = exported_symbols(library, routines | set(args.require))
    found = find_routine_exports(output, routines)
    if found:
        names = ", ".join(found)
        raise RuntimeError(f"built-in BLAS routines are exported: {names}")
    for symbol in args.require:
        require_symbol(output, symbol)
    print(f"Built-in BLAS exports hidden in {library}")


if __name__ == "__main__":
    try:
        main()
    except RuntimeError as error:
        print(f"error: {error}", file=sys.stderr)
        sys.exit(1)
