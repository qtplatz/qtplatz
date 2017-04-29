#!/bin/bash

source ./constants.sh
source ./prompt.sh

cwd=$(pwd)
arch=`uname`-`arch`

if [ -z $cross_target ]; then
    BUILD_DIR=$SRC/build-$arch/rdkit
    export RDBASE=$SRC/rdkit
    if [ -z $BOOST_ROOT ]; then
	if [ -d /usr/local/boost-1_63 ]; then
	    BOOST_ROOT=/usr/local/boost-1_63
	elif [ -d /usr/local/boost-1_62 ]; then
	    BOOST_ROOT=/usr/local/boost-1_62
	fi
    fi
else
    BUILD_DIR=$SRC/build-$cross_target/rdkit
    CROSS_ROOT=/usr/local/arm-linux-gnueabihf
    export RDBASE=$CROSS_ROOT/usr/local/rdkit
    TOOLCHAIN=$(dirname $cwd)/toolchain-arm-linux-gnueabihf.cmake
    if [ -z $BOOST_ROOT ]; then
	if [ -d $CROSS_ROOT/usr/local/boost-1_63 ]; then
	    BOOST_ROOT=/usr/local/boost-1_63
	elif [ -d $CROSS_ROOT/usr/local/boost-1_62 ]; then
	    BOOST_ROOT=/usr/local/boost-1_62
	fi
    fi
fi

cmake_args=("-DBOOST_ROOT=$BOOST_ROOT" "-DRDK_BUILD_INCHI_SUPPORT=ON" "-DRDK_BUILD_PYTHON_WRAPPERS=OFF")
if [ `uname` == "Darwin" ]; then
    cmake_args+=("-DCMAKE_MACOSX_RPATH=TRUE")
fi
if [ ! -z $cross_target ]; then
    cmake_args+=( "-DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN" "-DRDK_OPTIMIZE_NATIVE=OFF" )
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

if [ -z $cross_target ]; then
    echo "RDBASE    : " $RDBASE
    echo "BUILD_DIR : " `pwd`        
    echo cmake "${cmake_args[@]}" $RDBASE
    prompt
    cmake "${cmake_args[@]}" $RDBASE
    make -j8
    make test
    make install      
else
    echo "RDBASE    : " $RDBASE
    echo "BUILD_DIR : " `pwd`    
    echo cmake "${cmake_args[@]}" $RDBASE
    prompt
    cmake "${cmake_args[@]}" $RDBASE
    make -j8
    if [ $? -eq 0 ]; then
	make test
	make install
    fi
fi
