#!/bin/bash

sudo apt-get -y install curl

sudo dpkg --add-architecture armhf
dpkg --print-foreign-architectures

echo "deb http://emdebian.org/tools/debian/ jessie main" | sudo tee /etc/apt/sources.list.d/crosstools.list

curl http://emdebian.org/tools/debian/emdebian-toolchain-archive.key | sudo apt-key add -

sudo apt-get update
sudo apt-get -y install crossbuild-essential-armhf

echo "==========================="
dpkg --print-foreign-architectures
echo "==========================="
