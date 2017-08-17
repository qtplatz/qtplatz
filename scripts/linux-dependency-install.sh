#!/bin/bash

sudo apt update

#dependency for libjpeg
sudo apt install -y nasm

#dependency for ffmpeg

#dependency for opencv
echo "Install dependency"
sudo apt-get install -y python-dev python-numpy libtbb2 libtbb-dev # libjpeg-dev
sudo apt-get install -y libpng-dev libtiff-dev
sudo apt-get install -y libpng12-dev libtiff5-dev
sudo apt-get install -y libgtk2.0 pkg-config libavcodec-dev libavformat-dev libswscale-dev
