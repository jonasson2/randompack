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
    "mt",
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

REV_ABBREV = {v: k for k, v in ABBREV.items()}
THRESHOLDS = [1e-3, 1e-4, 1e-5, 1e-6]


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
    value, _family = entry
    return format_q(value)


def format_expected(nruns, tests_per_run, threshold):
    expected = nruns*tests_per_run*2*threshold
    return f"{expected:.1f}"


def format_sdev(values):
    if not values:
        return "-"
    mean = sum(values)/len(values)
    var = sum((x - mean)*(x - mean) for x in values)/len(values)
    return f"{math.sqrt(var):.2f}"


def format_mean(values):
    if not values:
        return "-"
    return sum(values)/len(values)


def format_mean_cell(value, index):
    decimals = [3, 4, 4, 5]
    return f"{value:.{decimals[index]}f}"


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


def summarize(values, seeds):
    seen_engines = []
    run_counts = {}
    pair_counts = {}
    for engine in values:
        seen_engines.append(engine)
        pair_counts[engine] = [set() for _ in THRESHOLDS]
    for engine, engine_values in values.items():
        for value, family, seed in engine_values:
            key = (engine, seed)
            if key not in run_counts:
                run_counts[key] = [0 for _ in THRESHOLDS]
            for i, threshold in enumerate(THRESHOLDS):
                if value < threshold:
                    run_counts[key][i] += 1
                    pair_counts[engine][i].add((seed, family))

    header = [
        "Engine",
        "n(q<1e-3)",
        "n(q<1e-4)",
        "n(q<1e-5)",
        "n(q<1e-6)",
        "lowest-q",
        "2nd-lowest",
        "3rd-lowest",
        "4th-lowest",
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
        counts = [sum(1 for value, _, _ in engine_values if value < threshold)
                  for threshold in THRESHOLDS]
        total_counts[engine] = counts
        mins = []
        for i in range(4):
            if len(engine_values) <= i:
                mins.append("-")
            else:
                mins.append(format_min(engine_values[i][:2]))
        row = [
            engine,
            format_count(counts[0], len(pair_counts[engine][0]), True),
            format_count(counts[1], len(pair_counts[engine][1]), True),
            format_count(counts[2], len(pair_counts[engine][2]), False),
            format_count(counts[3], len(pair_counts[engine][3]), False),
            mins[0],
            mins[1],
            mins[2],
            mins[3],
        ]
        rows.append(row)

    same_nruns = len(set(nruns_by_engine.values())) == 1
    same_tests_per_run = len(set(tests_per_run_by_engine.values())) == 1
    per_rows = []
    total_rows = []
    if same_tests_per_run:
        total_nruns = len(run_counts)
        tests_per_run = next(iter(tests_per_run_by_engine.values()))
        obs_mean_row = ["Obs.mean n"] + [
            format_mean([counts[i] for counts in run_counts.values()]) for i in range(4)
        ] + [
            "-",
            "-",
            "-",
            "-",
        ]
        exp_mean = [tests_per_run*2*threshold for threshold in THRESHOLDS]
        exp_mean_row = [
            "Exp.mean n",
            format_mean_cell(exp_mean[0], 0),
            format_mean_cell(exp_mean[1], 1),
            format_mean_cell(exp_mean[2], 2),
            format_mean_cell(exp_mean[3], 3),
            "-",
            "-",
            "-",
            "-",
        ]
        obs_mean_row = [
            "Obs.mean n",
            format_mean_cell(obs_mean_row[1], 0),
            format_mean_cell(obs_mean_row[2], 1),
            format_mean_cell(obs_mean_row[3], 2),
            format_mean_cell(obs_mean_row[4], 3),
            "-",
            "-",
            "-",
            "-",
        ]
        obs_sdev_row = ["Obs.n-sdev"] + [
            format_sdev([counts[i] for counts in run_counts.values()]) for i in range(4)
        ] + [
            "-",
            "-",
            "-",
            "-",
        ]
        exp_sdev = [math.sqrt(tests_per_run*(2*threshold)*(1 - 2*threshold))
                    for threshold in THRESHOLDS]
        exp_sdev_row = [
            "Exp.n-sdev",
            f"{exp_sdev[0]:.2f}",
            f"{exp_sdev[1]:.2f}",
            f"{exp_sdev[2]:.2f}",
            f"{exp_sdev[3]:.2f}",
            "-",
            "-",
            "-",
            "-",
        ]
        obs_sdev = [
            float(obs_sdev_row[1]),
            float(obs_sdev_row[2]),
            float(obs_sdev_row[3]),
            float(obs_sdev_row[4]),
        ]
        if same_nruns:
            nruns = next(iter(nruns_by_engine.values()))
            mu = [tests_per_run*nruns*2*threshold for threshold in THRESHOLDS]
            expected_row = [
                "Expected total",
                format_expected(nruns, tests_per_run, 1e-3),
                format_expected(nruns, tests_per_run, 1e-4),
                format_expected(nruns, tests_per_run, 1e-5),
                format_expected(nruns, tests_per_run, 1e-6),
                "-",
                "-",
                "-",
                "-",
            ]
            ci_row = [
                "95% CI",
                format_ci(mu[0], obs_sdev[0], nruns),
                format_ci(mu[1], obs_sdev[1], nruns),
                format_ci(mu[2], obs_sdev[2], nruns),
                format_ci(mu[3], obs_sdev[3], nruns),
                "-",
                "-",
                "-",
                "-",
            ]
            per_rows.extend([
                expected_row, obs_mean_row, exp_mean_row, obs_sdev_row,
                exp_sdev_row, ci_row
            ])
            if len(engines) == 1:
                counts = total_counts[engines[0]]
                gamma_p_row = [
                    "Gamma-p",
                    format_gamma_pvalue(counts[0], mu[0], obs_sdev[0], nruns),
                    format_gamma_pvalue(counts[1], mu[1], obs_sdev[1], nruns),
                    format_gamma_pvalue(counts[2], mu[2], obs_sdev[2], nruns),
                    format_gamma_pvalue(counts[3], mu[3], obs_sdev[3], nruns),
                    "-",
                    "-",
                    "-",
                    "-",
                ]
                per_rows.append(gamma_p_row)
        if len(engines) > 1:
            total_mu = [tests_per_run*total_nruns*2*threshold for threshold in THRESHOLDS]
            total_row = [
                "Total",
                str(sum(counts[0] for counts in total_counts.values())),
                str(sum(counts[1] for counts in total_counts.values())),
                str(sum(counts[2] for counts in total_counts.values())),
                str(sum(counts[3] for counts in total_counts.values())),
                "-",
                "-",
                "-",
                "-",
            ]
            total_expected_row = [
                "Expected total",
                format_expected(total_nruns, tests_per_run, 1e-3),
                format_expected(total_nruns, tests_per_run, 1e-4),
                format_expected(total_nruns, tests_per_run, 1e-5),
                format_expected(total_nruns, tests_per_run, 1e-6),
                "-",
                "-",
                "-",
                "-",
            ]
            total_ci_row = [
                "95% CI",
                format_ci(total_mu[0], obs_sdev[0], total_nruns),
                format_ci(total_mu[1], obs_sdev[1], total_nruns),
                format_ci(total_mu[2], obs_sdev[2], total_nruns),
                format_ci(total_mu[3], obs_sdev[3], total_nruns),
                "-",
                "-",
                "-",
                "-",
            ]
            total_rows.extend([
                total_row, total_expected_row, obs_mean_row, exp_mean_row,
                obs_sdev_row, exp_sdev_row, total_ci_row
            ])

    widths = [len(name) for name in header]
    for row in rows + per_rows + total_rows:
        for i, cell in enumerate(row):
            widths[i] = max(widths[i], len(cell))

    print(" ".join(header[i].ljust(widths[i]) if i == 0 or i >= 5 else header[i].rjust(widths[i])
                     for i in range(len(header))))
    for row in rows:
        print(" ".join(row[i].ljust(widths[i]) if i == 0 or i >= 5 else row[i].rjust(widths[i])
                         for i in range(len(row))))
    if per_rows:
        print()
        print("Per run:")
    for row in per_rows:
        print(" ".join(row[i].ljust(widths[i]) if i == 0 or i >= 5 else row[i].rjust(widths[i])
                         for i in range(len(row))))
    if total_rows:
        print()
        print("Overall:")
    for row in total_rows:
        print(" ".join(row[i].ljust(widths[i]) if i == 0 or i >= 5 else row[i].rjust(widths[i])
                         for i in range(len(row))))


def main():
    if len(sys.argv) != 3:
        raise SystemExit("usage: summary_fam.py DIR FAMILY")
    folder = sys.argv[1]
    short_name = sys.argv[2]
    if short_name not in REV_ABBREV:
        known = ", ".join(sorted(REV_ABBREV))
        raise SystemExit(f"unknown family abbreviation {short_name}; known: {known}")
    want_family = REV_ABBREV[short_name]

    values = {}
    seeds = set()
    engines = set()
    ntests = 0

    for raw_line in sys.stdin:
        parts = raw_line.split()
        if len(parts) != 4:
            continue
        seed_text, engine, family, value_text = parts
        if family != want_family:
            continue
        seed = int(seed_text)
        value = float(value_text)
        if engine not in values:
            values[engine] = []
        values[engine].append((value, family, seed))
        seeds.add(seed)
        engines.add(engine)
        ntests += 1

    if not values:
        raise SystemExit("no data")
    nruns = len(seeds)
    neng = len(engines)
    total_runs = 0
    for engine_values in values.values():
        total_runs += len({seed for _, _, seed in engine_values})
    if ntests != 254 and ntests != 254*neng*nruns:
        pass
    print(f"Folder:            {folder}")
    print(f"Family:            {want_family} ({short_name})")
    print(f"Number of engines: {neng}")
    print(f"Number of runs:    {total_runs}")
    print(f"Number of tests:   {ntests}")
    print()
    summarize(values, seeds)


if __name__ == "__main__":
    main()
