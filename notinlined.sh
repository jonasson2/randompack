rm -rf build
meson setup build -Dbuildtype=release -Dc_args="-Rpass=inline -Rpass-missed=inline -Rpass-analysis=inline -fsave-optimization-record -foptimization-record-file=opt"
ninja -C build | grep 'not inlined'|cols 3|sort -u|sub  "'" ""
