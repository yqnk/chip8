rm -rf "build"
cmake -G Ninja -B build -DCMAKE_C_COMPILER=/usr/bin/clang
cmake --build build
