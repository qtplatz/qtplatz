#!/bin/sh

# target should be either 'raspi'|'helio'

if [ $cross_target == "" ]; then
    echo "No cross_target variable has set."
    exit 3
fi

if [ $# -eq 0 ]; then
    echo "No arguments provided"
    exit 3
fi

echo "building package for " $cross_target


mkdir build-helio-qt5
cd build-helio-qt5
cmake -DCMAKE_TOOLCHAIN_FILE=../../../toolchain-helio.cmake ../qt5
echo ""
echo "build-helio-qt5 as build directory set up"

