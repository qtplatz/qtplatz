#!/bin/bash
cwd="$(cd "$(dirname "$0")" && pwd)"
source ${cwd}/config.sh
source ${cwd}/prompt.sh
source ${cwd}/nproc.sh
arch=`uname`-`arch`
__nproc NPROC

build_clean=false
while [ $# -gt 0 ]; do
	case "$1" in
		clean)
			shift
			build_clean=true
			;;
	esac
done

TARGET=gnuplot

function gnuplot_download {
	### https://sourceforge.net/projects/gnuplot/files/gnuplot/5.4.9/gnuplot-5.4.9.tar.gz/download
	### https://sourceforge.net/projects/gnuplot/files/latest/download
	VERSION=$1
    if [ ! -f ${DOWNLOADS}/${TARGET}-${VERSION}.tar.gz ]; then
		echo curl -L -o ${DOWNLOADS}/${TARGET}-${VERSION}.tar.gz \
			 https://sourceforge.net/projects/gnuplot/files/gnuplot/${VERSION}/gnuplot-${VERSION}.tar.gz/download
		echo "=============================="
		prompt
		curl -L -o ${DOWNLOADS}/${TARGET}-${VERSION}.tar.gz \
			 https://sourceforge.net/projects/gnuplot/files/gnuplot/${VERSION}/gnuplot-${VERSION}.tar.gz/download
	fi
	if [ ! -d ${GNUPLOT_BUILD_DIR} ]; then
		echo tar xvf ${DOWNLOADS}/gnuplot-${VERSION}.tar.gz -C $(dirname ${GNUPLOT_BUILD_DIR})
		prompt
		tar xvf ${DOWNLOADS}/gnuplot-${VERSION}.tar.gz -C $(dirname ${GNUPLOT_BUILD_DIR})
	fi
}

BUILD_ROOT=${SRC}/build-${arch}
CROSS_ROOT=
GNUPLOT_BUILD_DIR=$BUILD_ROOT/${TARGET}-${GNUPLOT_VERSION}

echo \${GNUPLOT_BUILD_DIR} " = " ${GNUPLOT_BUILD_DIR}

if [ ! -d ${BUILD_ROOT} ]; then  mkdir -p ${BUILD_ROOT}; fi

if [ $build_clean = true ]; then
	set -x
	rm -rf $BOOST_BUILD_DIR
	exit
fi

gnuplot_download ${GNUPLOT_VERSION}

echo "$target install"

(cd ${GNUPLOT_BUILD_DIR}
 ./prepare
 ./configure --without-qt
 make -j4
 echo sudo make install
 prompt
 sudo make install
 )
