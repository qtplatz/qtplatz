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
sudo apt-get install -y python-dev python-numpy libtbb2 libtbb-dev # libjpeg-dev
sudo apt-get install -y libpng-dev libtiff-dev
sudo apt-get install -y libpng12-dev libtiff5-dev
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

CUDA=OFF
if [ -d /usr/local/cuda ]; then
    CUDA=ON
fi

mkdir -p $BUILD_DIR;
cd $BUILD_DIR;

if [ -z $cross_target ]; then
    echo "BUILD_DIR : " `pwd`
    cmake -DCMAKE_EXTRA_MODULES_PATH=$contrib_dir/opencv_contrib/modules \
	  -DCMAKE_BUILD_TYPE=Release \
	  -DENABLE_CXX11=ON \
	  -DBUILD_PERF_TESTS=OFF           \
	  -DWITH_XINE=ON                   \
	  -DBUILD_TESTS=OFF                \
	  -DENABLE_PRECOMPILED_HEADERS=OFF \
	  -DCMAKE_SKIP_RPATH=ON            \
	  -DBUILD_WITH_DEBUG_INFO=OFF      \
	  -DCUDA_FAST_MATH=$CUDA           \
	  -DWITH_CUBLAS=$CUDA              \
	  $source_dir

    echo "Did you install ffmpeg and turbo-jpeg?"
    echo "make -j8 # at `pwd`"    
    prompt
    make -j8
#    export OPENCV_TEST_DATA_PATH=$extra_dir/testdata
#    make test
    sudo make -j8 install

    case $(uname -m) in
	x86_64) ARCH=intel64 ;;
	*) ARCH=ia32    ;;
    esac  &&
	sudo cp -v 3rdparty/ippicv/ippicv_lnx/lib/$ARCH/libippicv.a /usr/local/lib &&
	unset ARCH    
fi
