#!/bin/bash

source ./constants.sh
source ./prompt.sh

if type rdm > /dev/null; then
    version=$(rdm --version)
    echo "=========="
    echo "rdm $version already installed"
    prompt
fi

echo "=========="
echo "building rtags on the directory '$SRC'"

if [ ! -d $SRC ]; then
    echo "$SRC not exist.  Create?"
    prompt
    mkdir -p $SRC
fi

if [ ! -d $SRC/rtags ]; then
    ( cd $SRC; 
      git clone --recursive https://github.com/Andersbakken/rtags.git )
fi

if [ ! -d $SRC/rtags/build ]; then
    mkdir -p $SRC/rtags/build
fi

cd $SRC/rtags/build
cmake ..
if make -j4; then
    echo "==================================================="
    echo "'sudo make install' at `pwd`"
    echo "==================================================="
    prompt
    sudo make install
fi

echo "==================================================="
echo "start rtags daemon by typing 'rdm &'"
echo "Index project by run 'rc -J .' at project dirctory"
echo "==================================================="
