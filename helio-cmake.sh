mkdir build-helio
cd build-helio
cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-helio.cmake \
      -DCMAKE_INSTALL_PREFIX=/opt/local/arm-linux-gnueabihf/usr/local \
      -DCMAKE_PREFIX_PATH=/opt/local/arm-linux-gnueabihf/usr/local/qt5 ..
