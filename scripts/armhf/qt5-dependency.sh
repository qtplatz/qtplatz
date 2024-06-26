#!/bin/bash
#see 'http://ms-cheminfo.com/?q=node/52'
#=========================
#     Preparation
#========================

cwd="$(cd "$(dirname "$0")" && pwd)"
source ${cwd}/constants.sh

QTDIR=/opt/Qt/${QTVER}
QTSRC=${QTDIR}/Src/qtbase

if ! ${cwd}/qt5-mkspecs.sh; then
	echo "qt5-mkspecs.sh script failed."
	exit 1
fi

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
	if ! dpkg -s "$arg" ; then
		echo sudo apt-get install -y "$arg"
		sudo apt-get install -y "$arg" || failed_list+=("$arg")
	fi
done

if [ ${#failed_list[@]} -gt 0 ]; then
   echo "Error: Total " ${#failed_list[@]} packages failed to install.
   for arg in "${failed_list[@]}"; do
	echo "	" "$arg"
   done
fi
