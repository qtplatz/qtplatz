#!/bin/sh

#mkdir build-raspi
#cd build-raspi
#../build-raspi.sh

cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-raspi.cmake -DCMAKE_PREFIX_PATH=/opt/qt5pi ..
