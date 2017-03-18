#!/bin/sh

build=

if ! which cmake >/dev/null; then
    build=true
fi

if [ -z $build ]; then
    echo "=========="
    echo "cmake already installed"
    exit
fi

echo "=========="
echo "building cmake"

if [ ! -d ~/src/cmake ]; then
    git clone https://gitlab.kitware.com/cmake/cmake ~/src/cmake
fi

cd ~/src/cmake
./bootstrap
if make -j4; then
    sudo make install
fi


