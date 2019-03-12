#!/bin/bash
#see 'http://ms-cheminfo.com/?q=node/52'
#=========================
#     Preparation
#========================
QTVER=5.12.0
QTDIR=/opt/Qt/${QTVER}
QTSRC=${QTDIR}/Src/qtbase

mkdir -p ~/src/build-armhf/qt5-build
cd ~/src/build-armhf/qt5-build

#/opt/Qt/5.12.0/Src/configure -nomake examples -no-gui -skip qt3d -skip qtwebengine --release -xplatform linux-arm-gnueabihf-g++ -prefix /usr/local/arm-linux-gnueabihf/opt/Qt/5.12.0/
/opt/Qt/$QTVER/Src/configure -nomake examples -nomake tests -skip qtandroidextras -release -xplatform linux-arm-gnueabihf-g++ -prefix /usr/local/arm-linux-gnueabihf/opt/Qt/$QTVER/ -opensource -confirm-license

make -j8 module-qtbase module-qtlocation
make -j8 module-qtbase module-qtlocation install
