mkdir build && cd build || exit
cmake .. -DCMAKE_BUILD_TYPE="$BUILD_CONFIGURATION" -DTARGET_CPU="$TARGET_CPU" -DCODE_COVERAGE=ON
cmake --build .
ctest .. --output-on-failure