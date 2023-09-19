#!/bin/bash

cwd="$( cd "$( dirname "$0" )" && pwd )"

function build_uname() {
	local __uname=`uname`
    case "${__uname}" in
		MINGW64*|MSYS*)
			echo "mingw"-`arch`
			;;
		*)
			echo "${__uname}"-`arch`
			;;
	esac
}

if [ -z ${INSTALL_PREFIX} ]; then
	case `uname` in
		MINGW64*|MSYS*)
			INSTALL_PREFIX=/usr/local
			;;
		*)
			INSTALL_PREFIX=/usr/local
			;;
	esac
fi

## top directory for all relevant source archives
if [ -z ${SRC} ]; then
    SRC=~/src
fi


############### boost ################
if [ -z $BOOST_VERSION ]; then
	BOOST_VERSION=1_79_0
fi

## /usr/local/boost-1_79
if [ -z $BOOST_ROOT ]; then
    a=(${BOOST_VERSION//_/ })
	BOOST_INSTALL_PREFIX="${INSTALL_PREFIX}/boost-${a[0]}_${a[1]}"
	BOOST_ROOT=${BOOST_INSTALL_PREFIX}
fi

## ~/src/build-uname-arch/boost_1_79_0
if [ -z ${BUILD_ROOT} ]; then
	if [ -z ${cross_target} ]; then
		BUILD_ROOT=${SRC}/build-`build_uname`
	else
		BUILD_ROOT=${SRC}/build-${cross_target};
		CROSS_ROOT=/usr/local/arm-linux-gnueabihf
	fi
fi

if [ -z ${DOWNLOADS} ]; then
    DOWNLOADS=~/Downloads
fi

. ${cwd}/find_qmake.sh

if [ -z $CMAKE_VERSION ]; then
	CMAKE_VERSION=3.27.4
fi

if [ -z ${GNUPLOT_VERSION} ]; then
	GNUPLOT_VERSION=5.4.9
fi
