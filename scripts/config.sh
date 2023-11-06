#!/bin/bash

cwd="$( cd "$( dirname "$0" )" && pwd )"

. ${cwd}/find_qmake.sh

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

##########
if [ ! -z ${cross_target} ]; then
	case ${cross_target} in
		arm-linux-gnueabihf|armhf|armv7l|de0-nano-soc|helio)
			cross_target="arm-linux-gnueabihf"
			CROSS_ROOT="/usr/local/arm-linux-gnueabihf"
			TOOLCHAIN=$(dirname $cwd)/toolchain-arm-linux-gnueabihf.cmake
			;;
		x86_64-w64-mingw32)
			CROSS_ROOT="/usr/local/x86_64-w64-mingw32"
			TOOLCHAIN=$(dirname $cwd)/toolchain-x86_64-w64-mingw32.cmake
			;;
		*)
			echo "************************************"
			echo "* config.sh: Unknown cross target: ${cross_target}"
			echo "************************************"
			CROSS_ROOT=/usr/local/${cross_target}
			exit 1
			;;
	esac
fi

## default downloads directory
if [ -z ${DOWNLOADS} ]; then
    DOWNLOADS=~/Downloads
fi

## top directory for all relevant source archives
if [ -z ${SRC} ]; then
    SRC=~/src
fi

############### boost ################
if [ -z $BOOST_VERSION ]; then
	BOOST_VERSION=1_83_0
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
	fi
fi

if [ -z $CMAKE_VERSION ]; then
	CMAKE_VERSION=3.27.5
fi

if [ -z ${GNUPLOT_VERSION} ]; then
	GNUPLOT_VERSION=5.4.9
fi
