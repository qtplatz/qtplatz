#!/bin/sh

sudo apt-get install build-essential libncurses-dev fakeroot dpkg-dev
sudo apt-get build-dep linux

sudo apt-get install linux-source

