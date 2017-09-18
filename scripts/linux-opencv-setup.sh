#!/bin/bash

source ./constants.sh
source ./prompt.sh

cwd=$(pwd)
arch=`uname`-`arch`
target=opencv
config=release
source_dir=$SRC/$target
contrib_dir=$(dirname $source_dir)/opencv_contrib
extra_dir=$(dirname $source_dir)/opencv_extra

while [ $# -gt 0 ]; do
    case "$1" in
	debug)
	    config=debug
	    shift
	    ;;
	*)
	    break
	    ;;
    esac
done

if [ -z $cross_target ]; then
    BUILD_DIR=$SRC/build-$arch/$target.$config
else
    exit 0
fi

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

if [ "$config" = "debug" ]; then
    BUILD_CONFIG="Debug"
else
    BUILD_CONFIG="Release"
fi

if [ -z $cross_target ]; then
    echo "BUILD_DIR : " `pwd`
    cmake -DCMAKE_EXTRA_MODULES_PATH=$contrib_dir/opencv_contrib/modules \
	  -DCMAKE_BUILD_TYPE=$BUILD_CONFIG \
	  -DENABLE_CXX11=ON		   \
	  -DBUILD_PERF_TESTS=OFF           \
	  -DWITH_XINE=ON                   \
	  -DBUILD_TESTS=OFF                \
	  -DENABLE_PRECOMPILED_HEADERS=OFF \
	  -DCMAKE_SKIP_RPATH=ON            \
	  -DBUILD_WITH_DEBUG_INFO=OFF      \
	  -DCUDA_FAST_MATH=$CUDA           \
	  -DWITH_CUBLAS=$CUDA              \
	  -DCUDA_NVCC_FLAGS="--expt-relaxed-constexpr" \
	  $source_dir

    cmake -DOPENCV_ICV_URL="http://downloads.sourceforge.net/project/opencvlibrary/3rdparty/ippicv" .
    
    echo "########################"
    echo "You may need to add '--expt-relaxed-constexpr' to CUDA_NVCC_FLAGS cache using ccmake"
    echo In case you got failed with ippicv, try follwoing on the build directory '$BUILD_DIR'
    echo cmake -DOPENCV_ICV_URL="http://downloads.sourceforge.net/project/opencvlibrary/3rdparty/ippicv" .
    echo 
    echo "make sure cmake supports openssl"
    echo
    echo When you have an error 'nvcuvid.h no such file or directory',
    echo run 'sudo touch /usr/local/cuda/include/nvcuvid.h' as workaround
    echo
    echo "Did you install ffmpeg and turbo-jpeg?"
    echo "########################"
    echo "make -j8 # at `pwd`"    
    prompt
    make -j $(nproc --all)
#    export OPENCV_TEST_DATA_PATH=$extra_dir/testdata
    #    make test
    echo "sudo make -j8 install"
    prompt
    sudo make -j8 install

    case $(uname -m) in
	x86_64) ARCH=intel64 ;;
	*) ARCH=ia32    ;;
    esac  &&
	sudo cp -v 3rdparty/ippicv/ippicv_lnx/lib/$ARCH/libippicv.a /usr/local/lib &&
	unset ARCH    
fi
