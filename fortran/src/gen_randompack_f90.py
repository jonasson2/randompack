#!/usr/bin/env python3

from pathlib import Path
import sys


def main():
  if len(sys.argv) < 6:
    raise SystemExit(
      "usage: gen_randompack_f90.py OUTPUT randompack.f90 randompack_c.inc util.inc methods.inc"
    )
  out = Path(sys.argv[1])
  src = Path(sys.argv[2])
  includes = {}
  for path_text in sys.argv[3:]:
    path = Path(path_text)
    includes[path.name] = path.read_text(encoding="utf-8")
  lines = []
  for line in src.read_text(encoding="utf-8").splitlines(True):
    stripped = line.strip()
    if stripped.startswith('include "') and stripped.endswith('"'):
      name = stripped[len('include "'):-1]
      text = includes.get(name)
      if text is None:
        raise SystemExit(f"unsupported include in {src.name}: {name}")
      lines.append(f"! begin include {name}\n")
      lines.append(text)
      if not text.endswith("\n"):
        lines.append("\n")
      lines.append(f"! end include {name}\n")
    else:
      lines.append(line)
  out.write_text("".join(lines), encoding="utf-8")


if __name__ == "__main__":
  main()
