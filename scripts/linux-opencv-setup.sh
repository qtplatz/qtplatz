#!/bin/bash

source ./constants.sh
source ./prompt.sh

cwd=$(pwd)
arch=`uname`-`arch`
target=opencv

source_dir=$SRC/$target
contrib_dir=$(dirname $source_dir)/opencv_contrib
extra_dir=$(dirname $source_dir)/opencv_extra

if [ -z $cross_target ]; then
    BUILD_DIR=$SRC/build-$arch/$target
else
    exit 0
fi

echo "Install dependency"
sudo apt-get install -y python-dev python-numpy libtbb2 libtbb-dev libjpeg-dev libpng-dev libtiff-dev
sudo apt-get install -y libgtk2.0 pkg-config libavcodec-dev libavformat-dev libswscale-dev

echo "$target install"

if [ ! -d $source_dir ]; then
    src=$(dirname $source_dir)
    if [ ! -d $src ]; then mkdir -p $src; fi
    ( cd $src;  git clone https://github.com/opencv/opencv.git )
    if [ $? -ne 0 ]; then
	exit 1
    fi
fi

if [ ! -d $contrib_dir ]; then
    src=$(dirname $contrib_dir)
    if [ ! -d $src ]; then mkdir -p $src; fi
    (cd $src;  git clone https://github.com/opencv/opencv_contrib.git )
    if [ $? -ne 0 ]; then
	exit 1
    fi
fi

if [ ! -d $extra_dir ]; then
    src=$(dirname $extra_dir)
    if [ ! -d $src ]; then mkdir -p $src; fi
    (cd $src;  git clone https://github.com/opencv/opencv_extra.git )
    if [ $? -ne 0 ]; then
	exit 1
    fi
fi

mkdir -p $BUILD_DIR;
cd $BUILD_DIR;

if [ -z $cross_target ]; then
    echo "BUILD_DIR : " `pwd`
    cmake -DCMAKE_EXTRA_MODULES_PATH=$contrib_dir/opencv_contrib/modules -DENABLE_CXX11=ON $source_dir
    echo "make -j8 # at `pwd`"
    prompt
    export OPENCV_TEST_DATA_PATH=$extra_dir/testdata
    make -j8
#    make test
    sudo make -j8 install      
fi
