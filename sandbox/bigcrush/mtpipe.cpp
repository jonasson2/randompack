#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <csignal>
#include <random>

int main(int argc, char **argv) {
  signal(SIGPIPE, SIG_IGN);

  uint32_t seed = 12345u;
  if (argc > 1) {
    seed = (uint32_t)strtoul(argv[1], 0, 10);
  }

  std::mt19937 rng(seed);
  uint32_t buf[65536];

  for (;;) {
    for (int i=0; i<65536; i++) {
      buf[i] = rng();
    }
    size_t nwritten = fwrite(buf, sizeof(uint32_t), 65536, stdout);
    if (nwritten != 65536) {
      break;
    }
  }

  return 0;
}
