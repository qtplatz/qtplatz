#!/bin/bash

source ./prompt.sh

cwd=$(pwd)
arch=`uname`-`arch`
target=opencv
source_dir=~/src/opencv

if [ -z $cross_target ]; then
    BUILD_DIR=~/src/build-$arch/$target
else
    BUILD_DIR=~/src/build-$cross_target/$target
    CROSS_ROOT=/usr/local/arm-linux-gnueabihf
    TOOLCHAIN=$(dirname $cwd)/toolchain-arm-linux-gnueabihf.cmake
fi

echo "Install dependency"
sudo apt-get update
sudo apt-get install python-dev python-numpy libtbb2 libtbb-dev libjpeg-dev libpng-dev libtiff-dev
sudo apt-get install libgtk2.0 pkg-config libavcodec-dev libavformat-dev libswscale-dev

echo "$target install"

if [ ! -d $source_dir ]; then
    src=$(dirname $source_dir)
    if [ ! -d $(dirname $src) ]; then
	mkdir -p $(dirname $src)
    fi
    (cd $src;
     git clone https://github.com/opencv/opencv.git
     git clone https://github.com/opencv_contrib.git
    )
fi

mkdir -p $BUILD_DIR;
cd $BUILD_DIR;

if [ -z $cross_target ]; then
    echo "BUILD_DIR : " `pwd`
    cmake -DCMAKE_EXTRA_MODULES_PATH=$(dirname $source_dir)/opencv_contrib/modules $source_dir
    make -j8
    make install      
fi
