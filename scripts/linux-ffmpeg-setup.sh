#!/bin/bash

source ./constants.sh
source ./prompt.sh
VERSION=3.3.3

echo "Install dependency"
sudo apt install libass-dev
sudo apt install libfdk-aac-dev
#sudo apt install FreeType-2.8, LAME-3.99.5,
sudo apt install lame
sudo apt install libtheora-dev
sudo apt install libvorbis-dev
sudo apt install libvpx-dev
sudo apt install libopus-dev
sudo apt install yasm
sudo apt install x264
sudo apt install libx264-dev
sudo apt install x265
sudo apt install libx265-dev
sudo apt install libmp3lame0
sudo apt install libmp3lame-dev

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

./configure --prefix=/usr        \
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

make &&

gcc tools/qt-faststart.c -o tools/qt-faststart
