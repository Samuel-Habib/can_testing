rm -rf build/test &&
cmake -B build/test -S Test &&
cmake --build build/test &&
ctest --test-dir build/test --output-on-failure
