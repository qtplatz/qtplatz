#!/bin/bash

sudo apt update

#dependency for libjpeg
sudo apt install -y nasm

#dependency for ffmpeg

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

#dependency for opencv
echo "Install dependency"
sudo apt-get install -y python-dev python-numpy libtbb2 libtbb-dev # libjpeg-dev
sudo apt-get install -y libpng-dev libtiff-dev
sudo apt-get install -y libpng12-dev libtiff5-dev
sudo apt-get install -y libgtk2.0 pkg-config libavcodec-dev libavformat-dev libswscale-dev
