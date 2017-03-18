#!/bin/bash

SRC=~/src
BUILD_DIR=$SRC/build-armhf

# find host boost library, lookup host boost lib, then set same for target
if [ -z $BOOST_VERSION ]; then
    if [ -d /usr/local/boost-1_63 ]; then
	BOOST_VERSION=1_63_0
    elif [ -d /usr/local/boost-1_62 ]; then
	BOOST_VERSION=1_62_0
    else
	BOOST_VERSION=1_62_0
    fi
fi

BOOST_ARCHIVE=boost_${BOOST_VERSION}.tar.bz2
BZIP2_SOURCE=$SRC/bzip2-1.0.6

staff=$(id -Gn | grep -c staff)
echo "I am staff group: " $staff

if [ ! -d $BZIP2_SOURCE ]; then
    wget http://www.bzip.org/1.0.6/bzip2-1.0.6.tar.gz
    tar xvf bzip2-1.0.6.tar.gz -C ~/src
fi

if [ ! -f ~/user-config.jam ]; then
    echo "# Creating ~/user-config.jam..."
    cat <<EOF>~/user-config.jam
using gcc : arm : arm-linux-gnueabihf-g++ : <cxxflags>"-std=c++14 -fPIC" ;
using python : 2.7 ;
EOF
fi

if [ ! -d $BUILD_DIR ]; then
    mkdir -p $BUILD_DIR
fi

if [ ! -d $BUILD_DIR/boost_$BOOST_VERSION ]; then
    if [ ! -f ~/Downloads/$BOOST_ARCHIVE ]; then
	echo "# downloading boost_$BOOST_VERSION"
	VERSION=$(echo $BOOST_VERSION | tr _ .)
	wget https://sourceforge.net/projects/boost/files/boost/$VERSION/$BOOST_ARCHIVE/download
	mv download ~/Downloads/$BOOST_ARCHIVE
    fi
    tar xvf ~/Downloads/$BOOST_ARCHIVE -C $BUILD_DIR
fi

if [ ! -d $BUILD_DIR/boost_$BOOST_VERSION ]; then
    echo "no boost source"
    exit
fi

cd $BUILD_DIR/boost_$BOOST_VERSION

echo ./bootstrap.sh --prefix=/usr/local/arm-linux-gnueabihf/usr/local
./bootstrap.sh --prefix=/usr/local/arm-linux-gnueabihf/usr/local

if [ ! -d /usr/local/arm-linux-gnueabihf/usr/local ]; then
    if ! mkdir -p /usr/local/arm-linux-gnueabihf/usr/local ; then
	echo "mkdir -p /usr/local/arm-linux-gnueabihf/usr/local -- command faild. You need to be a 'staff' group"
	if [ staff -eq 0 ]; then
	    echo "You need to join 'staff' group as run follwoing command".
	    echo sudo usermod -a -G staff $(whoami)
	    echo "then login again"
	    exit
	fi
	sudo mkdir -p /usr/local/arm-linux-gnueabihf/usr/local
	sudo chgrp staff /usr/local/arm-linux-gnueabihf/usr/local
	sudo chmod g+sw /usr/local/arm-linux-gnueabihf/usr/local
    fi
fi

if [ ! -w /usr/local/arm-linux-gnueabihf/usr/local ]; then
    echo "Make /usr/local/arm-linux-gnueabihf/usr/local writable."
    echo "Or run following command with sudo"
    echo "./b2 toolset=gcc-arm -s BZIP2_SOURCE=$BZIP2_SOURCE -j4 install"
else
    ./b2 toolset=gcc-arm -s BZIP2_SOURCE=$BZIP2_SOURCE -j4 install
fi
    


