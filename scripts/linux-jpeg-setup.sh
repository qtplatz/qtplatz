#!/bin/bash

source ./constants.sh
source ./prompt.sh
VERSION=1.5.2

echo "=========="
echo "building libjpeg-turbo-$VERSION"

if [ ! -d $SRC/libjpeg-turbo-$VERSION ]; then
    if [ ! -d $SRC ]; then
	mkdir $SRC
    fi

    if [ ! -f ~/Downloads/libjpeg-turbo-$VERSION.tar.gz ]; then
	( cd ~/Downloads;
	  wget http://downloads.sourceforge.net/libjpeg-turbo/libjpeg-turbo-$VERSION.tar.gz )
    fi
    tar xvf ~/Downloads/http://downloads.sourceforge.net/libjpeg-turbo/libjpeg-turbo-$VERSION.tar.gz -C $SRC
fi

cd $SRC/libjpeg-turbo-$VERSION

./configure --prefix=/usr/local           \
            --mandir=/usr/local/share/man \
            --with-jpeg8            \
            --disable-static        \
            --docdir=/usr/local/share/doc/libjpeg-turbo-$VERSION &&
make -j8
sudo make install
