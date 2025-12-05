// Check that counts is consistent with counts in bins with equal probability
bool check_balanced_counts(int *counts, int n) {
  double q = 1e-13; // astromically low
  double N = 0;
  for (int i=0; i<n; i++) N += counts[i];
  double mean = (double)N/n;
  double var = (double)N*(n - 1)/(n*n); // of each count
  double max_dev = sqrt(2.0*var*log(2*n/q));
  for (int i=0; i<n; i++)
    if (fabs(counts[i] - mean) > max_dev) return false;
  return true;
}
