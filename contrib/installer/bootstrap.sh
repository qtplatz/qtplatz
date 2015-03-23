#!/bin/sh

if [ -z $cross_target ]; then
    echo "No cross_target variable has set."
    exit 3
fi

args="$@"

if [ $# -eq 0 ]; then
    args="boost qt5 ace+tao"
fi

for var in $args
do
    case $var in
	qt5|boost|ace+tao)
	    echo "building package generator for $var on $cross_target"
	    mkdir -p build-$cross_target/$var
	    ( cd build-$cross_target/$var;
	      cmake -DCMAKE_TOOLCHAIN_FILE=../../../../toolchain-$cross_target.cmake ../../$var )
	    ;;
	clean)
	    echo rm -rf build-$cross_target
	    rm -rf build-$cross_target
	    ;;
	build)
	    for pkg in boost qt5 ace+tao; do
		( cd build-$cross_target/$pkg;
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
