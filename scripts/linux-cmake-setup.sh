#!/bin/bash

cwd="$(cd "$(dirname "$0")" && pwd)"
source ${cwd}/config.sh
source ${cwd}/prompt.sh
source ${cwd}/nproc.sh

__nproc nproc
arch=`uname`-`arch`

a=(${CMAKE_VERSION}//_/ })
VDIR=v${a[0]}.${a[1]}

if type cmake > /dev/null; then
    version=$(cmake --version | grep version)
    echo "=========="
    echo "cmake $version already installed"
    prompt
fi

echo "=========="
echo "building cmake"

sudo apt install openssl libssl-dev

if [ ! -d $SRC/cmake ]; then
    if [ ! -d $SRC ]; then
	mkdir $SRC
    fi
    # git clone https://gitlub.com/cmake/cmake $SRC/cmake
    # git clone https://github.com/Kitware/CMake.git
    if [ ! -f ${DOWNLOADS}/cmake-$CMAKE_VERSION.tar.gz ]; then
		( cd ~/Downloads;
		  curl -L -o ${DOWNLOADS}/cmake-$CMAKE_VERSION.tar.gz \
			   https://github.com/Kitware/CMake/releases/download/v$CMAKE_VERSION/cmake-$CMAKE_VERSION.tar.gz )
    fi
    tar xvf ${DOWNLOADS}/cmake-$CMAKE_VERSION.tar.gz -C ${SRC}
fi

cd ${SRC}/cmake-${CMAKE_VERSION}
./bootstrap --system-curl

if make -j $nproc; then
    sudo make install
fi
