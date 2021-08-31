#!/bin/bash
#see 'http://ms-cheminfo.com/?q=node/52'
#=========================
#     Preparation
#========================

cwd="$(cd "$(dirname "$0")" && pwd)"
source ${cwd}/constants.sh

prefix="/usr/local/arm-linux-gnueabihf/opt/Qt/$QTVER/"

if ! ${cwd}/qt5-mkspecs.sh; then
	echo "qt5-mkspecs.sh script failed."
	exit 1
fi

mkdir -p ~/src/build-armhf/qt5-build
cd ~/src/build-armhf/qt5-build

/opt/Qt/$QTVER/Src/configure -nomake examples -nomake tests -skip qtandroidextras -release -xplatform linux-arm-gnueabihf-g++ -prefix $prefix -opensource -confirm-license

if make -j8 module-qtbase module-qtlocation; then
	echo "build success. going to install..."
	sudo make -j8 module-qtbase module-qtlocation install
else
	echo "build qt5 failed."
fi
