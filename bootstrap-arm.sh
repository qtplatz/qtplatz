mkdir -p build/armhf
cd build/armhf
cmake -DCMAKE_TOOLCHAIN_FILE=../../toolchain.cmake ../..
