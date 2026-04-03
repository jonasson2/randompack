#!/usr/bin/env python3

import sys


ABBREV = {
    "multin_MultinomialOver": "MultOver",
    "marsa_BirthdaySpacings": "BirthSp",
    "npair_ClosePairs": "ClosePair",
    "knuth_SimpPoker": "SimpPok",
    "knuth_CouponCollector": "Coupon",
    "knuth_Gap": "Gap",
    "knuth_Run": "KnuthRun",
    "multin_Multinomial": "Multinom",
    "knuth_MaxOft": "MaxOft",
    "varia_SampleProd": "SampProd",
    "varia_SampleMean": "SampMean",
    "varia_SampleCorr": "SampCorr",
    "varia_AppearanceSpacings": "AppSpac",
    "varia_WeightDistrib": "Weight",
    "varia_SumCollector": "SumColl",
    "marsa_MatrixRank": "Matrix",
    "marsa_Savir2": "Savir2",
    "marsa_GCD": "GCD",
    "walk_RandomWalk1": "RandWalk",
    "comp_LinearComp": "LinComp",
    "comp_LempelZiv": "LempelZiv",
    "spectral_Fourier3": "Fourier3",
    "string_LongestHeadRun": "LongHead",
    "string_PeriodsInStrings": "PeriodStr",
    "string_HammingWeight2": "HamWeight",
    "string_HammingCorr": "HamCorr",
    "string_HammingIndep": "HamIndep",
    "string_Run": "StringRun",
    "string_AutoCor": "AutoCor",
}


def format_q(value):
    if value == 0:
        return "0"
    text = f"{value:.0e}"
    text = text.replace("e-0", "e-")
    text = text.replace("e+0", "e+")
    return text


def format_min(entry):
    if entry is None:
        return "-"
    return format_q(entry)


def format_count(total, nseeds, show_seeds):
    if show_seeds:
        return f"{total} ({nseeds})"
    return str(total)


def print_table(header, rows):
    widths = [len(name) for name in header]
    for row in rows:
        for i, cell in enumerate(row):
            widths[i] = max(widths[i], len(cell))
    for row in [header, *rows]:
        pieces = []
        for i, cell in enumerate(row):
            if i == 0 or i >= 5:
                pieces.append(cell.ljust(widths[i]))
            else:
                pieces.append(cell.rjust(widths[i]))
        print("  ".join(pieces))


def main():
    folder = sys.argv[1] if len(sys.argv) >= 2 else ""
    want_engine = sys.argv[2] if len(sys.argv) >= 3 else ""
    exclude_family = sys.argv[3] if len(sys.argv) >= 4 else ""
    values = {}
    seeds = set()
    engines = set()
    ntests = 0

    for raw_line in sys.stdin:
        parts = raw_line.split()
        if len(parts) != 4:
            continue
        seed_text, engine, family, value_text = parts
        if want_engine and engine != want_engine:
            continue
        abbrev = ABBREV.get(family, family)
        if exclude_family and abbrev == exclude_family:
            continue
        seed = int(seed_text)
        value = float(value_text)
        seeds.add(seed)
        engines.add(engine)
        ntests += 1
        if family not in values:
            values[family] = []
        values[family].append((value, seed))

    if len(engines) != 1:
        if want_engine:
            raise SystemExit(f"summary_eng.py expected engine {want_engine}, found {len(engines)} engines")
        raise SystemExit(f"summary_eng.py expects 1 engine, found {len(engines)}")
    if not seeds:
        raise SystemExit("no data")

    engine = next(iter(engines))
    nseeds = len(seeds)
    header = [
        "Family",
        "ntests",
        "n(q<1e-3)",
        "n(q<1e-4)",
        "n(q<1e-5)",
        "lowest-q",
        "2nd-lowest",
        "3rd-lowest",
    ]
    rows = []
    for family, family_values in values.items():
        if len(family_values) % nseeds != 0:
            raise SystemExit(
                f"family {family} has {len(family_values)} values, not divisible by {nseeds} seeds"
            )
        nper = len(family_values)//nseeds
        family_values = sorted(family_values, key=lambda item: (item[0], item[1]))
        counts = [0, 0, 0]
        seeds_below = [set(), set(), set()]
        for value, seed in family_values:
            if value < 1e-3:
                counts[0] += 1
                seeds_below[0].add(seed)
            if value < 1e-4:
                counts[1] += 1
                seeds_below[1].add(seed)
            if value < 1e-5:
                counts[2] += 1
                seeds_below[2].add(seed)
        mins = [
            format_min(family_values[0][0] if len(family_values) >= 1 else None),
            format_min(family_values[1][0] if len(family_values) >= 2 else None),
            format_min(family_values[2][0] if len(family_values) >= 3 else None),
        ]
        rows.append([
            ABBREV.get(family, family),
            str(nper),
            format_count(counts[0], len(seeds_below[0]), True),
            format_count(counts[1], len(seeds_below[1]), True),
            format_count(counts[2], len(seeds_below[2]), False),
            mins[0],
            mins[1],
            mins[2],
        ])

    rows.sort(key=lambda row: (-int(row[1]), row[0]))

    if folder:
        print(f"Folder:          {folder}")
    print(f"Engine:          {engine}")
    if exclude_family:
        print(f"Excluded family: {exclude_family}")
    print(f"Number of seeds: {nseeds}")
    print(f"Number of tests: {ntests}")
    print()
    print_table(header, rows)


if __name__ == "__main__":
    main()
