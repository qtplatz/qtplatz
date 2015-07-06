#!/bin/sh

arch=`arch`
if [ -z $cross_target ]; then
    echo "No cross_target variable has set -- creating native build."
    cross_target=$arch
fi

build_eclipse=

while [ $# -gt 0 ]; do
    case "$1" in
	eclipse)
	    echo "*********** building eclipse project"
	    build_eclipse='true'
	    shift
	    ;;
	*)
	    break
	    ;;
    esac
done    
    
echo "creating build environment for qtplatz for target: $cross_target"

if [ build_eclipse='true' ]; then
    mkdir ../build-qtplatz-$cross_target
    cd ../build-qtplatz-$cross_target
else
    mkdir build-$cross_target
    cd build-$cross_target
fi

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
	if [ build_eclipse='true' ]; then
	    cmake -G "Eclipse CDT4 - Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=/opt/Qt/5.4/gcc_64 ../qtplatz
	else
	    cmake -DCMAKE_PREFIX_PATH=/opt/Qt/5.4/gcc_64 ..
	fi
	;;
    armv7l)
	cmake -DCMAKE_PREFIX_PATH=/usr/local/qt5 -DQTPLATZ_CORELIB_ONLY=1 ..
	;;
    *)
	echo "Unknown cross_target: $cross_target"
	;;
esac
