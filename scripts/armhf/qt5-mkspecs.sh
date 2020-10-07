#!/bin/bash
#see 'http://ms-cheminfo.com/?q=node/52'
#=========================
#     Preparation
#========================

cwd="$(cd "$(dirname "$0")" && pwd)"
source ${cwd}/constants.sh

if [ ! -d /opt/Qt/${QTVER} ]; then
	echo "Directory /opt/Qt/${QTVER} does not exist"
	exit 1
fi

if [ ! -d /opt/Qt/${QTVER}/Src/qtbase/mkspecs/linux-arm-gnueabihf-g++ ]; then
	if [ -w /opt/Qt/${QTVER}/Src/qtbase/mkspecs ]; then
		mkdir /opt/Qt/${QTVER}/Src/qtbase/mkspecs/linux-arm-gnueabihf-g++
		cp -r /opt/Qt/${QTVER}/Src/qtbase/mkspecs/linux-arm-gnueabi-g++/* /opt/Qt/${QTVER}/Src/qtbase/mkspecs/linux-arm-gnueabihf-g++
		sed -i -e 's/arm-linux-gnueabi-/arm-linux-gnueabihf-/g' /opt/Qt/${QTVER}/Src/qtbase/mkspecs/linux-arm-gnueabihf-g++/qmake.conf
	else
		echo "###################################################################"
		echo "## Directory /opt/Qt/${QTVER}/Src/qtbase/mkspecs is not writable"
		echo "## Run this script with sudo or make directory writable"
		echo "###################################################################"
		exit 1
	fi
fi

cat /opt/Qt/${QTVER}/Src/qtbase/mkspecs/linux-arm-gnueabihf-g++/qmake.conf
