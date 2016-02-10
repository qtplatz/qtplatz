#!/bin/sh

arch=`arch`
cwd=`pwd`
target_arch=$cross_target

if [ $# -eq 0 ]; then
    echo "usage:"
    echo "native"
    echo "\t./bootstrap.sh [boost [qt5 [ace+tao]]]"
    echo "cross"
    echo "\tcross_target=[arm-linux-gnueabihf|raspi] ./bootstrap.sh [boost [qt5 [ace+tao]]]"
    exit
fi

if [ -z $cross_target ]; then
    target_arch=$arch    
    echo "No cross_target variable found -- creating native build for $target_arch."
fi

args="$@"


for var in $args
do
    case $var in
	qt5|boost|ace+tao)
	    echo "building package generator for $var on $cross_target"
	    mkdir -p build-$target_arch/$var
	    if [ -z $cross_target ]; then
		( cd build-$target_arch/$var;
		  cmake -DCMAKE_PREFIX_PATH=/usr/local/boost-1_58 ../../$var )
	    else
		( cd build-$target_arch/$var;
		  cmake -DCMAKE_TOOLCHAIN_FILE=../../../../toolchain-$target_arch.cmake ../../$var )
	    fi
	    ;;
	clean)
	    echo rm -rf build-$target_arch
	    -rm -rf build-$target_arch	    
	    ;;
	build)
	    for pkg in boost qt5 ace+tao; do
		( cd build-$target_arch/$pkg;
		  make package;
		  mv *.deb ..
		)
	    done
	    ;;
	*)
	    echo "unknown target: $var"
	    ;;
    esac
done
