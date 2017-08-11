#!/bin/bash

source ./constants.sh
source ./prompt.sh

VERSION=3.9.1
VDIR=v3.9

if type cmake > /dev/null; then
    version=$(cmake --version | grep version)
    echo "=========="
    echo "cmake $version already installed"
    prompt
fi

echo "=========="
echo "building cmake"

if [ ! -d $SRC/cmake ]; then
    if [ ! -d $SRC ]; then
	mkdir $SRC
    fi
    # git clone https://gitlab.kitware.com/cmake/cmake $SRC/cmake
    if [ ! -f ~/Downloads/cmake-$VERSION.tar.gz ]; then
      ( cd ~/Downloads;
        wget https://cmake.org/files/$VDIR/cmake-$VERSION.tar.gz )
    fi
    tar xvf ~/Downloads/cmake-$VERSION.tar.gz -C $SRC
fi

cd $SRC/cmake-$VERSION
./bootstrap
if make -j4; then
    sudo make install
fi


