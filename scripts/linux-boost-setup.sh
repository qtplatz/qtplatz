#!/bin/bash

function prompt {
    while true; do
	read -p "Proceed (y/n)? " yn
	case $yn in
            [Yy]* ) break;;
            [Nn]* ) exit;;
            * ) echo "Please answer yes or no.";;
	esac
    done
}

if [ -z $BOOST_VERSION ]; then
    BOOST_VERSION=1_62_0
fi

if [ -z $PREFIX ]; then
    PREFIX=/usr/local
fi

SRC=~/src
BOOST_INSTALL_DIR=boost-${BOOST_VERSION/%_0//}
BOOST_BUILD_DIR=$SRC/boost_${BOOST_VERSION}
BOOST_PREFIX=$PREFIX/$BOOST_INSTALL_DIR

if [ -d $BOOST_PREFIX ]; then
    echo "=============================="
    echo "boost-$BOOST_VERSION already installed in $BOOST_PREFIX"
    exit 0
fi

echo "=============================="
echo "boost $BOOST_VERSION to be built on $BOOST_BUILD_DIR, install to $BOOST_PREFIX"

prompt

if [ ! -d $SRC ]; then
    mkdir $SRC
fi

cd $SRC;

if [ ! -d bzip2-1.0.6 ]; then
    wget http://www.bzip.org/1.0.6/bzip2-1.0.6.tar.gz
    tar xvf bzip2-1.0.6.tar.gz
fi

if [ ! -d $BOOST_BUILD_DIR ]; then
    VERSION=$(echo $BOOST_VERSION | tr _ .)
    echo "=============================="
    echo wget https://sourceforge.net/projects/boost/files/boost/$VERSION/boost_$BOOST_VERSION.tar.bz2/download
    prompt "Proceed ? "
    if [ -f download ]; then
	rm -f download
    fi
    wget https://sourceforge.net/projects/boost/files/boost/$VERSION/boost_$BOOST_VERSION.tar.bz2/download
    tar xvf download
    mv download ~/Downloads/${BOOST_VERSION}.tar.bz2
fi

cd $BOOST_BUILD_DIR

./bootstrap.sh --prefix=$BOOST_PREFIX
./b2 -j4 address-model=64 cflags=-fPIC cxxflags="-fPIC -std=c++11" -s BZIP2_SOURCE=${SRC}/bzip2-1.0.6

if [ $? ]; then
    echo "build success. install "; prompt
    sudo ./b2 -j4 address-model=64 cflags=-fPIC cxxflags="-fPIC -std=c++11" -s BZIP2_SOURCE=${SRC}/bzip2-1.0.6 install
fi

echo "Setting ld.so.conf.d"
echo "$BOOST_PREFIX/lib" | sudo tee /etc/ld.so.conf.d/boost-${BOOST_VERSION/%_0//}.conf
sudo ldconfig

echo "=============================="
echo "   BOOST $BOOST_VERSION Installed "
echo "=============================="
