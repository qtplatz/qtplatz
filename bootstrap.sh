#!/bin/sh

if [ -z $cross_target ]; then
    echo "No cross_target variable has set."
    echo "usage: cross_target=helio|raspi $0"
    exit 3
fi

echo "creating build environment for qtplatz for target: $cross_target"

mkdir build-$cross_target
cd build-$cross_target

case $cross_target in
    helio)
	cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-$cross_target.cmake \
	      -DCMAKE_PREFIX_PATH=/usr/local/qt5 ..
	;;
    raspi)
	cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-raspi.cmake \
	      -DCMAKE_PREFIX_PATH=/opt/qt5pi ..
	;;
    *)
	echo "Unknown cross_target: $cross_target"
	;;
esac
