#!/bin/bash

cwd="$(cd "$(dirname "$0")" && pwd)"
source ${cwd}/config.sh
source ${cwd}/prompt.sh
source ${cwd}/nproc.sh

arch=`uname`-`arch`

__nproc nproc

build_clean=false

while [ $# -gt 0 ]; do
	case "$1" in
		clean)
			shift
			build_clean=true
			;;
	esac
done

if [ -z $cross_target ]; then
    BUILD_DIR=$SRC/build-$arch/maeparser.release
    SRCDIR=$SRC/maeparser
else
    BUILD_DIR=${SRC}/build-${cross_target}/maeparser.release
    SRCDIR=$SRC/maeparser
fi

if [ $build_clean = true ]; then
	set -x
	rm -rf $BUILD_DIR
	exit
fi

if [ `uname` == "Darwin" ]; then
    cmake_args=( "-DCMAKE_OSX_DEPLOYMENT_TARGET=12.0" )
fi

if [ ! -z ${cross_target} ]; then
    cmake_args+=( "-DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN" )
else
	cmake_args+=( "-DBOOST_ROOT=$BOOST_ROOT" )
fi

if [ ! -d $SRCDIR ]; then
    if [ ! -d $(dirname $SRCDIR) ]; then
		mkdir -p $(dirname $SRCDIR)
    fi
    git clone https://github.com/schrodinger/maeparser $SRCDIR
fi

mkdir -p $BUILD_DIR;
cd $BUILD_DIR;

echo cmake "${cmake_args[@]}" $SRCDIR
prompt
cmake "${cmake_args[@]}" $SRCDIR

echo "make -j${nproc}"
prompt
make -k -j${nproc}

if [ $? -eq 0 ]; then
	echo sudo make install
	prompt
	sudo make install
fi
