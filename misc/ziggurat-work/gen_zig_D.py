import re
from pathlib import Path

import numpy as np
from scipy.special import lambertw


ROOT = Path(__file__).resolve().parent
CONST_PATH = ROOT / "ziggurat_constants.h"
OUT_PATH = ROOT / "zig_D.h"


def parse_uint32_array(name, text):
  body = re.search(
    rf"static const uint32_t {name}\[\] = \{{(.*?)\}};",
    text,
    re.S,
  ).group(1)
  vals = re.findall(r"0x[0-9A-Fa-f]+", body)
  return np.array([int(v.rstrip("UL"), 16) for v in vals], dtype=np.uint32)


def parse_uint64_array(name, text):
  body = re.search(
    rf"static const uint64_t {name}\[\] = \{{(.*?)\}};",
    text,
    re.S,
  ).group(1)
  vals = re.findall(r"0x[0-9A-Fa-f]+(?:[uUlL]+)?", body)
  return np.array([int(re.sub(r"[uUlL]+$", "", v), 16) for v in vals], dtype=np.uint64)


def parse_float_array(name, text):
  body = re.search(
    rf"static const float {name}\[\] = \{{(.*?)\}};",
    text,
    re.S,
  ).group(1)
  vals = re.findall(r"[-+]?\d*\.\d+(?:[eE][-+]?\d+)?f?", body)
  return np.array([v.rstrip("f") for v in vals], dtype=np.float32)


def parse_double_array(name, text):
  body = re.search(
    rf"static const double {name}\[\] = \{{(.*?)\}};",
    text,
    re.S,
  ).group(1)
  vals = re.findall(r"[-+]?\d*\.\d+(?:[eE][-+]?\d+)?", body)
  return np.array(vals, dtype=np.float64)


def compute_dnorm(fi, wi, expon, qdtype):
  fi = fi.astype(np.float64)
  wi = wi.astype(np.float64)
  xi = wi*(2**expon)
  xi[0] = 0
  h = np.ones(256, dtype=np.float64)
  dx = np.ones(256, dtype=np.float64)
  l = np.zeros(256, dtype=np.float64)
  sec = np.ones(256, dtype=np.float64)
  gap = np.ones(256, dtype=np.float64)
  h[1:] = fi[:-1] - fi[1:]
  dx[1:] = xi[1:] - xi[:-1]
  l[1:] = dx[1:]/xi[1:]
  L = (2**expon)*l
  arg = -(h/dx)**2
  si = np.empty_like(xi)
  si[xi < 1] = np.sqrt(-np.real(lambertw(arg[xi < 1], 0)))
  si[xi >= 1] = np.sqrt(-np.real(lambertw(arg[xi >= 1], -1)))
  si52 = np.sqrt(-np.real(lambertw(arg[52], 0)))
  sec[1:] = fi[:-1] - (si[1:] - xi[:-1])*h[1:]/dx[1:]
  sec52 = fi[51] - (si52 - xi[51])*h[52]/dx[52]
  gap[1:] = np.abs(np.exp(-si[1:]**2/2) - sec[1:])
  d = np.array(np.floor(L*gap/h), dtype=qdtype)
  d[0] = 0
  d52 = qdtype(np.floor(np.abs(np.exp(-si52**2/2) - sec52)*L[52]/h[52]))
  return d, d52


def compute_dexp(fi, wi, expon, qdtype):
  fi = fi.astype(np.float64)
  wi = wi.astype(np.float64)
  xi = wi*(2**expon)
  h = np.ones(256, dtype=np.float64)
  dx = np.ones(256, dtype=np.float64)
  l = np.zeros(256, dtype=np.float64)
  sec = np.ones(256, dtype=np.float64)
  gap = np.ones(256, dtype=np.float64)
  h[1:] = fi[:-1] - fi[1:]
  dx[1] = xi[1]
  dx[2:] = xi[2:] - xi[1:-1]
  l[1:] = dx[1:]/xi[1:]
  L = (2**expon)*l
  si = np.zeros(256, dtype=np.float64)
  si[1:] = np.log(dx[1:]/h[1:])
  sec[1] = 1 - si[1]*h[1]/dx[1]
  sec[2:] = fi[1:-1] - (si[2:] - xi[1:-1])*h[2:]/dx[2:]
  gap[1:] = np.abs(sec[1:] - np.exp(-si[1:]))
  d = np.array(np.floor(L*gap/h), dtype=qdtype)
  d[0] = 0
  return d


def emit_array(name, ctype, width, suffix, vals):
  lines = [f"static const {ctype} {name}[] = {{\n"]
  for i in range(0, len(vals), 2):
    if i + 1 < len(vals):
      lines.append(
        f"  0x{int(vals[i]):0{width}X}{suffix}, "
        f"0x{int(vals[i + 1]):0{width}X}{suffix},\n"
      )
    else:
      lines.append(f"  0x{int(vals[i]):0{width}X}{suffix},\n")
  lines.append("};\n")
  return "".join(lines)


def main():
  text = CONST_PATH.read_text()
  out = ["#ifndef ZIG_D_H\n", "#define ZIG_D_H\n\n"]
  d52 = 0
  d52f = 0

  configs = [
    ("Dnorm", False, False, "uint64_t", 16, "ULL"),
    ("Dexp", False, True, "uint64_t", 16, "ULL"),
    ("Dnormf", True, False, "uint32_t", 8, "UL"),
    ("Dexpf", True, True, "uint32_t", 8, "UL"),
  ]

  for name, use_float, use_exp, ctype, width, suffix in configs:
    if use_float:
      fi = parse_float_array("fe_float" if use_exp else "fi_float", text)
      wi = parse_float_array("we_float" if use_exp else "wi_float", text)
      qdtype = np.uint32
      expon = 23
    else:
      fi = parse_double_array("fe_double" if use_exp else "fi_double", text)
      wi = parse_double_array("we_double" if use_exp else "wi_double", text)
      qdtype = np.uint64
      expon = 53 if use_exp else 52
    if use_exp:
      vals = compute_dexp(fi, wi, expon, qdtype)
    else:
      vals, dnorm52 = compute_dnorm(fi, wi, expon, qdtype)
      if use_float:
        d52f = dnorm52
      else:
        d52 = dnorm52
    out.append(emit_array(name, ctype, width, suffix, vals))
    out.append("\n")

  out.append(f"static const uint64_t Dzig52 = 0x{int(d52):016X}ULL;\n\n")
  out.append(f"static const uint32_t Dzig52f = 0x{int(d52f):08X}UL;\n\n")

  out.append("#endif\n")
  OUT_PATH.write_text("".join(out))


if __name__ == "__main__":
  main()
