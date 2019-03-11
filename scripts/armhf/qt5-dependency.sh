#!/bin/bash
#see 'http://ms-cheminfo.com/?q=node/52'
#=========================
#     Preparation
#========================
QT_VER=5.12.0
QT_DIR=/opt/Qt/${QT_VER}
QT_SRC=${QT_DIR}/Src/qtbase

mkdir ${QT_SRC}/mkspecs/linux-arm-gnueabihf-g++
cd ${QT_SRC}/mkspecs/linux-arm-gnueabihf-g++
cp ../linux-arm-gnueabi-g++/* .
sed -i -e 's/arm-linux-gnueabi-/arm-linux-gnueabihf-/g' qmake.conf

failed_list=()
list_dependency+=('libclang-3.8-dev'
				  'bison'
				  'flex'
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
