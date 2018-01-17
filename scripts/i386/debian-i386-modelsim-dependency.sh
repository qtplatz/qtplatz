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
