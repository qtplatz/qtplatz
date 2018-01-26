#!/bin/bash

if [ -z $distro ]; then
    distro=$(lsb_release -cs)
fi
echo "distribution=$distro"

#sudo apt-get -y install curl
sudo dpkg --add-architecture i386
dpkg --print-foreign-architectures

sudo apt-get update
sudo apt-get install libc6:i386 libncurses5:i386 libstdc++6:i386
sudo apt-get install lib32z1 lilb32ncurses5 lib32bz2-1.0
sudo apt-get install libxext6:i386 libxft2:i386

echo "==========================="
echo dpkg --print-foreign-architectures
dpkg --print-foreign-architectures
echo "==========================="

echo "wget http://download.savannah.gnu.org/releases/freetype/freetype-2.4.12.tar.bz2"
echo "tar freetype-2.4.12.tar.bz2 -C ~/src/"
echo "cd ~/src/freetype-2.4.12"
echo "./configure --build=i686-pc-linux-gnu "CFLAGS=-m32" "CXXFLAGS=-m32" "LDFLAGS=-m32""
echo "make -j8"

echo "open http://mattaw.blogspot.jp/2014/05/making-modelsim-altera-starter-edition.html"
