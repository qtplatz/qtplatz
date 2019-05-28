#!/bin/bash

cwd="$( cd "$( dirname "$0" )" && pwd )"
OS="$(uname -s)"

if [ -z $BOOST_VERSION ]; then
	case "${OS}" in
		Darwin*) BOOST_VERSION=1_69_0;;
		*) BOOST_VERSION=1_67_0;;
	esac
fi

if [ -z $BOOST_ROOT ]; then
    a=(${BOOST_VERSION//_/ })
    BOOST_ROOT=/usr/local/boost-${a[0]}_${a[1]}
fi

if [ -z $CMAKE_VERSION ]; then
	CMAKE_VERSION=3.14.3
fi

if [ -z $SRC ]; then
    SRC=~/src
fi

if [ -z $DOWNLOADS ]; then
    DOWNLOADS=~/Downloads
fi

. ${cwd}/find_qmake.sh
