#!/bin/bash
#see 'http://ms-cheminfo.com/?q=node/52'
#=========================
#     Preparation
#========================
QT_VER=5.12.0
QT_DIR=/opt/Qt/${QT_VERSION}
QT_SRC=${QT_DIR}/Src/qtbase

mkdir -p ~/src/build-armhf/qt5-build
cd ~/src/build-armhf/qt5-build

#/opt/Qt/5.12.0/Src/configure -nomake examples -no-gui -skip qt3d -skip qtwebengine --release -xplatform linux-arm-gnueabihf-g++ -prefix /usr/local/arm-linux-gnueabihf/opt/Qt/5.12.0/
/opt/Qt/5.12.0/Src/configure -nomake examples -nomake tests -skip qtandroidextras -release -xplatform linux-arm-gnueabihf-g++ -prefix /usr/local/arm-linux-gnueabihf/opt/Qt/5.12.0/ -opensource -confirm-license
