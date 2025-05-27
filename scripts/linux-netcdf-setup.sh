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
    BUILD_DIR=$SRC/build-$arch/netcdf-c.release
    SRCDIR=$SRC/netcdf-c
else
    BUILD_DIR=$SRC/build-$cross_target/netcdf-c.release
    CROSS_ROOT=/usr/local/arm-linux-gnueabihf
    SRCDIR=$CROSS_ROOT/usr/local/netcdf-c
    TOOLCHAIN=$(dirname $cwd)/toolchain-arm-linux-gnueabihf.cmake
fi

if [ $build_clean = true ]; then
	set -x
	rm -rf $BUILD_DIR
	exit
fi

if [ `uname` == "Darwin" ]; then
    cmake_args=( "-DCMAKE_OSX_DEPLOYMENT_TARGET=10.15"  # Catalina
			   )
fi

if [ ! -z $cross_target ]; then
    cmake_args+=( "-DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN" "-DCMAKE_BUILD_TYPE=Release" )
else
	cmake_args+=( "-DCMAKE_BUILD_TYPE=Release" )
fi
cmake_args+=( "-DBUILD_SHARED_LIBS=OFF" )
cmake_args+=( "-DENABLE_NETCDF_4=OFF" )
cmake_args+=( "-DENABLE_NETCDF4=OFF" )
cmake_args+=( "-DENABLE_HDF4=OFF" )
cmake_args+=( "-DENABLE_PNETCDF=OFF" )
cmake_args+=( "-DENABLE_FILTER_BLOSC=OFF" )
cmake_args+=( "-DENABLE_FILTER_ZSTD=OFF" )

if [ ! -d $SRCDIR ]; then
    if [ ! -d $(dirname $SRCDIR) ]; then
		mkdir -p $(dirname $SRCDIR)
    fi
	git clone https://github.com/Unidata/netcdf-c.git ${SRCDIR}
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
