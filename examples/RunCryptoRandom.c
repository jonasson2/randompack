// Test program for crypto_random() - checks odd/even distribution
//
// Generates random uint64_t values and verifies they're roughly 50/50 odd/even
// Uses 7-sigma test: standard deviation = sqrt(n/4)
// Probability of accidental failure outside 7 sigma is astronomically low

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

// Declaration of crypto_random function
bool crypto_random(void *buffer, size_t size);

// getopt declarations
extern int optind;
extern int optopt;
extern int opterr;
extern char *optarg;
int getopt(int argc, char * const argv[], const char *optstring);

void usage(const char *progname) {
  fprintf(stderr, "Usage: %s [-n samples]\n", progname);
  fprintf(stderr, "  -n samples  Number of samples (default: 100000)\n");
  exit(1);
}

int main(int argc, char *argv[]) {
  int num_samples = 100000;  // default
  int opt;

  // Parse command-line options
  while ((opt = getopt(argc, argv, "n:")) != -1) {
    switch (opt) {
      case 'n':
        num_samples = atoi(optarg);
        if (num_samples <= 0) {
          fprintf(stderr, "Error: number of samples must be positive\n");
          usage(argv[0]);
        }
        break;
      default:
        usage(argv[0]);
    }
  }

  int odd_count = 0;
  int even_count = 0;

  printf("Testing crypto_random() with %d samples...\n", num_samples);

  // Generate samples and count odd/even
  for (int i = 0; i<num_samples; i++) {
    uint64_t value;
    if (!crypto_random(&value, sizeof(value))) {
      fprintf(stderr, "Error: crypto_random() failed at sample %d\n", i);
      return 1;
    }

    if (value%2 == 0)
      even_count++;
    else
      odd_count++;
  }

  // Calculate statistics
  double expected = num_samples/2.0;
  double sigma = sqrt(num_samples/4.0);
  double deviation = odd_count-expected;
  double num_sigmas = fabs(deviation)/sigma;

  printf("\nResults:\n");
  printf("  Odd:  %d (%.4f%%)\n", odd_count, (double)odd_count/num_samples*100);
  printf("  Even: %d (%.4f%%)\n", even_count, (double)even_count/num_samples*100);
  printf("  Expected: %.1f\n", expected);
  printf("  Standard deviation (sigma): %.2f\n", sigma);
  printf("  Deviation: %.1f (%.2f sigma)\n", deviation, num_sigmas);

  // 7-sigma test: probability of accidental failure is astronomically low
  double lower_bound = expected-7*sigma;
  double upper_bound = expected+7*sigma;

  printf("\nAcceptable range: [%.1f, %.1f]\n", lower_bound, upper_bound);

  if (odd_count >= lower_bound && odd_count <= upper_bound) {
    printf("\n✓ TEST PASSED: Distribution within 7 sigma\n");
    return 0;
  }
  else {
    printf("\n✗ TEST FAILED: Distribution outside 7 sigma (astronomically unlikely)\n");
    return 1;
  }
}
