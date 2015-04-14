#!/bin/sh

arch=`arch`
if [ -z $cross_target ]; then
    echo "No cross_target variable has set -- creating native build."
    cross_target=$arch
fi

echo "creating build environment for qtplatz for target: $cross_target"

mkdir build-$cross_target
cd build-$cross_target

case $cross_target in
    helio)
	cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-$cross_target.cmake \
	      -DCMAKE_PREFIX_PATH=/usr/local/qt5 -DQTPLATZ_CORELIB_ONLY=1 ..
	;;
    raspi)
	cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-raspi.cmake \
	      -DCMAKE_PREFIX_PATH=/opt/qt5pi ..
	;;
    i686)
	cmake -DCMAKE_PREFIX_PATH=/opt/Qt/5.4/gcc ..
	;;	
    x86_64)
	cmake -DCMAKE_PREFIX_PATH=/opt/Qt/5.4/gcc_64 ..
	;;
    *)
	echo "Unknown cross_target: $cross_target"
	;;
esac
