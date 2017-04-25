#!/bin/bash

source ./constants.sh
source ./prompt.sh

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
    if [ ! -f ~/Downloads/cmake-3.8.0.tar.gz ]; then
      ( cd ~/Downloads;
        wget https://cmake.org/files/v3.8/cmake-3.8.0.tar.gz )
    fi
    tar xvf ~/Downloads/cmake-3.8.0.tar.gz -C $SRC
fi

cd $SRC/cmake-3.8.0
./bootstrap
if make -j4; then
    sudo make install
fi


