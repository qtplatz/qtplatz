#!/bin/bash

cwd="$( cd "$( dirname "$0" )" && pwd )"

if [ -z $BOOST_VERSION ]; then
	if [ -d /usr/local/boost-1_67 ]; then
		BOOST_VERSION=1_67_0
	elif [ -d /usr/local/boost-1_62 ]; then
		BOOST_VERSION=1_62_0
	fi
fi

if [ -z $BOOST_ROOT ]; then
    a=(${BOOST_VERSION//_/ })
    BOOST_ROOT=/usr/local/boost-${a[0]}_${a[1]}
fi

if [ -z $CMAKE_VERSION ]; then
	CMAKE_VERSION=3.12.3
fi

if [ -z $SRC ]; then
    SRC=~/src
fi

if [ -z $DOWNLOADS ]; then
    DOWNLOADS=~/Downloads
fi

. ${cwd}/find_qmake.sh
