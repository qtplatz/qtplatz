#!/bin/bash

cwd="$(cd "$(dirname "$0")" && pwd)"
source ${cwd}/constants.sh
source ${cwd}/prompt.sh
source ${cwd}/nproc.sh

cwd=$(pwd)
arch=`uname`-`arch`
__nproc nproc

if [ -z $cross_target ]; then
    BUILD_DIR=$SRC/build-$arch/rdkit.release
    export RDBASE=$SRC/rdkit
else
    BUILD_DIR=$SRC/build-$cross_target/rdkit
    CROSS_ROOT=/usr/local/arm-linux-gnueabihf
    export RDBASE=$CROSS_ROOT/usr/local/rdkit
    TOOLCHAIN=$(dirname $cwd)/toolchain-arm-linux-gnueabihf.cmake
    if [ -z $BOOST_ROOT ]; then
		if [ -d $CROSS_ROOT/usr/local/boost-1_67 ]; then
			BOOST_ROOT=/usr/local/boost-1_67
		elif [ -d $CROSS_ROOT/usr/local/boost-1_62 ]; then
			BOOST_ROOT=/usr/local/boost-1_62
		fi
    fi
fi

cmake_args=( "-DBOOST_ROOT=$BOOST_ROOT"
			 "-DRDK_BUILD_INCHI_SUPPORT=ON"
			 "-DRDK_BUILD_PYTHON_WRAPPERS=OFF"
			 "-DRDK_INSTALL_INTREE=OFF"
			 "-DRDK_INSTALL_STATIC_LIBS=OFF"
			 "-DRDK_INSTALL_DYNAMIC_LIBS=ON"
		   )
if [ `uname` == "Darwin" ]; then
    cmake_args+=("-DCMAKE_MACOSX_RPATH=TRUE")
fi
if [ ! -z $cross_target ]; then
    cmake_args+=( "-DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN" "-DRDK_OPTIMIZE_NATIVE=OFF" )
	sudo apt install libeigen3-dev
fi

echo "RDKit install on $RDBASE"

if [ ! -d $RDBASE ]; then
    if [ ! -d $(dirname $RDBASE) ]; then
		mkdir -p $(dirname $RDBASE)
    fi
    git clone https://github.com/rdkit/rdkit $RDBASE
fi

mkdir -p $BUILD_DIR;
cd $BUILD_DIR;

echo "RDBASE    : " $RDBASE
echo "BUILD_DIR : " `pwd`
echo cmake "${cmake_args[@]}" $RDBASE
prompt
cmake "${cmake_args[@]}" $RDBASE
make -j${nproc}

#if [ $? -eq 0 ]; then
#	make test
#	make install
#fi

echo "You may need to edit rdkit-target.cmake manually to set '_IMPORT_PREFIX' or get failed to find Catalogs library"
echo "You may also need to make sym-link RDKIT/lib/cmake/*.cmake RDKIT/lib"
echo "Edit /usr/local/lib/cmake/rdkit/rdkit-config.cmake as:"
echo '    include ("\${_prefix}/rdkit/rdkit-targets.cmake")'
echo "change lib to rdkit in the middle of path name"

##CMake Error at /usr/local/lib/cmake/rdkit/rdkit-config.cmake:6 (include):
##  include could not find load file:
##
##    /usr/local/lib/cmake/lib/rdkit-targets.cmake
