#!/bin/bash

set -eu

file=${1:-test-mt/mt_s1.txt}

if [ ! -f "$file" ]
then
  echo "file not found: $file" >&2
  exit 1
fi

./extract.sh "$file" \
  | awk '{ count[$3]++ } END { for (fam in count) print fam, count[fam] }' \
  | sort -k2,2nr -k1,1 \
  | awk '
    BEGIN {
      abbrev["multin_MultinomialOver"] = "MultOver";
      abbrev["marsa_BirthdaySpacings"] = "BirthSp";
      abbrev["npair_ClosePairs"] = "ClosePair";
      abbrev["knuth_SimpPoker"] = "SimpPok";
      abbrev["knuth_CouponCollector"] = "Coupon";
      abbrev["knuth_Gap"] = "Gap";
      abbrev["knuth_Run"] = "KnuthRun";
      abbrev["multin_Multinomial"] = "Multinom";
      abbrev["knuth_MaxOft"] = "MaxOft";
      abbrev["varia_SampleProd"] = "SampProd";
      abbrev["varia_SampleMean"] = "SampMean";
      abbrev["varia_SampleCorr"] = "SampCorr";
      abbrev["varia_AppearanceSpacings"] = "AppSpac";
      abbrev["varia_WeightDistrib"] = "Weight";
      abbrev["varia_SumCollector"] = "SumColl";
      abbrev["marsa_MatrixRank"] = "Matrix";
      abbrev["marsa_Savir2"] = "Savir2";
      abbrev["marsa_GCD"] = "GCD";
      abbrev["walk_RandomWalk1"] = "RandWalk";
      abbrev["comp_LinearComp"] = "LinComp";
      abbrev["comp_LempelZiv"] = "LempelZiv";
      abbrev["spectral_Fourier3"] = "Fourier3";
      abbrev["string_LongestHeadRun"] = "LongHead";
      abbrev["string_PeriodsInStrings"] = "PeriodStr";
      abbrev["string_HammingWeight2"] = "HamWeight";
      abbrev["string_HammingCorr"] = "HamCorr";
      abbrev["string_HammingIndep"] = "HamIndep";
      abbrev["string_Run"] = "StringRun";
      abbrev["string_AutoCor"] = "AutoCor";
      printf "%-28s %-10s %3s\n", "family", "abbrev", "n";
    }
    {
      printf "%-28s %-10s %3d\n", $1, abbrev[$1], $2;
    }'
