#!/bin/bash
#see 'http://ms-cheminfo.com/?q=node/52'
#=========================
#     Preparation
#========================
QTVER=5.12.0
QTDIR=/opt/Qt/${QTVER}
QTSRC=${QTDIR}/Src/qtbase

mkdir /opt/Qt/${QTVER}/Src/qtbase/mkspecs/linux-arm-gnueabihf-g++
cp -r /opt/Qt/${QTVER}/Src/qtbase/mkspecs/linux-arm-gnueabi-g++ /opt/Qt/${QTVER}/Src/qtbase/mkspecs/linux-arm-gnueabihf-g++
sed -i -e 's/arm-linux-gnueabi-/arm-linux-gnueabihf-/g' /opt/Qt/${QTVER}/Src/qtbase/mkspecs/linux-arm-gnueabihf-g++/qmake.conf

failed_list=()
list_dependency+=('libclang-3.8-dev'
				  'bison'
				  'flex'
				  'gperf'
				 )

list_dependency+=('mesa-common-dev:armhf'
				  'libglu1-mesa-dev:armhf'
				  'freeglut3-dev:armhf'
				  )


for arg in "${list_dependency[@]}"; do
    echo sudo apt-get install -y "$arg"
    sudo apt-get install -y "$arg" || failed_list+=("$arg")
done

if [ ${#failed_list[@]} -gt 0 ]; then
   echo "Error: Total " ${#failed_list[@]} packages failed to install.
   for arg in "${failed_list[@]}"; do
	echo "	" "$arg"
   done
fi
