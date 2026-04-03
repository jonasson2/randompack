#!/usr/bin/env python3

import math
import sys


PREFERRED_ENGINES = [
    "x256++simd",
    "x256++",
    "x256**",
    "x256ss",
    "x128+",
    "xoro++",
    "pcg64",
    "squares",
    "philox",
    "sfc64",
    "cwg128",
    "ranlux++",
    "chacha20",
]

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
    if value == 0.0:
        return "0"
    text = f"{value:.0e}"
    text = text.replace("e-0", "e-")
    text = text.replace("e+0", "e+")
    return text


def format_min(entry):
    if entry is None:
        return "-"
    value, family, seed = entry
    return f"{format_q(value)} ({ABBREV[family]},{seed})"


def format_expected(nruns, tests_per_run, threshold):
    expected = nruns*tests_per_run*2*threshold
    return f"{expected:.1f}"


def format_sdev(values):
    if not values:
        return "-"
    mean = sum(values)/len(values)
    var = sum((x - mean)*(x - mean) for x in values)/len(values)
    return f"{math.sqrt(var):.2f}"


def format_overdispersion(obs_sdev, exp_sdev):
    if exp_sdev <= 0:
        return "-"
    return f"{obs_sdev/exp_sdev:.3f}"


def gamma_p(a, x):
    if x <= 0:
        return 0.0
    gln = math.lgamma(a)
    if x < a + 1:
        ap = a
        total = 1/a
        delta = total
        for _ in range(200):
            ap += 1
            delta *= x/ap
            total += delta
            if abs(delta) < abs(total)*1e-14:
                break
        return total*math.exp(-x + a*math.log(x) - gln)
    b = x + 1 - a
    c = 1e300
    d = 1/b
    h = d
    for i in range(1, 200):
        an = -i*(i - a)
        b += 2
        d = an*d + b
        if abs(d) < 1e-300:
            d = 1e-300
        c = b + an/c
        if abs(c) < 1e-300:
            c = 1e-300
        d = 1/d
        delta = d*c
        h *= delta
        if abs(delta - 1) < 1e-14:
            break
    return 1 - math.exp(-x + a*math.log(x) - gln)*h


def gamma_quantile(shape, scale, prob):
    if prob <= 0:
        return 0.0
    if prob >= 1:
        return math.inf
    lo = 0.0
    hi = max(shape*scale, 1.0)
    while gamma_p(shape, hi/scale) < prob:
        hi *= 2
    for _ in range(100):
        mid = 0.5*(lo + hi)
        if gamma_p(shape, mid/scale) < prob:
            lo = mid
        else:
            hi = mid
    return 0.5*(lo + hi)


def format_ci(mean, sdev, nruns):
    if nruns <= 0:
        return "-"
    var = nruns*sdev*sdev
    if mean <= 0 or var <= 0:
        x = max(0.0, mean)
        return f"[{x:.1f},{x:.1f}]"
    shape = mean*mean/var
    scale = var/mean
    lo = gamma_quantile(shape, scale, 0.025)
    hi = gamma_quantile(shape, scale, 0.975)
    return f"[{lo:.1f},{hi:.1f}]"


def format_gamma_pvalue(total, mean, sdev, nruns):
    var = nruns*sdev*sdev
    if mean <= 0 or var <= 0:
        return "-"
    shape = mean*mean/var
    scale = var/mean
    cdf = gamma_p(shape, total/scale)
    pval = 2*min(cdf, 1 - cdf)
    if pval >= 0.01:
        return f"{pval:.3f}"
    text = f"{pval:.1e}"
    text = text.replace("e-0", "e-")
    text = text.replace("e+0", "e+")
    return text


def format_count(total, npairs, show_pairs):
    if show_pairs:
        return f"{total} ({npairs})"
    return str(total)


def summarize(values, seeds, show_family_in_min, overdisp):
    seen_engines = []
    run_counts = {}
    pair_counts = {}
    for engine in values:
        seen_engines.append(engine)
        pair_counts[engine] = [set(), set(), set()]
    for engine, engine_values in values.items():
        for value, family, seed in engine_values:
            key = (engine, seed)
            if key not in run_counts:
                run_counts[key] = [0, 0, 0]
            if value < 1e-3:
                run_counts[key][0] += 1
                pair_counts[engine][0].add((seed, family))
            if value < 1e-4:
                run_counts[key][1] += 1
                pair_counts[engine][1].add((seed, family))
            if value < 1e-5:
                run_counts[key][2] += 1
                pair_counts[engine][2].add((seed, family))

    header = [
        "Engine",
        "n(q<1e-3)",
        "n(q<1e-4)",
        "n(q<1e-5)",
        "lowest-q",
        "2nd-lowest",
        "3rd-lowest",
    ]

    rows = []
    total_counts = {}
    engines = [engine for engine in PREFERRED_ENGINES if engine in values]
    for engine in seen_engines:
        if engine not in PREFERRED_ENGINES:
            engines.append(engine)
    nruns_by_engine = {}
    tests_per_run_by_engine = {}
    tests_per_run = 0
    for engine in engines:
        nruns = len({seed for _, _, seed in values[engine]})
        nvals = len(values[engine])
        if nruns <= 0 or nvals % nruns != 0:
            raise SystemExit(
                f"engine {engine} has {nvals} values, not divisible by {nruns} seeds"
            )
        nper = nvals // nruns
        nruns_by_engine[engine] = nruns
        tests_per_run_by_engine[engine] = nper
        if tests_per_run == 0:
            tests_per_run = nper
        elif nper != tests_per_run:
            tests_per_run = -1
    for engine in engines:
        engine_values = sorted(values[engine], key=lambda item: (item[0], item[1], item[2]))
        counts = [
            sum(1 for value, _, _ in engine_values if value < 1e-3),
            sum(1 for value, _, _ in engine_values if value < 1e-4),
            sum(1 for value, _, _ in engine_values if value < 1e-5),
        ]
        total_counts[engine] = counts
        mins = []
        for i in range(3):
            if len(engine_values) <= i:
                mins.append("-")
            elif show_family_in_min:
                mins.append(format_min(engine_values[i]))
            else:
                mins.append(format_q(engine_values[i][0]))
        row = [
            engine,
            format_count(counts[0], len(pair_counts[engine][0]), True),
            format_count(counts[1], len(pair_counts[engine][1]), True),
            format_count(counts[2], len(pair_counts[engine][2]), False),
            mins[0],
            mins[1],
            mins[2],
        ]
        rows.append(row)

    same_nruns = len(set(nruns_by_engine.values())) == 1
    same_tests_per_run = len(set(tests_per_run_by_engine.values())) == 1
    per_rows = []
    total_rows = []
    if same_tests_per_run:
        total_nruns = len(run_counts)
        tests_per_run = next(iter(tests_per_run_by_engine.values()))
        obs_sdev_row = [
            "Obs.sdev",
            format_sdev([counts[0] for counts in run_counts.values()]),
            format_sdev([counts[1] for counts in run_counts.values()]),
            format_sdev([counts[2] for counts in run_counts.values()]),
            "-",
            "-",
            "-",
        ]
        exp_sdev = [
            math.sqrt(tests_per_run*0.002*(1 - 0.002)),
            math.sqrt(tests_per_run*0.0002*(1 - 0.0002)),
            math.sqrt(tests_per_run*0.00002*(1 - 0.00002)),
        ]
        exp_sdev_row = [
            "Exp.sdev",
            f"{exp_sdev[0]:.2f}",
            f"{exp_sdev[1]:.2f}",
            f"{exp_sdev[2]:.2f}",
            "-",
            "-",
            "-",
        ]
        obs_sdev = [
            float(obs_sdev_row[1]),
            float(obs_sdev_row[2]),
            float(obs_sdev_row[3]),
        ]
        used_overdisp = [
            overdisp[0] if overdisp else obs_sdev[0]/exp_sdev[0],
            overdisp[1] if overdisp else obs_sdev[1]/exp_sdev[1],
            overdisp[2] if overdisp else obs_sdev[2]/exp_sdev[2],
        ]
        model_sdev = [
            used_overdisp[0]*exp_sdev[0],
            used_overdisp[1]*exp_sdev[1],
            used_overdisp[2]*exp_sdev[2],
        ]
        overdisp_row = [
            "Overdisp",
            f"{used_overdisp[0]:.3f}",
            f"{used_overdisp[1]:.3f}",
            f"{used_overdisp[2]:.3f}",
            "-",
            "-",
            "-",
        ]
        if same_nruns:
            nruns = next(iter(nruns_by_engine.values()))
            mu = [
                tests_per_run*nruns*0.002,
                tests_per_run*nruns*0.0002,
                tests_per_run*nruns*0.00002,
            ]
            expected_row = [
                "Expected",
                format_expected(nruns, tests_per_run, 1e-3),
                format_expected(nruns, tests_per_run, 1e-4),
                format_expected(nruns, tests_per_run, 1e-5),
                "-",
                "-",
                "-",
            ]
            ci_row = [
                "95% CI",
                format_ci(mu[0], model_sdev[0], nruns),
                format_ci(mu[1], model_sdev[1], nruns),
                format_ci(mu[2], model_sdev[2], nruns),
                "-",
                "-",
                "-",
            ]
            per_rows.extend([expected_row, obs_sdev_row, exp_sdev_row, overdisp_row, ci_row])
            if len(engines) == 1:
                counts = total_counts[engines[0]]
                gamma_p_row = [
                    "Gamma-p",
                    format_gamma_pvalue(counts[0], mu[0], model_sdev[0], nruns),
                    format_gamma_pvalue(counts[1], mu[1], model_sdev[1], nruns),
                    format_gamma_pvalue(counts[2], mu[2], model_sdev[2], nruns),
                    "-",
                    "-",
                    "-",
                ]
                per_rows.append(gamma_p_row)
        if len(engines) > 1:
            total_mu = [
                tests_per_run*total_nruns*0.002,
                tests_per_run*total_nruns*0.0002,
                tests_per_run*total_nruns*0.00002,
            ]
            total_row = [
                "Total",
                str(sum(counts[0] for counts in total_counts.values())),
                str(sum(counts[1] for counts in total_counts.values())),
                str(sum(counts[2] for counts in total_counts.values())),
                "-",
                "-",
                "-",
            ]
            total_expected_row = [
                "Expected",
                format_expected(total_nruns, tests_per_run, 1e-3),
                format_expected(total_nruns, tests_per_run, 1e-4),
                format_expected(total_nruns, tests_per_run, 1e-5),
                "-",
                "-",
                "-",
            ]
            total_ci_row = [
                "95% CI",
                format_ci(total_mu[0], model_sdev[0], total_nruns),
                format_ci(total_mu[1], model_sdev[1], total_nruns),
                format_ci(total_mu[2], model_sdev[2], total_nruns),
                "-",
                "-",
                "-",
            ]
            total_rows.extend([total_row, total_expected_row, obs_sdev_row,
                               exp_sdev_row, overdisp_row, total_ci_row])

    widths = [len(name) for name in header]
    for row in [*rows, *per_rows, *total_rows]:
        for i, cell in enumerate(row):
            widths[i] = max(widths[i], len(cell))

    return engines, header, rows, per_rows, total_rows, tests_per_run, len(run_counts)


def print_table(header, rows, per_rows, total_rows):
    widths = [len(name) for name in header]
    for row in [*rows, *per_rows, *total_rows]:
        for i, cell in enumerate(row):
            widths[i] = max(widths[i], len(cell))
    for row in [header, *rows]:
        pieces = []
        for i, cell in enumerate(row):
            if i == 0 or i >= 4:
                pieces.append(cell.ljust(widths[i]))
            else:
                pieces.append(cell.rjust(widths[i]))
        print("  ".join(pieces))
    if per_rows:
        print()
        print("Per family:")
    for row in per_rows:
        pieces = []
        for i, cell in enumerate(row):
            if i == 0 or i >= 4:
                pieces.append(cell.ljust(widths[i]))
            else:
                pieces.append(cell.rjust(widths[i]))
        print("  ".join(pieces))
    if total_rows:
        print()
        print("Overall:")
    for row in total_rows:
        pieces = []
        for i, cell in enumerate(row):
            if i == 0 or i >= 4:
                pieces.append(cell.ljust(widths[i]))
            else:
                pieces.append(cell.rjust(widths[i]))
        print("  ".join(pieces))


def main():
    args = sys.argv[1:]
    overdisp = 0
    if len(args) >= 2 and args[0] == "-o":
        vals = args[1].split(",")
        if len(vals) != 3:
            raise SystemExit("summary.py: -o expects x.xxx,y.yyy,z.zzz")
        overdisp = [float(v) for v in vals]
        args = args[2:]
    folder = args[0] if len(args) >= 1 else ""
    nfam = int(args[1]) if len(args) >= 2 else 0
    exclude_family = args[2] if len(args) >= 3 else ""
    values = {}
    seeds = set()
    ntests = 0
    family_counts = {}

    for raw_line in sys.stdin:
        parts = raw_line.split()
        if len(parts) != 4:
            continue
        seed_text, engine, family, value_text = parts
        abbrev = ABBREV.get(family, family)
        if exclude_family and abbrev == exclude_family:
            continue
        seed = int(seed_text)
        value = float(value_text)
        seeds.add(seed)
        ntests += 1
        if engine not in values:
            values[engine] = []
        values[engine].append((value, family, seed))
        family_counts[family] = family_counts.get(family, 0) + 1

    engines, header, rows, per_rows, total_rows, tests_per_run, nruns_total = summarize(
      values, seeds, True, overdisp
    )
    nengines = len(engines)
    expected_ntests = 0
    for engine, engine_values in values.items():
        nruns = len({seed for _, _, seed in engine_values})
        nvals = len(engine_values)
        if nruns <= 0 or nvals % nruns != 0:
            raise SystemExit(
                f"engine {engine} has {nvals} values, not divisible by {nruns} seeds"
            )
        expected_ntests += nvals
    if ntests != expected_ntests:
        raise SystemExit(
            f"expected {expected_ntests} tests from per-engine run counts, got {ntests}"
        )
    if folder:
        print(f"Folder:            {folder}")
    if exclude_family:
        print(f"Excluded family:   {exclude_family}")
    print(f"Number of engines: {nengines}")
    print(f"Number of runs:    {nruns_total}")
    print(f"Number of tests:   {ntests}")
    print()
    print_table(header, rows, per_rows, total_rows)

    if nfam <= 0:
        return

    print()
    print("Families")
    for family, count in sorted(family_counts.items(), key=lambda item: (-item[1], item[0]))[:nfam]:
        print(f"{family:<28} {count:3d}")
    for family, count in sorted(family_counts.items(), key=lambda item: (-item[1], item[0]))[:nfam]:
        print()
        print(f"Family: {family}")
        fam_values = {}
        for engine, engine_values in values.items():
            fam_engine_values = [item for item in engine_values if item[1] == family]
            if fam_engine_values:
                fam_values[engine] = fam_engine_values
        _, fam_header, fam_rows, fam_per_rows, fam_total_rows, _, _ = summarize(
            fam_values, seeds, False, overdisp
        )
        print_table(fam_header, fam_rows, fam_per_rows, fam_total_rows)


if __name__ == "__main__":
    main()
