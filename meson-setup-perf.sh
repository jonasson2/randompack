rm -rf perf
meson setup perf -Dbuildtype=release -Dperf=true
ninja -C perf
