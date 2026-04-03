#!/usr/bin/env python3

import argparse
import bisect
import math
import sys
from collections import defaultdict
from statistics import NormalDist


BASE_ENGINES = [
    "x256ss",
    "pcg64",
    "sfc64",
    "chacha20",
    "cwg128",
    "ranlux++",
    "philox",
    "squares",
]

XOR_ENGINES = {
    "x256ss",
    "x256++",
    "x128+",
    "xoro++",
}


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--families-out",
        default="calibrated-families.tsv",
        help="output path for family-by-engine calibrated Simes table",
    )
    parser.add_argument(
        "--engines-out",
        default="calibrated-engines.tsv",
        help="output path for engine summary table",
    )
    return parser.parse_args()


def read_input(stream):
    data = defaultdict(list)
    families = []
    family_seen = set()
    engines = []
    engine_seen = set()

    for raw_line in stream:
        line = raw_line.strip()
        if not line:
          continue

        parts = line.split()
        if len(parts) != 3:
          continue

        engine, family, value_text = parts
        value = float(value_text)
        data[engine, family].append(value)

        if engine not in engine_seen:
          engine_seen.add(engine)
          engines.append(engine)

        if family not in family_seen:
          family_seen.add(family)
          families.append(family)

    return data, families, engines


def reference_engines(engine):
    if engine in XOR_ENGINES:
      drop = "x256ss"
    else:
      drop = engine
    return [name for name in BASE_ENGINES if name != drop]


def simes(values):
    ordered = sorted(values)
    n = len(ordered)
    best = 1.0
    for index, value in enumerate(ordered, start=1):
      scaled = n*value/index
      if scaled < best:
        best = scaled
    return min(best, 1.0)


def fisher(values):
    return -2.0*sum(math.log(value) for value in values)


def stouffer(values):
    normal = NormalDist()
    total = 0.0
    eps = 1e-15
    for value in values:
      tail = 1.0 - value
      if tail <= eps:
        tail = eps
      elif tail >= 1.0 - eps:
        tail = 1.0 - eps
      total += normal.inv_cdf(tail)
    return total/math.sqrt(len(values))


def calibrated_u(value, reference):
    less = bisect.bisect_left(reference, value)
    more = bisect.bisect_right(reference, value)
    equal = more - less
    return (1 + less + 0.5*equal)/(len(reference) + 1)


def format_float(value):
    if math.isnan(value):
      return "nan"
    if math.isinf(value):
      return "inf" if value > 0 else "-inf"
    return f"{value:.6g}"


def write_table(path, header, rows):
    with open(path, "w", encoding="utf-8") as handle:
      handle.write("\t".join(header))
      handle.write("\n")
      for row in rows:
        handle.write("\t".join(row))
        handle.write("\n")


def main():
    args = parse_args()
    data, families, engines = read_input(sys.stdin)

    missing = [engine for engine in BASE_ENGINES if all((engine, fam) not in data
      for fam in families)]
    if missing:
      raise SystemExit("missing base engines: " + ", ".join(missing))

    family_table = {}
    for engine in engines:
      refs = reference_engines(engine)
      for family in families:
        target = data.get((engine, family))
        if not target:
          raise SystemExit(f"missing raw data for {engine} {family}")

        reference = []
        for ref_engine in refs:
          values = data.get((ref_engine, family))
          if not values:
            raise SystemExit(f"missing reference data for {ref_engine} {family}")
          reference.extend(values)

        reference.sort()
        calibrated = [calibrated_u(value, reference) for value in target]
        family_table[family, engine] = simes(calibrated)

    family_rows = []
    for family in families:
      row = [family]
      for engine in engines:
        row.append(format_float(family_table[family, engine]))
      family_rows.append(row)

    engine_rows = []
    for engine in engines:
      values = [family_table[family, engine] for family in families]
      engine_rows.append([
        engine,
        format_float(min(values)),
        format_float(fisher(values)),
        format_float(stouffer(values)),
        format_float(simes(values)),
      ])

    write_table(args.families_out, ["Family", *engines], family_rows)
    write_table(
        args.engines_out,
        ["Engine", "min-u", "Fisher", "Stouffer", "Simes"],
        engine_rows,
    )


if __name__ == "__main__":
    main()
