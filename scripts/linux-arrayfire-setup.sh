#!/bin/bash

source ./constants.sh
source ./prompt.sh

cwd=$(pwd)
arch=`uname`-`arch`
target=arrayfire

source_dir=$SRC/$target

if [ -z $cross_target ]; then
    BUILD_DIR=$SRC/build-$arch/$target
else
    exit 0
fi

if [ -z $BOOST_ROOT ]; then
    if [ -d /usr/local/boost-1_63 ]; then
	BOOST_ROOT=/usr/local/boost-1_63
    elif [ -d /usr/local/boost-1_62 ]; then
	BOOST_ROOT=/usr/local/boost-1_62
    fi
fi

cmake_args=("-DBOOST_ROOT=$BOOST_ROOT" "-DCMAKE_BUILD_TYPE=Release")

echo "Install dependency"
sudo apt-get install -y libfreeimage-dev cmake-curses-gui
sudo apt-get install -y libopenblas-dev libfftw3-dev liblapacke-dev # OpenBLAS
sudo apt-get install -y libglfw3-dev libfontconfig1-dev

echo "$target install"

if [ ! -d $source_dir ]; then
    src=$(dirname $source_dir)
    if [ ! -d $src ]; then mkdir -p $src; fi
    ( cd $src;
      git clone https://github.com/arrayfire/arrayfire.git;
      cd arrayfire
      git submodule init
      git submodule update
    )
    if [ $? -ne 0 ]; then
	exit 1
    fi
fi

mkdir -p $BUILD_DIR;
cd $BUILD_DIR;

echo "BUILD_DIR : " `pwd`
cmake "${cmake_args[@]}" $source_dir
make -j8
