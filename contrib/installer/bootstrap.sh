#!/bin/sh

if [ -z $cross_target ]; then
    echo "No cross_target variable has set."
    exit 3
fi

if [ $# -eq 0 ]; then
    echo "No arguments provided"
    echo $0 'qt5 | boost | ace+tao'
    exit 3
fi

for var in "$@"
do
    case $var in
	qt5)
	    echo "building package generator for $var on $cross_target"
	    mkdir -p build-$cross_target/$var
	    cd build-$cross_target/$var
	    cmake -DCMAKE_TOOLCHAIN_FILE=../../../../toolchain-$cross_target.cmake ../../$var
	    ;;
	boost)
	    echo "building package generator for $var on $cross_target"
	    mkdir -p build-$cross_target/$var
	    cd build-$cross_target/$var
	    cmake -DCMAKE_TOOLCHAIN_FILE=../../../../toolchain-$cross_target.cmake ../../$var
	    ;;
	ace+tao)
	    echo "building package generator for $var on $cross_target"
	    mkdir -p build-$cross_target/$var
	    cd build-$cross_target/$var
	    cmake -DCMAKE_TOOLCHAIN_FILE=../../../../toolchain-$cross_target.cmake ../../$var
	    ;;
	*)
	    echo "unknown target: $var"
	    ;;
    esac
done
