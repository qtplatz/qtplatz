#!/bin/bash

source ./constants.sh
source ./prompt.sh
VERSION=3.3.3

echo "=========="
echo "building ffmpeg-$VERSION"

if [ ! -d $SRC/ffmpeg-$VERSION ]; then
    if [ ! -d $SRC ]; then
	mkdir $SRC
    fi

    if [ ! -f ~/Downloads/ffmpeg-$VERSION.tar.xz ]; then
	( cd ~/Downloads;
	  wget http://ffmpeg.org/releases/ffmpeg-$VERSION.tar.xz )
    fi
    tar xvf ~/Downloads/ffmpeg-$VERSION.tar.xz -C $SRC
fi

cd $SRC/ffmpeg-$VERSION

sed -i 's/-lflite"/-lflite -lasound"/' configure &&
./configure --prefix=/usr/local  \
            --enable-gpl         \
            --enable-version3    \
            --enable-nonfree     \
            --disable-static     \
            --enable-shared      \
            --disable-debug      \
            --enable-libass      \
            --enable-libfdk-aac  \
            --enable-libfreetype \
            --enable-libmp3lame  \
            --enable-libopus     \
            --enable-libtheora   \
            --enable-libvorbis   \
            --enable-libvpx      \
            --enable-libx264     \
            --enable-libx265     \
            --docdir=/usr/share/doc/ffmpeg-3.3.3 &&
make -j $(nproc --all) &&
gcc tools/qt-faststart.c -o tools/qt-faststart &&
sudo make install

