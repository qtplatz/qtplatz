#!/bin/bash

source ./prompt.sh

if type cmake > /dev/null; then
    version=$(cmake --version | grep version)
    echo "=========="
    echo "cmake $version already installed"
    prompt
fi

exit
echo "=========="
echo "building cmake"

if [ ! -d ~/src/cmake ]; then
    if [ ! -d ~/src ]; then
	mkdir ~/src
    fi
    # git clone https://gitlab.kitware.com/cmake/cmake ~/src/cmake
    ( cd ~/Downloads;
      wget https://cmake.org/files/v3.8/cmake-3.8.0.tar.gz )
    tar xvf ~/Downlodads/cmake-3.8.0.tar.gz -C ~/src
fi

cd ~/src/cmake
./bootstrap
if make -j4; then
    sudo make install
fi


